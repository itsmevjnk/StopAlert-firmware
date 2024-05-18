/* LIST FILE SYSTEM CONTENTS */

#include <host_cmds.h>
#include <cmd_handlers/helpers.h>
#include <SPIFFS.h>

/* stub function for listing file system structure (accepts entry output callback function) */
void host_cmd_listfs_stub(void (*cb)(const char*, size_t)) {
    File root = SPIFFS.open("/"); // open root directory
    
    /* iterate through all files in file system */
    File file;
    while(1) {
        if(!(file = root.openNextFile())) break; // no more files to open
        cb(file.path(), file.size());
        file.close(); // close file
    }

    root.close(); // remember to close the root directory!
}

/* list file system structure (human-readable) */
void host_cmd_listfs() {
    host_cmd_listfs_stub([](const char* path, size_t size) { // we can use anonymous functions here for convenience
        // if(path[0] != '/') Serial.print('/'); // add / prefix if it's not in our path yet
        Serial.printf_P(PSTR("%s:%u%c"), path, size, HOST_NEWLINE);
    });
}

/* list file system structure (raw) */
void host_cmd_listfs_raw() {
    host_cmd_listfs_stub([](const char* path, size_t size) { // we can use anonymous functions here for convenience
        host_cmd_write32(size);
        // if(path[0] != '/') Serial.print('/'); 
        Serial.print(path);
        Serial.write('\0'); // null terminate entry
    });

    uint8_t end[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    Serial.write(end, 5); // output termination entry
}