/* FILE SYSTEM REFORMAT COMMAND HANDLER */

#include <host_cmds.h>
#include <cmd_handlers/helpers.h>
#include <SPIFFS.h>
#include <io.h>
#include <ui.h>
#include <filesystem.h>

#define HOST_CMD_REFORMAT_CONFIRM_TIMEOUT                   30  // reformat confirmation timeout in seconds

/* reformat file system */
void host_cmd_reformat() {
    /* display warning message */
    ui_clear_content();
    oled.println(F("Host PC wants to wipe this device."));
    oled.print(F("Press "));
    int x = oled.getCursorX(); // get X position so we can go back there on the second line
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F(" UP ")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled.println(F(" to proceed"));
    oled.setCursor(x, oled.getCursorY());
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("DOWN")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled.println(F(" to cancel."));
    oled_set_text_xy(0, OLED_TEXT_LINES - 1); oled.print(F("Cancelling in    sec."));
    oled.display();

    io_buzz(); // bleep to alert user

    /* wait until either Up or Down button is pressed */
    bool up, down;
    for(int i = HOST_CMD_REFORMAT_CONFIRM_TIMEOUT; i > 0; i--) {
        oled_set_text_xy(14, OLED_TEXT_LINES - 1); oled.printf("%2u", i); oled.display(); // update countdown timer
        if(io_wait_buttons(&up, &down, NULL, 1000)) break;
    }
    
    if(up) {
        /* reformat */
        Serial.write(0x2E); // indicate that user confirmed format request

        SPIFFS.end(); // deinitialise file system for reformatting
        if(!fs_format()) {
            /* formatting failed */
            Serial.write(0x58);

            ui_clear_content();
            oled.println(F("Reformatting failed."));
            oled.println(F("Device halted."));
            oled.display();

            io_buzz_error();
            while(1);
        }

        fs_mount(); // reinitialise FS
        io_buzz();
        Serial.write(0x40); // reformat successful
    } else {
        /* cancel */
        Serial.write(0x58);
    }

    host_draw_ui(); // redraw UI
}