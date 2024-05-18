#include <ui.h>
#include <gps.h>
#include <io.h>

/* run menu to select network */
network_t* ui_menu_network(bool back, const char* title) {
    ui_title(title);

    int idx = ui_display_menu((back) ? -1 : 0, fs_networks_count - 1, [](void* ctx, int idx) -> const char* {
        if(idx == -1) return "<-- Back";
        return fs_networks[idx].name;
    }, NULL, 0);

    if(idx == -1) return NULL;
    return &fs_networks[idx];
}

/* run menu to select route */
route_t* ui_menu_route(network_t* network, bool back, const char* title) {
    ui_title(title);

    char temp_buf[64]; // for composing route names
    void* route_cb_ctx[2] = { // what we'll pass into the callback
        network,
        &temp_buf
    };

    int idx = ui_display_menu((back) ? -1 : 0, network->routes_count - 1, [](void* ctx, int idx) -> const char* {
        if(idx == -1) return "<-- Back";

        void** ctx_array = (void**) ctx; // unpack to array of void*
        route_t* route = ((network_t*) ctx_array[0])->routes[idx];
        snprintf_P((char*) ctx_array[1], 64, PSTR("%s %s"), route->num, route->name); // construct name
        return (const char*) ctx_array[1];
    }, route_cb_ctx, 0);

    if(idx == -1) return NULL;
    return network->routes[idx];
}

/* run menu to select direction of a route */
int ui_menu_route_dir(route_t* route, bool back, const char* title) {
    ui_title(title);

    return ui_display_menu(-1, 1, [](void* ctx, int idx) -> const char* {
        if(idx == -1) return "<-- Back";
        route_t* route = (route_t*) ctx;
        return (idx) ? route->dir1 : route->dir0;
    }, route, 0);
}

/* run menu to select stop in network */
stop_t* ui_menu_stop(network_t* network, bool back, size_t* idx_out, int idx_start, const char* title) {
    ui_title(title);

    int idx; // so the compiler is happy
    if(idx_start == -1) goto find_nearest; // initialise idx_start with nearest stop
select_stop:
    idx = ui_display_menu((back) ? -2 : -1, network->stops_count - 1, [](void* ctx, int idx) -> const char* {
        if(idx == -2) return "<-- Back";
        if(idx == -1) return "[Nearest stop]";

        return fs_get_stop((network_t*) ctx, idx)->name;
    }, network, idx_start);

    if(idx == -2) return NULL; // back
    
    if(idx == -1) {
find_nearest:
        /* get GPS data */
        ui_clear_content();
        oled.setCursor((OLED_WIDTH - OLED_FONT_WIDTH * 19) / 2, (OLED_HEIGHT - OLED_FONT_HEIGHT) / 2); oled.print(F("Getting location..."));
        oled.display();
        gps_update();

        /* find nearest stop */
        ui_clear_content();
        oled.setCursor((OLED_WIDTH - OLED_FONT_WIDTH * 15) / 2, (OLED_HEIGHT - OLED_FONT_HEIGHT) / 2); oled.print(F("Finding stop..."));
        oled.display();
        idx_start = gps_find_nearest_stop(network);

        io_buzz_success();
        goto select_stop; // go back to stop selection
    }

    if(idx_out) *idx_out = idx;
    return fs_get_stop(network, idx);
}

/* run menu to select stop in specified route direction */
stop_t* ui_menu_stop(network_t* network, route_t* route, bool dir, bool back, size_t* seq_idx_out, int idx_start, const char* title) {
    ui_title(title);

    size_t seq_count;
    uint16_t* seq = fs_get_seq(route, dir, seq_count);

    void* stop_cb_ctx[] = {
        network,
        seq
    };

    int idx; // so the compiler is happy
    if(idx_start == -1) goto find_nearest; // initialise idx_start with nearest stop
select_stop:
    idx = ui_display_menu((back) ? -2 : -1, seq_count - 1, [](void* ctx, int idx) -> const char* {
        if(idx == -2) return "<-- Back";
        if(idx == -1) return "[Nearest stop]";

        void** ctx_array = (void**) ctx;
        network_t* network = (network_t*) ctx_array[0];
        uint16_t* seq = (uint16_t*) ctx_array[1];

        return fs_get_stop(network, seq[idx])->name;
    }, stop_cb_ctx, idx_start);

    if(idx == -2) return NULL; // back
    
    if(idx == -1) {
find_nearest:
        /* get GPS data */
        ui_clear_content();
        oled.setCursor((OLED_WIDTH - OLED_FONT_WIDTH * 19) / 2, (OLED_HEIGHT - OLED_FONT_HEIGHT) / 2); oled.print(F("Getting location..."));
        oled.display();
        gps_update();

        /* find nearest stop */
        ui_clear_content();
        oled.setCursor((OLED_WIDTH - OLED_FONT_WIDTH * 15) / 2, (OLED_HEIGHT - OLED_FONT_HEIGHT) / 2); oled.print(F("Finding stop..."));
        oled.display();
        idx_start = gps_find_nearest_stop(network, seq, seq_count);

        io_buzz_success();
        goto select_stop; // go back to stop selection
    }

    if(seq_idx_out) *seq_idx_out = idx;
    return fs_get_stop(network, seq[idx]);
}

/* display information about a stop */
void ui_display_stop_info(network_t* network, size_t idx, bool display_routes) {
    stop_t* stop = fs_get_stop(network, idx); // resolve stop information struct

start:
    ui_title("Stop information");
    
    ui_clear_content();
    oled.println(stop->name);
    ui_display_coordinates(stop);

    oled.display();

    /* user interaction */
    if(!display_routes) {
        oled.setCursor(0, OLED_HEIGHT - OLED_FONT_HEIGHT);
        oled.print(F("Press "));
        oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("OK")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        oled.print(F(" to return"));
        io_wait_button(BTN_OK_PIN);
    } else {
        /* get routes associated with this stop (TODO: figure out better way to do this) */
        route_t** routes = (route_t**) malloc(network->routes_count * sizeof(route_t*)); assert(routes); // list of routes
        uint8_t* dirs = (uint8_t*) malloc((network->routes_count + 7) >> 3); assert(dirs); // direction bitmap for each route (>> 3 is the equivalent of / 8)
        uint16_t* seq_idxs = (uint16_t*) malloc(network->routes_count * sizeof(uint16_t)); assert(seq_idxs); // sequence indices of our stop in the found routes
        size_t routes_cnt = 0; // number of routes found
        for(size_t i = 0; i < network->routes_count; i++) {
            routes[routes_cnt] = NULL; // use routes[routes_cnt] as flag for whether a new route was found
            route_t* route = network->routes[i];
            
            /* check seq0 (dir = false) */
            for(size_t j = 0; j < route->seq0_count; j++) {
                if(route->seq0[j] == idx) {
                    routes[routes_cnt] = route;
                    seq_idxs[routes_cnt] = j;
                    dirs[routes_cnt >> 3] &= ~(1 << (routes_cnt & 0b111)); // & 0b111 is the equivalent of % 7
                    break;
                }
            }
            if(routes[routes_cnt]) routes[++routes_cnt] = NULL;

            /* check seq1 (dir = true) */
            for(size_t j = 0; j < route->seq1_count; j++) {
                if(route->seq1[j] == idx) {
                    routes[routes_cnt] = route;
                    seq_idxs[routes_cnt] = j;
                    dirs[routes_cnt >> 3] |= (1 << (routes_cnt & 0b111)); // & 0b111 is the equivalent of % 7
                    break;
                }
            }
            if(routes[routes_cnt]) routes_cnt++;
        }

        /* display list of routes to user */
        int line = oled.getCursorY() / OLED_FONT_HEIGHT; // get current line
        char buf[64]; // character buffer
        void* cb_ctx[] = {
            routes,
            dirs,
            buf
        };
        int selection = ui_display_menu(-1, routes_cnt - 1, [](void* ctx, int idx) -> const char* {
            if(idx == -1) return "<-- Back";
            
            void** ctx_array = (void**) ctx;
            route_t** routes = (route_t**) ctx_array[0]; uint8_t* dirs = (uint8_t*) ctx_array[1]; char* buf = (char*) ctx_array[2]; // unpack context array
            // Serial.println(idx);
            snprintf_P(buf, 64, PSTR("%s %s"), routes[idx]->num, (dirs[idx >> 3] & (1 << (idx & 0b111))) ? routes[idx]->dir1 : routes[idx]->dir0); // construct route name
            return buf;
        }, (void*) cb_ctx, -1, line, OLED_TEXT_LINES - line);

        if(selection == -1) {
            free(routes); free(dirs); free(seq_idxs); // deallocate our buffers
            return; // go back
        }

        route_t* route = routes[selection]; bool dir = (dirs[selection >> 3] & (1 << (selection & 0b111))); uint16_t seq_idx = seq_idxs[selection]; // get user selection
        snprintf_P(buf, 64, PSTR("%s %s"), route->num, (dir) ? route->dir1 : route->dir0); // construct route name for title
        free(routes); free(dirs); free(seq_idxs); // deallocate our buffers (to avoid overflow due to recursion)

        size_t route_seq_idx;
        stop_t* selected_stop = ui_menu_stop(network, route, dir, true, &route_seq_idx, seq_idx, buf);
        if(selected_stop && selected_stop != stop) {
            stop = selected_stop;
            idx = fs_get_stop_index(network, stop);
            assert(idx != -1);
        }
        goto start; // go back to ourselves
    }
}
