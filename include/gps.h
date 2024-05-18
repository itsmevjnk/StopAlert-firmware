/* GPS MODULE INTERFACE */

#ifndef _GPS_H
#define _GPS_H

#include <filesystem.h>

#define GPS_TIMEOUT                         2000        // timeout duration for data from GPS module (so we can trigger error message)
#define GPS_EARTH_RADIUS                    6378000.0   // Earth radius, used for gps_distance()

// #define GPS_LOC_FROM_SERIAL // uncomment to read location via user prompt from Serial (for debugging only)

/* GPS geolocation fix (RMC) data */
extern volatile uint32_t gps_rmc_update_timestamp; // timestamp (millis()) of last RMC update
extern float gps_rmc_lat, gps_rmc_lon; // GPS latitude and longitude (in radians)

/* check for data from GPS module and return whether there's new data */
bool gps_loop();

/* update GPS data if needed */
void gps_update();

/* calculate distance between current GPS coordinates and a specified coordinates */
float gps_distance_sq(float lat, float lon); // returns square of multiplier to Earth's radius (fastest for comparing)
float gps_distance_mul(float lat, float lon); // returns multiplier to Earth's radius
float gps_distance(float lat, float lon); // returns distance in metres

/* distance functions accepting stop information (for syntax simplicity) */
float gps_distance_sq(stop_t* stop); // returns square of multiplier to Earth's radius (fastest for comparing)
float gps_distance_mul(stop_t* stop); // returns multiplier to Earth's radius
float gps_distance(stop_t* stop); // returns distance in metres

/* find nearest stop from current GPS coordinates (gps_update() may be needed before ) */
size_t gps_find_nearest_stop(network_t* network, uint16_t* seq = NULL, size_t count = (size_t)-1, size_t start = 0); // NOTE: count MUST be specified if seq is specified (for number of stops in sequence starting from start index) - otherwise this will return -1

#endif
