#include <ui.h>
#include <info.h>

/* display splash screen */
void ui_display_splash() {
    ui_title(BRAND_NAME " " STR(VER_MAJOR) "." STR(VER_MINOR));
    ui_clear_content();
    oled.setCursor((OLED_WIDTH - 15 * OLED_FONT_WIDTH) / 2, (OLED_HEIGHT - 3 * OLED_FONT_HEIGHT) / 2); oled.println(F("Initialising..."));
    oled.setCursor((OLED_WIDTH - 19 * OLED_FONT_WIDTH) / 2, oled.getCursorY()); oled.println(F("GPS init may take a"));
    oled.setCursor((OLED_WIDTH - 12 * OLED_FONT_WIDTH) / 2, oled.getCursorY()); oled.print(F("few minutes."));
    oled_set_text_xy(0, OLED_TEXT_LINES - 1); oled.print(F("SIT111 3.8HD project"));
    oled.display();
}

/* display about screen */
void ui_display_about() {
    ui_title("About");
    ui_clear_content();
    oled.println(F(BRAND_NAME));
    oled.println(F(" - an ESP32-based bus"));
    oled.println(F("   /tram companion."));
    oled.println(F("Created for SIT111 by"));
    oled.println(F("Thanh Vinh Nguyen"));
    oled.println(F("(223145883/itsmevjnk)"));
    oled_set_text_xy(0, OLED_TEXT_LINES - 1);
    oled.print(F("Press ")); oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("OK")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); oled.print(F(" to return."));
    oled.display();
}

/* display stop countdown screen */
void ui_display_stop_cdown(network_t* network, route_t* route, bool dir, size_t seq_dest, size_t seq_nearest) {
    char buf[OLED_CHARS_PER_LINE + 2]; // character buffer
    size_t seq_count; uint16_t* seq = fs_get_seq(route, dir, seq_count); // get stop sequence of direction

    /* display title */
    snprintf_P(buf, OLED_CHARS_PER_LINE + 2, PSTR("%s %s"), route->num, (dir) ? route->dir1 : route->dir0);
    ui_title(buf);

    ui_clear_content();

    /* display destination stop */
    assert(seq_dest < seq_count); // otherwise we have a much larger problem
    stop_t* stop_dest = fs_get_stop(network, seq[seq_dest]);
    memset(buf, 0, OLED_CHARS_PER_LINE + 2); strncpy(buf, stop_dest->name, OLED_CHARS_PER_LINE + 1); // copy stop name over in case we need to truncate
    ui_truncate_line(buf, ">", OLED_CHARS_PER_LINE - 3); strcpy(&buf[strlen(buf)], " is");
    oled.setCursor((OLED_WIDTH - strlen(buf) * OLED_FONT_WIDTH) / 2, oled.getCursorY()); oled.println(buf);

    /* display number of stops away */
    int stops_away = seq_dest - seq_nearest;
    oled.setTextSize(UI_STOP_CDOWN_SIZE); if(stops_away <= UI_STOP_CDOWN_ALERT) oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // highlight stop count if we're getting close (or we missed the stop)
    oled.printf_P(PSTR("%3u"), abs(stops_away));
    oled.setTextSize(1); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    int x_start = oled.getCursorX() + OLED_FONT_WIDTH; // starting X coordinate for the "stops away" text
    oled.setCursor(x_start, oled.getCursorY() + (UI_STOP_CDOWN_SIZE - 2) * OLED_FONT_WIDTH / 2); oled.print(F("stop")); if(abs(stops_away) != 1) oled.print('s'); // can't forget the plural!
    oled.setCursor(x_start, oled.getCursorY() + OLED_FONT_HEIGHT); oled.print(F("away"));

    /* display the stop we're arriving at */
    stop_t* stop_nearest = fs_get_stop(network, seq[seq_nearest]);
    oled.setCursor((OLED_WIDTH - 16 * OLED_FONT_WIDTH) / 2, (2 + UI_STOP_CDOWN_SIZE) * OLED_FONT_HEIGHT); oled.println(F("Now arriving at:"));
    if(!stops_away) {
        /* highlight stop if it's the one we're arriving at */
        oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        oled.fillRect(oled.getCursorX(), oled.getCursorY(), OLED_WIDTH, OLED_FONT_HEIGHT, SSD1306_WHITE);
    }
    strncpy(buf, stop_nearest->name, OLED_CHARS_PER_LINE + 1); ui_truncate_line(buf, ">", OLED_CHARS_PER_LINE);
    oled.setCursor((OLED_WIDTH - strlen(buf) * OLED_FONT_WIDTH) / 2, oled.getCursorY()); oled.println(buf);
    oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

    /* display footer */
    oled_set_text_xy(0, OLED_TEXT_LINES - 1);
    oled.print(F("Press "));
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("OK")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled.print(F(" to exit"));

    oled.display();
}
