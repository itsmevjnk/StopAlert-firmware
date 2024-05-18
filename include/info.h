/* FIRMWARE INFORMATION */

#ifndef _INFO_H
#define _INFO_H

#define BRAND_NAME                      "StopAlert" // "brand" name of the device
#define VER_MAJOR                       1           // firmware major version number
#define VER_MINOR                       0           // firmware minor version number

/* stringification helper */
#define STR_HELPER(x)                   #x
#define STR(x)                          STR_HELPER(x)

/* firmware information string */
extern const char fwinfo[];

#endif
