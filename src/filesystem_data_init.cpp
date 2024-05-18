#include <filesystem.h>
#include <ui.h>
#include <io.h>
#include <host.h>

/* read SPIFFS partition and check for data validity - to be called AFTER fs_mount() */
static void fs_init_error_stub() { // helper function to enter host interface mode for restoring dataset
    oled.println(F("Use the PC companion software to reupload device data."));
    oled_set_text_xy(0, OLED_TEXT_LINES - 1);
    oled.print(F("Press "));
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("OK")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled.print(F(" to continue."));

    ui_title("FS error");

    oled.display(); io_buzz_error();
    
    io_wait_button(BTN_OK_PIN);
    host_interface();
}

/* network information - we don't want to use C++ vectors and friends here as we're working with relatively little memory */
network_t* fs_networks = NULL; // array of network information
size_t fs_networks_count = 0; // number of networks

/* helper function to (re)allocate fs_networks space */
static void fs_networks_alloc(size_t count) {
    if(fs_networks) {
        /* clean up fs_networks beforehand */
        for(size_t i = 0; i < fs_networks_count; i++) {
            /* deallocate each entry's associated info */
            free(fs_networks[i].stops);
            free(fs_networks[i].stops_off);
        }

        if(count == fs_networks_count) memset(fs_networks, 0, count * sizeof(network_t)); // we can just reuse the previously allocated space after clearing it
        else { // deallocate fs_networks
            free(fs_networks);
            fs_networks = NULL;
        }
    }

    if(!fs_networks) {
        fs_networks = (network_t*) calloc(count, sizeof(network_t)); // allocate and clear memory for fs_networks
        assert(fs_networks); // assert makes sure the condition is satisfied (fs_networks is not null in this case) and aborts execution if the condition is not satisfied
    }
    fs_networks_count = count; // set count
}

/* helper function to read string up to delimiter, discarding any extra characters */
static size_t fs_read_string_until(File& file, char delimiter, char* str, size_t maxlen) { // NOTE: maxlen is maximum string length (i.e. buffer length - 1)
    size_t read_len = 0; // number of bytes read
    for(size_t i = 0; file.available(); i++) {
        char c = file.read(); // read a single character
        if(c == delimiter) break; // we've reached the delimiter

        if(i < maxlen) str[read_len++] = c; // add character to buffer if we still can
    }

    str[read_len] = '\0'; // null terminate
    return read_len;
}

/* helper function to read networks file */
static void fs_networks_init() {
    /* open networks file */
    File file = SPIFFS.open("/networks");
    if(!file || file.isDirectory()) {
        ui_clear_content();
        oled.println(F("Cannot open /networks"));
        fs_init_error_stub();
    }

    /* count number of networks */
    size_t count = fs_count_newlines(file);

    fs_networks_alloc(count); // allocate fs_networks

    /* actually read networks file */
    network_t* network = fs_networks; // network info struct to store data
    while(file.available()) {
        fs_read_string_until(file, ':', network->id, 2); // read network_id
        char datestamp[9]; fs_read_string_until(file, ':', datestamp, 8); // read datestamp
        unsigned int year, month, date; sscanf(datestamp, "%4u%2u%2u", &year, &month, &date); network->year = year; network->month = month; network->date = date; // parse datestamp (we need to create intermediate variables as sscanf expects unsigned int*)
        fs_read_string_until(file, '\n', network->name, 21); // read network_name
        network++; // increment to next entry
    }

    file.close(); // close networks file once we're done
}

/* helper function to read stops file for the specified network */
static void fs_stops_init(network_t* network) {
    char buf[33]; // for stops file path

    /* open stops file */
    sprintf_P(buf, PSTR("/%s/stops"), network->id); // construct path
    File file = SPIFFS.open(buf);
    if(!file || file.isDirectory()) {
        ui_clear_content();
        oled.println(F("Cannot open stops"));
        oled.print(F("file of network ")); oled.println(network->id);
        fs_init_error_stub();
    }

    /* read stops file */
    if(network->stops) free(network->stops); // deallocate if needed
    network->stops = (uint8_t*) malloc(file.size()); // allocate space for storing stops
    assert(network->stops);
    file.read(network->stops, file.size()); // read stops file contents to the buffer we just allocated

    file.close();

    /* open stops.map file */
    strcpy(&buf[strlen(buf)], ".map"); // add .map to path
    file = SPIFFS.open(buf);
    if(!file || file.isDirectory()) {
        ui_clear_content();
        oled.println(F("Cannot open stops.map"));
        oled.print(F("file of network ")); oled.println(network->id);
        fs_init_error_stub();
    }

    /* read stops.map file */
    network->stops_count = file.size() >> 2; // >> 2 is more efficient than divide by 4
    if(network->stops_off) free(network->stops_off);
    network->stops_off = (uint32_t*) malloc(network->stops_count << 2);
    assert(network->stops_off);
    file.read((uint8_t*)network->stops_off, network->stops_count << 2);

    file.close();
}

/* helper function to read routes of a network */
static void fs_routes_init(network_t* network) {
    char path_buf[33]; // for route data file paths

    /* deallocate routes if needed */
    if(network->routes) {
        for(size_t i = 0; i < network->routes_count; i++) free(network->routes[i]); // free individual route data structs
        free(network->routes); // and free the routes table too
        network->routes = NULL;
    }

    /* open routes list */
    sprintf_P(path_buf, PSTR("/%s/routes"), network->id);
    File f_routes = SPIFFS.open(path_buf);
    if(!f_routes || f_routes.isDirectory()) {
        ui_clear_content();
        oled.println(F("Cannot open routes"));
        oled.print(F("file of network ")); oled.println(network->id);
        fs_init_error_stub();
    }

    /* count number of routes on the network and allocate routes table */
    network->routes_count = fs_count_newlines(f_routes);
    network->routes = (route_t**) malloc(network->routes_count * sizeof(route_t*));
    assert(network->routes); // ensure malloc doesn't fail
    
    /* go through each route */
    size_t data_base_len = sprintf_P(path_buf, PSTR("/%s/route_data/"), network->id); // path_buf now stores the base route_data path
    for(size_t route_idx = 0; route_idx < network->routes_count; route_idx++) {
        /* read route number */
        char route_num[6]; size_t route_num_len = fs_read_string_until(f_routes, '\n', route_num, 5);
        strcpy(&path_buf[data_base_len], route_num);
        size_t route_file_idx = data_base_len + route_num_len + 1; path_buf[route_file_idx - 1] = '/'; // add slash to make it base of route's route_data entry

        /* open route data files */
        strcpy(&path_buf[route_file_idx], "info"); File f_info = SPIFFS.open(path_buf); // info
        if(!f_info || f_info.isDirectory()) {
            ui_clear_content();
            oled.println(F("Cannot open info file"));
            oled.print(F("of route ")); oled.println(route_num);
            fs_init_error_stub();
        }
        size_t info_size = f_info.size();
        strcpy(&path_buf[route_file_idx], "seq0"); File f_seq0 = SPIFFS.open(path_buf); // seq0
        if(!f_seq0 || f_seq0.isDirectory()) {
            f_info.close(); // close any files we've already opened
            ui_clear_content();
            oled.println(F("Cannot open seq0 file"));
            oled.print(F("of route ")); oled.println(route_num);
            fs_init_error_stub();
        }
        size_t seq0_size = f_seq0.size();
        strcpy(&path_buf[route_file_idx], "seq1"); File f_seq1 = SPIFFS.open(path_buf); // seq1
        if(!f_seq1 || f_seq1.isDirectory()) {
            f_info.close(); f_seq0.close();
            ui_clear_content();
            oled.println(F("Cannot open seq1 file"));
            oled.print(F("of route ")); oled.println(route_num);
            fs_init_error_stub();
        }
        size_t seq1_size = f_seq1.size();

        /* allocate route_t struct for our route */
        route_t* route = (route_t*) malloc(sizeof(route_t) + info_size + seq0_size + seq1_size); // also allocate space for file contents
        assert(route); 
        network->routes[route_idx] = route;

        /* copy route number */
        memcpy(route->num, route_num, route_num_len + 1);

        /* read info file and extract information */
        f_info.read(&route->buf[0], info_size - 1); route->buf[info_size - 1] = '\0'; // skip reading the ending newline character (which we'll set to NUL anyway)
        route->name = (char*) route->buf; // route name at the beginning
        size_t i = 0;
        for(; i < info_size; i++) { // find end of route name
            if(route->buf[i] == '\n') {
                route->buf[i] = '\0';
                break;
            }
        }
        route->dir0 = (char*) &route->buf[++i];
        for(; i < info_size; i++) { // find end of dir0 name
            if(route->buf[i] == '\n') {
                route->buf[i] = '\0';
                break;
            }
        }
        route->dir1 = (char*) &route->buf[++i];
        f_info.close();

        /* read seq0 file */
        f_seq0.read(&route->buf[info_size], seq0_size);
        route->seq0 = (uint16_t*) &route->buf[info_size];
        route->seq0_count = seq0_size >> 1; // divide by 2 (bytes per entry)
        f_seq0.close();

        /* read seq1 file */
        f_seq1.read(&route->buf[info_size + seq0_size], seq1_size);
        route->seq1 = (uint16_t*) &route->buf[info_size + seq0_size];
        route->seq1_count = seq1_size >> 1; // divide by 2 (bytes per entry)
        f_seq1.close();
    }
    assert(!f_routes.available()); // make sure that we've read everything

    f_routes.close(); // close routes file after we're all done
    
}

/* read SPIFFS partition and check for data validity - to be called AFTER fs_mount() */
void fs_init() {
    fs_networks_init();
    
    /* initialise each network */
    network_t* network = fs_networks;
    for(size_t i = 0; i < fs_networks_count; i++, network++) {
        fs_stops_init(network);
        fs_routes_init(network);
    }
}
