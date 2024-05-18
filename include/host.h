/* HOST COMPUTER INTERFACE */

#ifndef _HOST_H
#define _HOST_H

#include <Arduino.h>

/* special characters in host-device communication protocol */
#define HOST_NEWLINE                        '\n'    // newline character
#define HOST_ARG_DELIMITER                  ':'     // argument delimiter
#define HOST_PROMPT                         '>'     // prompt character
#define HOST_ABORT_INPUT                    '\t'    // abort command input character

#define HOST_CMD_ARGS_LEN                   64      // maximum command arguments buffer length

#define HOST_CMD_TRIM_PATHS // uncomment to trim / suffix from paths

/* draw host interface UI */
void host_draw_ui();

/* run host interface until exited by command or reset button */
void host_interface();

#endif
