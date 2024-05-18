#include <filesystem.h>
#include <ui.h>
#include <io.h>
#include <host.h>

/* mount SPIFFS */
void fs_mount() {
    if(!SPIFFS.begin(false)) { // attempt to mount SPIFFS without formatting on failure
        /* display error message */
        ui_title("FS mount error");
        ui_clear_content();
        oled.println(F("Failed to mount file system."));

        oled_set_text_xy(0, 4);
        oled.print(F("Press "));
        oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("OK")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        oled.println(F(" to reformat."));

        oled_set_text_xy(0, OLED_TEXT_LINES - 1);
        oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        oled.println(F("ALL DATA WILL BE LOST"));
        oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

        oled.display();
        io_buzz_error(); // make sound to indicate error

        io_wait_button(BTN_OK_PIN); // wait until user press OK button - then we'll format the SPIFFS partition
        
        bool reformat_ok = fs_format(); // reformat partition
        ui_clear_content(); // clear screen so we're ready to show more info
        if(!reformat_ok) { // reformatting failed - display error and hang
            io_buzz_fatal();
            oled.println(F("File system reformatting failed."));
            oled.println(F("Device halted."));
            oled.display();

            while(1);
        } else io_buzz_success();

        /* attempt to remount */
        if(!SPIFFS.begin(false)) { // remounting failed - display error and hang
            io_buzz_fatal();
            ui_title("FS mount error");
            ui_clear_content();
            oled.println(F("File system mounting failed after reformatting."));
            oled.println(F("Device halted."));
            oled.display();

            while(1);
        }
    }
}

/* format SPIFFS partition and return result */
bool fs_format() {
    /* indicate on screen that we're formatting */
    ui_title("FS reformat");
    ui_clear_content();
    oled.setCursor((OLED_WIDTH - OLED_FONT_WIDTH * 14) / 2, (OLED_HEIGHT - OLED_FONT_HEIGHT) / 2); oled.print(F("Please wait..."));
    oled.display();

    return SPIFFS.format(); // format and return result
}

/* count number of newline characters in a file */
size_t fs_count_newlines(File file) {
    size_t pos = file.position(); // get current position
    assert(file.seek(0)); // seek back to beginning
    size_t count = 0;
    while(file.available()) {
        if(file.read() == '\n') count++;
    }
    assert(file.seek(pos)); // seek back to current position
    return count;
}
