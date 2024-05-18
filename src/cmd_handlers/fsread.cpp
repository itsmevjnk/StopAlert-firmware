/* FILE READING (DUMPING) COMMAND HANDLERS */

#include <host_cmds.h>
#include <cmd_handlers/helpers.h>
#include <SPIFFS.h>

/* print hexadecimal dump of file */
void host_cmd_hexdump(const char* path) {
    /* check if file exists before opening */
    if(!SPIFFS.exists(path)) {
        Serial.print(path);
        Serial.print(F(" not found"));
        PRINT_NEWLINE;
        return;
    }

    /* open file */
    File file = SPIFFS.open(path, "r");
    if(!file) {
        Serial.print(path);
        Serial.print(F(" cannot be opened"));
        PRINT_NEWLINE;
        return;
    }
    
    /* check if it turns out to be a directory instead */
    if(file.isDirectory()) {
        Serial.print(path);
        Serial.print(F(" is a directory"));
        PRINT_NEWLINE;
        file.close(); // close it before exiting
        return;
    }

    /* produce hex dump */
    uint8_t buf[16]; // file data buffer
    while(file.available()) {
        Serial.printf_P(PSTR("%08X:"), file.position()); // get current file position
        size_t read = file.readBytes((char*)buf, 16); // read up to 16 bytes
        for(size_t i = 0; i < read; i++) Serial.printf_P(PSTR(" %02X"), buf[i]); // output each byte we've read
        PRINT_NEWLINE;
    }

    file.close(); // close file before exiting
}

/* helper to output Intel HEX record */
void host_cmd_print_ihex_record(uint8_t type, uint16_t address, const uint8_t* data, uint8_t data_len) {
    uint8_t checksum = type + data_len + (address >> 8) + (address & 0xFF); // checksum to be outputted at the end
    Serial.printf_P(PSTR(":%02X%04X%02X"), data_len, address, type); // output data length (byte count), address, and record type
    for(uint8_t i = 0; i < data_len; i++, data++) { // output each data byte and add into checksum
        Serial.printf_P(PSTR("%02X"), *data);
        checksum += *data;
    }
    Serial.printf_P(PSTR("%02X%c"), (-checksum) & 0xFF, HOST_NEWLINE); // output checksum and newline character - we're all done here
}

/* provide Intel HEX dump of file */
void host_cmd_read_ihex(const char* path) {
    /* check if file exists before opening */
    if(!SPIFFS.exists(path)) {
        Serial.print(path);
        Serial.print(F(" not found"));
        PRINT_NEWLINE;
        return;
    }

    /* open file */
    File file = SPIFFS.open(path, "r");
    if(!file) {
        Serial.print(path);
        Serial.print(F(" cannot be opened"));
        PRINT_NEWLINE;
        return;
    }
    
    /* check if it turns out to be a directory instead */
    if(file.isDirectory()) {
        Serial.print(path);
        Serial.print(F(" is a directory"));
        PRINT_NEWLINE;
        file.close(); // close it before exiting
        return;
    }

    /* produce Intel HEX dump data records */
    uint8_t buf[16]; // file data buffer
    uint16_t prev_ext = 0x0000; // previous extended address base (upper 16 bits of address) - used to check if we need to insert 0x04 records
    while(file.available()) {
        size_t pos = file.position(); // get current file position
        uint16_t ext = (pos >> 16), off = (pos & 0xFFFF); // extract extended address base and offset from file position

        if(prev_ext != ext) { // segment change - insert 0x04 record
            uint8_t new_ext[] = {(uint8_t)(ext >> 8), (uint8_t)(ext & 0xFF)};
            host_cmd_print_ihex_record(0x04, 0x0000, new_ext, 2);
            prev_ext = ext;
        }

        size_t read = file.readBytes((char*)buf, 16); // read up to 16 bytes
        host_cmd_print_ihex_record(0x00, off, buf, read); // output record for the bytes we've read
    }

    Serial.print(F(":00000001FF")); PRINT_NEWLINE; // output end of file record (which is constant so we can just do a Serial.print() here)
    file.close(); // close file before exiting
}

/* provide raw dump of file */
void host_cmd_read_raw(const char* path) {
    /* check if file exists before opening */
    if(!SPIFFS.exists(path)) {
        Serial.print(path);
        Serial.print(F(" not found"));
        PRINT_NEWLINE;
        return;
    }

    /* open file */
    File file = SPIFFS.open(path, "r");
    if(!file) {
        Serial.print(path);
        Serial.print(F(" cannot be opened"));
        PRINT_NEWLINE;
        return;
    }
    
    /* check if it turns out to be a directory instead */
    if(file.isDirectory()) {
        Serial.print(path);
        Serial.print(F(" is a directory"));
        PRINT_NEWLINE;
        file.close(); // close it before exiting
        return;
    }

    /* output file size */
    uint32_t size = file.size();
    uint8_t checksum = (size >> 24) + (size >> 16) + (size >> 8) + (size & 0xFF); // checksum to be sent at the end
    host_cmd_write32(size);

    /* output file contents */
    while(file.available()) {
        uint8_t data = file.read();
        Serial.write(data);
        checksum += data;
    }

    Serial.write((-checksum) & 0xFF); // send checksum

    file.close(); // close file before exiting
}
