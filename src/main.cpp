#include <Arduino.h>
#include <io.h>
#include <oled.h>
#include <ui.h>
#include <host.h>
#include <filesystem.h>
#include <gps.h>
#include <info.h>

void setup() {
    io_init(); // initialise GPIO pins and serial communication
    oled_init(); // initialise OLED display
    ui_display_splash();
    fs_mount(); fs_init(); // initialise (mount) SPIFFS, then read its data
    gps_update(); // run this so we can be sure we have GPS ready to go

    io_buzz_success(); // indicate everything's good to go
}

/* main menu */
enum main_menu_option_t {
    OPT_START_TRACKING = 0,
    OPT_VIEW_DATA,
    OPT_INTERFACE,
    OPT_REFORMAT,
    OPT_ABOUT
};
main_menu_option_t main_menu_last_sel = OPT_START_TRACKING; // last selected option
main_menu_option_t main_menu() {
    ui_title(BRAND_NAME " " STR(VER_MAJOR) "." STR(VER_MINOR));
    return (main_menu_last_sel = (main_menu_option_t) ui_display_menu((int) OPT_START_TRACKING, (int) OPT_ABOUT, [](void* ctx, int idx) {
        switch((main_menu_option_t) idx) {
            case OPT_START_TRACKING: return "Start tracking";
            case OPT_VIEW_DATA: return "View stored data";
            case OPT_INTERFACE: return "Host interface";
            case OPT_REFORMAT: return "Reformat device";
            case OPT_ABOUT: return "About";
            default: return ""; // so compiler doesn't complain
        }
    }, NULL, main_menu_last_sel));
}

/* track route */
void track_route() {
    /* select network */
select_network:
    network_t* network = ui_menu_network(true);
    if(!network) return;

    /* select route */
select_route:
    route_t* route = ui_menu_route(network, true);
    if(!route) goto select_network; // user selected to go back

    /* select route direction */
select_dir:
    int direction = ui_menu_route_dir(route, true);
    if(direction == -1) goto select_route;

    /* select destination stop */
select_stop:
    size_t dest_stop_idx;
    stop_t* dest_stop = ui_menu_stop(network, route, direction, true, &dest_stop_idx, -1, "Select destination");
    if(!dest_stop) goto select_dir;

    /* find nearest stop along route to initialise on */
    ui_clear_content();
    oled.setCursor((OLED_WIDTH - 16 * OLED_FONT_WIDTH) / 2, OLED_FONT_HEIGHT + (OLED_HEIGHT - 2 * OLED_FONT_HEIGHT) / 2);
    oled.print(F("Locating stop...")); oled.display();
    gps_update();
    size_t seq_count; uint16_t* seq = fs_get_seq(route, direction, seq_count); // get stop sequence
    size_t nearest_stop_idx = gps_find_nearest_stop(network, seq, seq_count);
    stop_t* nearest_stop = fs_get_stop(network, seq[nearest_stop_idx]);

    /* track our journey */
    bool led_status_state = true; // state of the status LED (used for toggling it on and off when approaching stop)
    while(!digitalRead(BTN_OK_PIN)) {
        /* move to next stop if needed */
        // float nearest_stop_dist = gps_distance_sq(nearest_stop);
        // if(nearest_stop_idx < seq_count - 1 && gps_distance_sq(fs_get_stop(network, seq[nearest_stop_idx + 1])) < nearest_stop_dist)
        //     nearest_stop_idx++;
        // else if(nearest_stop_idx > 0 && gps_distance_sq(fs_get_stop(network, seq[nearest_stop_idx - 1])) < nearest_stop_dist)
        //     nearest_stop_idx--;
        size_t search_stop_start = nearest_stop_idx; // start index of nearest stop search
        size_t search_stop_count = dest_stop_idx - nearest_stop_idx + 1; // number of stops to cover in our nearest stop search 
        if(search_stop_start && search_stop_count < seq_count) { // extend back to the stop before the current one if possible
            search_stop_count++;
            search_stop_start--;
        }
        if(dest_stop_idx < seq_count - 1) search_stop_count++; // also extend to stop after the destination if possible
        nearest_stop_idx = gps_find_nearest_stop(network, seq, search_stop_count, search_stop_start);

        /* sound buzzer and flash status LED if we're getting close to our destination */
        if((int)dest_stop_idx - (int)nearest_stop_idx <= UI_STOP_CDOWN_ALERT) {
            BUZZER_ON;
            digitalWrite(LED_STATUS_PIN, (led_status_state) ? LED_ACTIVE : LED_INACTIVE);
            led_status_state = !led_status_state;
        } else {
            BUZZER_OFF;
            LED_STATUS_ON;
        }

        ui_display_stop_cdown(network, route, direction, dest_stop_idx, nearest_stop_idx); // display stop countdown
        
        while(!gps_loop() && !digitalRead(BTN_OK_PIN)); // wait until we receive new data (or user presses OK)
    }
    LED_STATUS_OFF; LED_GPS_OFF;
    BUZZER_OFF;
    io_wait_button(BTN_OK_PIN); // wait for button to be released
}

/* view stored data */
void view_data() {
    /* select network */
select_network:
    network_t* network = ui_menu_network(true);
    if(!network) return;

    /* show network info */
    int network_info_sel = 1; 
network_info:
    ui_title(network->name);
    ui_clear_content();
    oled.printf_P(PSTR("Created: %02u-%02u-%04u"), network->date, network->month, network->year); oled.println();
    oled.printf_P(PSTR("Stop:%5u  Route:%3u"), network->stops_count, network->routes_count);

    /* show menu to allow user to either view stops or view routes */
    network_info_sel = ui_display_menu(0, 2, [](void* ctx, int idx) ->const char* {
        switch(idx) {
            case 0: return "<-- Back";
            case 1: return "View stops";
            case 2: return "View routes";
            default: return "";
        }
    }, NULL, network_info_sel, 3, OLED_TEXT_LINES - 3);
    if(!network_info_sel) goto select_network; // go back to network selection

    if(network_info_sel == 1) {
        /* view stops */
        size_t stop_idx;
        stop_t* stop = ui_menu_stop(network, true, &stop_idx, 0, "Stops");
        if(!stop) goto network_info; // user chose to go back

        ui_display_stop_info(network, stop_idx); // display info about selected stop
        goto network_info; // go back to network information screen
    } else {
        /* view routes */
select_route:
        route_t* route = ui_menu_route(network, true, "Routes");
        if(!route) goto network_info; // user selected to go back

        /* select route direction */
select_dir:
        char buf[OLED_CHARS_PER_LINE + 1]; snprintf_P(buf, OLED_CHARS_PER_LINE + 1, PSTR("%s directions"), route->num);
        int direction = ui_menu_route_dir(route, true, buf);
        if(direction == -1) goto select_route;

        /* select stop */
select_stop:
        size_t stop_idx;
        stop_t* stop = ui_menu_stop(network, route, direction, true, &stop_idx, 0, "Stops");
        if(!stop) goto select_dir;
        
        stop_idx = fs_seqidx_to_idx(route, direction, stop_idx); assert(stop_idx != -1); // convert sequence index returned above to network stop index
        ui_display_stop_info(network, stop_idx); // display info about selected stop
        goto network_info;
    }
}

/* reformat device */
void reformat_device() {
    ui_title("FS reformat");
    ui_clear_content();
    oled.println(F("ALL data stored on"));
    oled.println(F("this device will be"));
    oled.println(F("lost!"));
    oled.print(F("Press ")); int x = oled.getCursorX();
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F(" UP ")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); oled.println(F(" to proceed"));
    oled.setCursor(x, oled.getCursorY());
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("DOWN")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); oled.println(F(" to cancel."));

    oled.display();

    bool up, down; io_wait_buttons(&up, &down, NULL); // wait until either UP or DOWN Is pressed

    if(up) {
        if(!fs_format()) {
            /* formatting failed */
            ui_clear_content();
            oled.println(F("Reformatting failed."));
            oled.println(F("Device halted."));
            oled.display();

            io_buzz_error();
            while(1);
        } else {
            fs_mount();

            ui_clear_content();
            oled.println(F("Reformatting done."));
            oled.println(F("Use the PC companion"));
            oled.println(F("software to reupload"));
            oled.println(F("data to the device."));
            oled_set_text_xy(0, OLED_TEXT_LINES - 1);
            oled.print(F("Press ")); oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("OK")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); oled.print(F(" to continue."));
            io_buzz_success();
            oled.display();
            io_wait_button(BTN_OK_PIN);

            host_interface(); // since we've wiped our device, we'll send it into host interface mode
        }
    }
}

/* about screen */
void about() {
    ui_display_about();
    io_wait_button(BTN_OK_PIN);
}

void loop() {
    switch(main_menu()) {
        case OPT_START_TRACKING: track_route(); break;
        case OPT_VIEW_DATA: view_data(); break;
        case OPT_INTERFACE: host_interface(); break;
        case OPT_REFORMAT: reformat_device(); break;
        case OPT_ABOUT: about(); break;
        default: break;
    }
}
