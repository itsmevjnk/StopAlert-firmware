/* HOST SERIAL INTERFACE COMMAND BYTES AND HANDLERS */

#ifndef _HOST_CMDS_H
#define _HOST_CMDS_H

#include <Arduino.h>

// #define HOST_CMDS_WEAK_HANDLERS // uncomment to declare handler functions as weak (allowing them to be left unimplemented without causing compilation errors)

/* show device firmware information */
#define HOST_CMD_FWINFO                         'v' // command byte
void host_cmd_fwinfo(); // handler function (defined in its own file in cmd_handlers/)

/* show file system information (human-readable) */
#define HOST_CMD_FSINFO                         'i'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_fsinfo();

/* show file system information (raw) */
#define HOST_CMD_FSINFO_RAW                     'I'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_fsinfo_raw();

/* list file system structure (human-readable) */
#define HOST_CMD_LISTFS                         'l'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_listfs();

/* list file system structure (raw) */
#define HOST_CMD_LISTFS_RAW                     'L'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_listfs_raw();

/* print hexadecimal dump of file */
#define HOST_CMD_HEXDUMP                        'd'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_hexdump(const char* path);

/* provide Intel HEX dump of file */
#define HOST_CMD_READ_IHEX                      'D'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_read_ihex(const char* path);

/* provide raw dump of file */
#define HOST_CMD_READ_RAW                       'R'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_read_raw(const char* path);

/* create/overwrite file with raw contents */
#define HOST_CMD_WRITE                          'W'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_write(const char* path);

/* delete file/directory */
#define HOST_CMD_DELETE                         'X'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_delete(const char* path);

/* reformat file system */
#define HOST_CMD_REFORMAT                       'Z'
#ifdef HOST_CMDS_WEAK_HANDLERS
__attribute__((weak))
#endif
void host_cmd_reformat();

#define HOST_CMD_EXIT                           'q' // exit host interface mode (no handler functions needed)

#endif
