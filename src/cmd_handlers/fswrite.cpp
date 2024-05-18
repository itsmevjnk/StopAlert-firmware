/* FILE WRITING COMMAND HANDLER */

#include <host_cmds.h>
#include <cmd_handlers/helpers.h>
#include <SPIFFS.h>

/* create/overwrite file with raw contents */
void host_cmd_write(const char* path) {
    /* open file for writing */
    File file = SPIFFS.open(path, "w", true);
    Serial.write((!file) ? 0x58 : 0x40); // status code (success/failure)
    host_cmd_write32(0); // send zeros
    if(!file) return; // opening failed - exit right then and there

    /* receive data blocks */
    bool cont = true; // to indicate that we'll continue receiving data blocks
    do {
        while(!Serial.available());
        uint8_t header = Serial.read(); // get header (0x42 or 0x46 only)
        if(header == 0x46) break; // end file write
        else if(header == 0x42) { // data block contents
            uint8_t buf[256]; // buffer to receive data before writing

            while(!Serial.available());
            size_t len = Serial.read(); // get block size (minus 1)
            uint8_t checksum = header + len; // our calculated checksum (so far)
            
            // Serial.println(len);

            /* read block contents */
            for(size_t i = 0; i <= len; i++) {
                while(!Serial.available());
                buf[i] = Serial.read();
                checksum += buf[i];
            }

            while(!Serial.available()); // wait for checksum to be transmitted
            if(Serial.read() != ((-checksum) & 0xFF)) Serial.write(0x21); // checksum mismatch (but we'll continue receiving data blocks)
            else {
                /* write buffer contents to file */
                len++; // don't forget to increment length as it was passed as len - 1!
                cont = (file.write(buf, len) == len); // if this is successful then we continue; otherwise, we stop here
                Serial.write((cont) ? 0x40 : 0x58); // status code
            }
            host_cmd_write32(file.position()); // output file data position after operation
        }
    } while(cont);

    file.close(); // close file after we're done
}
