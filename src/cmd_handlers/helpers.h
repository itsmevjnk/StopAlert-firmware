/* HELPERS FOR COMMAND HANDLERS */

#ifndef _CMD_HANDLERS_HELPERS_H
#define _CMD_HANDLERS_HELPERS_H

#include <Arduino.h>
#include <host.h>

#define PRINT_NEWLINE                           Serial.print(HOST_NEWLINE) // print newline character

/* write 16-bit little-endian integer */
void host_cmd_write16(uint16_t data);

/* write 32-bit little-endian integer */
void host_cmd_write32(uint32_t data);

/* read 16-bit little-endian integer */
uint16_t host_cmd_read16();

/* read 32-bit little-endian integer */
uint32_t host_cmd_read32();

#endif
