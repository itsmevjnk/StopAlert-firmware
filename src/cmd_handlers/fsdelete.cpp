/* FILE DELETION COMMAND HANDLER */

#include <host_cmds.h>
#include <cmd_handlers/helpers.h>
#include <SPIFFS.h>

/* helper to delete file given its File object */
bool host_cmd_delete_file(File file) {
    char buf[34]; // large enough buffer to store path (NOTE: there are conflicting information on whether the limit is 32 chars with or without NUL, so we'll play safe here)    
    strcpy(buf, file.path()); // get file path
    // if(buf[0] != '/') {
    //     /* add / prefix to path */
    //     memmove(&buf[1], buf, 33); // shift everything back
    //     buf[0] = '/';
    // }

    file.close(); // close file before deleting
    return SPIFFS.remove(buf);
}

/* delete file/directory */
void host_cmd_delete(const char* path) {
    /* open file/directory to check if it exists and whether it's a directory */
    // Serial.println(path);
    File file = SPIFFS.open(path);
    if(!file) { // failure
fail:
        Serial.write(0x58);
        return;
    }

    if(!file.isDirectory()) {
        /* this is a file, so we can delete it right away */
        if(!host_cmd_delete_file(file)) goto fail;
    } else {
        /* this is a directory - let's go through each file in it */
        File child;
        while(1) {
            if(!(child = file.openNextFile())) break; // no more files to open
            if(!host_cmd_delete_file(child)) goto fail; // cannot delete this one
        }
        file.close(); // close directory for good measure (though it should be non-existant now)
    }

    Serial.write(0x40); // success
}
