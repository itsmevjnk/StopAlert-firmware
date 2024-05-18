/* OUTPUT FIRMWARE INFO */

#include <host_cmds.h>
#include <cmd_handlers/helpers.h>
#include <info.h>

/* show device firmware information */
void host_cmd_fwinfo() {
    Serial.print(fwinfo);
    Serial.print(HOST_NEWLINE); // don't forget the newline too!
}