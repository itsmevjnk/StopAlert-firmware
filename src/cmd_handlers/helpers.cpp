/* COMMAND HANDLER HELPERS */

#include <cmd_handlers/helpers.h>

/* write 16-bit little-endian integer */
void host_cmd_write16(uint16_t data) {
    // Serial.write((uint8_t) data);
    // Serial.write((uint8_t) (data >> 8));
    Serial.write((const uint8_t*) &data, 2); // since ESP32 is little endian, we can just pass data as a buffer
}

/* write 32-bit little-endian integer */
void host_cmd_write32(uint32_t data) {
    // Serial.write((uint8_t) data);
    // Serial.write((uint8_t) (data >> 8));
    // Serial.write((uint8_t) (data >> 16));
    // Serial.write((uint8_t) (data >> 24));
    Serial.write((const uint8_t*) &data, 4); // since ESP32 is little endian, we can just pass data as a buffer
}

/* read 16-bit little-endian integer */
uint16_t host_cmd_read16() {
    uint16_t result = Serial.read();
    result |= Serial.read() << 8;
    return result;
}

/* read 32-bit little-endian integer */
uint32_t host_cmd_read32() {
    uint32_t result = Serial.read();
    result |= Serial.read() << 8;
    result |= Serial.read() << 16;
    result |= Serial.read() << 24;
    return result;
}
