/* OUTPUT FILE SYSTEM INFO */

#include <host_cmds.h>
#include <cmd_handlers/helpers.h>
#include <SPIFFS.h>

/* show file system information (human-readable) */
void host_cmd_fsinfo() {
    Serial.print(F("Total : ")); Serial.print(SPIFFS.totalBytes()); PRINT_NEWLINE;
    Serial.print(F("Used  : ")); Serial.print(SPIFFS.usedBytes()); PRINT_NEWLINE;
}

/* show file system information (raw) */
void host_cmd_fsinfo_raw() {
    host_cmd_write32(SPIFFS.totalBytes());
    host_cmd_write32(SPIFFS.usedBytes());
}