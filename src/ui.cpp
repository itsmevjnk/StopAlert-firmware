#include <ui.h>
#include <io.h>

/* print UI title bar */
void ui_title(const char* str) {
    int old_x = oled.getCursorX(), old_y = oled.getCursorY(); // save current cursor position

    oled.fillRect(0, 0, OLED_WIDTH, OLED_FONT_HEIGHT, SSD1306_WHITE); // clear title bar region (1st line)
    int str_len = strlen(str); if(str_len > OLED_CHARS_PER_LINE) str_len = OLED_CHARS_PER_LINE; // number of chars we'll actually display
    oled.setCursor((OLED_WIDTH - str_len * OLED_FONT_WIDTH) / 2, 0); oled.setTextColor(SSD1306_BLACK); // set cursor to top centre and set text colour to black (on white background)
    for(size_t i = 0; i < str_len; i++) oled.print(str[i]); // print title
    oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // set text colour back to white on black
    oled.setCursor(old_x, old_y); // restore cursor position 
}

/* clear UI content area and set text cursor to top left of it */
void ui_clear_content() {
    oled.fillRect(0, OLED_FONT_HEIGHT, OLED_WIDTH, OLED_HEIGHT - OLED_FONT_HEIGHT, SSD1306_BLACK);
    oled.setCursor(0, OLED_FONT_HEIGHT);
    oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // in case it hasn't been reset
}

/* display menu and allow user to select items */
int ui_display_menu(int from, int to, const char* (*name_cb)(void*, int), void* name_cb_ctx, int initial_idx, int start_line, int lines) {
    if(initial_idx < from) initial_idx = from; // set initial index to 1st item by default
    int disp_start = initial_idx; if(disp_start - lines + 1 < from) disp_start = from; // first index on screen
    int selected = initial_idx; // selected item
    if(to - disp_start + 1 < lines) disp_start = to - lines + 1; if(disp_start < from) disp_start = from; // ensure we always show the specified number of items
    int name_start = 0; // item name's starting index (for scrolling)

    uint32_t t_scroll = millis(); // timestamp of last item text scroll
    bool update = true; // set to update screen
    bool reset_scroll = false; // set to reset scrolling on next update
    while(1) { // continue on until we manually exit (by pressing OK)
        if(update) {
            /* print items */
            oled.fillRect(0, start_line * OLED_FONT_HEIGHT, OLED_WIDTH, lines * OLED_FONT_HEIGHT, SSD1306_BLACK); // clear screen region for menu
            oled_set_text_xy(0, start_line);
            for(int i = disp_start; i < disp_start + lines && i <= to; i++) {
                const char* name = name_cb(name_cb_ctx, i); // get item name
                size_t name_len = strlen(name);
                if(i == selected && name_len > OLED_CHARS_PER_LINE) {
                    /* handle scrolling */
                    if(name_start + OLED_CHARS_PER_LINE > name_len) {
                        reset_scroll = true; // reset scrolling on next scroll cycle
                        name_start--; // so we keep displaying the last scroll
                    }
                    name = &name[name_start]; // scroll
                }
                // Serial.print(i); Serial.print(' '); Serial.println(name);

                /* print item */
                oled_set_text_xy(0, start_line + i - disp_start);
                if(i == selected) {
                    /* highlight selected item */
                    oled.fillRect(0, (start_line + i - disp_start) * OLED_FONT_HEIGHT, OLED_WIDTH, OLED_FONT_HEIGHT, SSD1306_WHITE);
                    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
                }
                for(int j = 0; name[j] && j < OLED_CHARS_PER_LINE; j++) {
                    if(j == OLED_CHARS_PER_LINE - 1 && name[j + 1]) oled.print('>'); // we're on the last character now, and we still have stuff to print out, so let's cut that off with a mini-ellipsis
                    else oled.print(name[j]); // print as usual
                }
                if(i == selected) oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // stop highlighting
            }
            oled.display();

            update = false; // reset
        }

        /* watch for keypresses */
        bool up, down, ok;
        while(!io_wait_buttons_nb(&up, &down, &ok)) {
            if(millis() - t_scroll >= UI_ITEM_SCROLL_PERIOD) {
                t_scroll = millis();
                if(reset_scroll) {
                    name_start = 0;
                    reset_scroll = false;
                } else name_start++; // if we overflow, this will be handled by the displaying code above
                update = true; // update screen on next iteration
                break; // exit loop - the code below will send us back to screen updating
            }
        }

        if(update) continue; // we exited the loop because of an update request
        
        /* handle keypresses */
        if(ok) return selected; // return what the user selected
        if(up) {
            if(selected > from) {
                selected--;
                if(selected < disp_start) disp_start = selected; // scroll list up such that selected item is on top
            } else {
                /* go to last item */
                selected = to;
                disp_start = selected - lines + 1; if(disp_start < from) disp_start = from; // scroll list down such that selected item is on bottom
            }            
            name_start = 0; t_scroll = millis(); // reset scrolling
            update = true; // update screen on next iteration
            reset_scroll = false;
        }
        if(down) {
            if(selected < to) {
                selected++;
                if(selected >= disp_start + lines) disp_start = selected - lines + 1; // scroll list down such that selected item is on bottom
            } else {
                selected = from;
                disp_start = selected; // scroll list up such that selected item is on top
            }
            name_start = 0; t_scroll = millis(); // reset scrolling
            update = true; // update screen on next iteration
        }
    }
}

/* truncate string to fit */
char* ui_truncate_line(char* str, const char* ellipsis, size_t maxlen) {
    if(strlen(str) > maxlen) strcpy(&str[maxlen - strlen(ellipsis)], ellipsis);
    return str;
}

/* display GPS coordinates */
void ui_display_coordinates(float lat, float lon) {
    oled.printf_P(PSTR("Lat. : %11.6f %c\n"), abs(lat / PI * 180.0), (lat < 0) ? 'S' : 'N');
    oled.printf_P(PSTR("Lon. : %11.6f %c\n"), abs(lon / PI * 180.0), (lon < 0) ? 'W' : 'E');
}
void ui_display_coordinates(stop_t* stop) {
    ui_display_coordinates(stop->lat, stop->lon);
}
