#include <host.h>
#include <host_cmds.h>
#include <io.h>
#include <limits.h>
#include <ui.h>

#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

/* read command byte from Serial */
char host_read_cmd_byte(bool &args_available) {
    while(1) { // so we can return back to reading command
        Serial.print(HOST_PROMPT); // print prompt character to indicate that we're ready to receive command

        /* get command byte */
        char cmd = '\0'; // command byte
        while(cmd != HOST_ABORT_INPUT) { // read until we exit manually (got argument separator character) or we get an input abortion
            while(!Serial.available()); // wait until there's something to be read
            char c = Serial.read(); // read the byte that came from the host computer

            if(c == HOST_ABORT_INPUT) break; // go back to beginning
            if(c == HOST_NEWLINE || c == HOST_ARG_DELIMITER) {
                /* we're done here */
                args_available = (c == HOST_ARG_DELIMITER); // return if arguments are available after the command
                return cmd; // and return the command itself
            }

            cmd = c; // save command byte (so we only consider the last one we received)
        }
    }
}

/* read command arguments from Serial */
size_t host_read_cmd_args(uint8_t* buf, size_t maxlen) { // returns INT_MAX if input is aborted
    /* read bytes from Serial until input is completed or aborted */
    size_t idx = 0; // buffer index
    while(1) {
        while(!Serial.available());
        uint8_t c = Serial.read();

        switch(c) {
            case HOST_NEWLINE: return idx; // input finished
            case HOST_ABORT_INPUT: return INT_MAX; // input aborted
            default:
                if(c == HOST_ARG_DELIMITER) c = '\0'; // convert all delimiters to null
                if(idx < maxlen) buf[idx++] = c; // save to buffer if there's still space
                break;
        }
    }
}

/* helper to trim / suffix from path */
#ifdef HOST_CMD_TRIM_PATHS
void host_cmd_trim_path(char* path) {
    for(int i = strlen(path) - 1; i > 0; i--) { // spare the first character
        if(path[i] == '/') path[i] = '\0'; // trim the path back and remove the / suffix
        else return; // no more to remove
    }
}
#else
#define host_cmd_trim_path(path) // dummy
#endif

/* draw host interface UI */
void host_draw_ui() {
    ui_title("Host interface");
    ui_clear_content();
    oled.println(F("Connect device to PC and use PC companion software to sync data"));
    oled_set_text_xy(0, OLED_TEXT_LINES - 1);
    oled.print(F("Press "));
    oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); oled.print(F("RESET")); oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled.println(F(" to exit."));
    oled.display();
}

/* helper function to hard reset the ESP32 (code from https://github.com/espressif/arduino-esp32/issues/1270#issuecomment-427597713) */
void host_reset() {
    esp_task_wdt_init(1, true);
    esp_task_wdt_add(NULL);
    while(1);
}

/* run host interface on Serial until exited by command or reset button */
void host_interface() {
    host_draw_ui(); // draw UI (this is to be redrawn by command handlers if they use the screen)

    while(1) { // continue running until we exit
        LED_HOST_ON; // turn host interface indicator on

        /* read command and its associated arguments */
        char cmd; // command byte
        uint8_t cmd_args[HOST_CMD_ARGS_LEN + 1]; // create command arguments buffer
        uint8_t cmd_args_len; // received arguments length
        while(1) {
            bool args_available; cmd = host_read_cmd_byte(args_available); // read command byte
            if(args_available) {
                cmd_args_len = host_read_cmd_args(cmd_args, HOST_CMD_ARGS_LEN);
                if(cmd_args_len == INT_MAX) continue;
                cmd_args[cmd_args_len] = '\0'; // null terminate command args buffer for convenience
            } else cmd_args_len = 0; // no arguments received
            break; // we're all done with reading commands here
        }

        LED_HOST_OFF; // turn host interface indicator off to indicate we're processing commands

        /* handle commands */
        switch(cmd) {
            case HOST_CMD_FWINFO:
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_fwinfo)
#endif
                    host_cmd_fwinfo();
                break;
            case HOST_CMD_FSINFO:
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_fsinfo)
#endif
                    host_cmd_fsinfo();
                break;
            case HOST_CMD_FSINFO_RAW:
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_fsinfo_raw)
#endif
                    host_cmd_fsinfo_raw();
                break;
            case HOST_CMD_LISTFS:
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_listfs)
#endif
                    host_cmd_listfs();
                break;
            case HOST_CMD_LISTFS_RAW:
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_listfs_raw)
#endif
                    host_cmd_listfs_raw();
                break;
            case HOST_CMD_HEXDUMP:
                host_cmd_trim_path((char*) cmd_args);
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_hexdump)
#endif
                    host_cmd_hexdump((char*) cmd_args);
                break;
            case HOST_CMD_READ_IHEX:
                host_cmd_trim_path((char*) cmd_args);
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_read_ihex)
#endif
                    host_cmd_read_ihex((char*) cmd_args);
                break;
            case HOST_CMD_READ_RAW:
                host_cmd_trim_path((char*) cmd_args);
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_read_ihex)
#endif
                    host_cmd_read_raw((char*) cmd_args);
                break;
            case HOST_CMD_WRITE:
                host_cmd_trim_path((char*) cmd_args);
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_write)
#endif
                    host_cmd_write((char*) cmd_args);
                break;
            case HOST_CMD_DELETE:
                host_cmd_trim_path((char*) cmd_args);
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_delete)
#endif
                    host_cmd_delete((char*) cmd_args);
                break;
            case HOST_CMD_REFORMAT:
#ifdef HOST_CMDS_WEAK_HANDLERS
                if(host_cmd_reformat)
#endif
                    host_cmd_reformat();
                break;
            case HOST_CMD_EXIT: host_reset(); // reboot board (easy way out)
            default: break; // invalid command
        }
    }

    host_reset(); // reboot board (easy way out)
}