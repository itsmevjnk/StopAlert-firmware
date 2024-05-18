#include <gps.h>

/* calculate distance between current GPS coordinates and a specified coordinates */
float gps_cos_lat = NAN; // cosine of latitude (to be initialised when we get our first data)

float gps_distance_sq(float lat, float lon) { // returns square of multiplier to Earth's radius (fastest for comparing)
    if(isnan(gps_rmc_lat) || isnan(gps_rmc_lon) || isnan(gps_cos_lat) || isnan(lat) || isnan(lon)) return NAN; // invalid data

    float dx = (lon - gps_rmc_lon) * gps_cos_lat;
    float dy = (lat - gps_rmc_lat);
    
    return dx*dx + dy*dy;
}

float gps_distance_sq(stop_t* stop) {
    return gps_distance_sq(stop->lat, stop->lon);
}

float gps_distance_mul(float lat, float lon) { // returns multiplier to Earth's radius
    float dist = gps_distance_sq(lat, lon);
    return isnan(dist) ? NAN : sqrt(dist);
}

float gps_distance_mul(stop_t* stop) {
    return gps_distance_mul(stop->lat, stop->lon);
}

float gps_distance(float lat, float lon) { // returns distance in metres
    float dist = gps_distance_mul(lat, lon);
    return isnan(dist) ? NAN : (GPS_EARTH_RADIUS * dist);
}

float gps_distance(stop_t* stop) {
    return gps_distance(stop->lat, stop->lon);
}

/* find nearest stop from current GPS coordinates (gps_update() may be needed beforehand) */
size_t gps_find_nearest_stop(network_t* network, uint16_t* seq, size_t count, size_t start) {
    /* verify start and count */
    if(seq && count == (size_t)-1) return (size_t)-1; // ensure that seq and count are specified together
    if(!seq) {
        if(count == (size_t)-1) count = network->stops_count - start;
        else if(start + count > network->stops_count) return (size_t)-1; // ensure we don't ask for more stops than the network can have
    }

    /* iterate through stops */
    float d_min = INFINITY; size_t idx_min = 0; // minimum distance to a stop and the index of that stop
    for(size_t i = 0; i < count; i++) {
        size_t idx = start + i;
        float d = gps_distance_sq(fs_get_stop(network, (seq) ? seq[idx] : idx)); // calculate distance
        if(d < d_min) { // record new minimum
            d_min = d;
            idx_min = idx;
        }
    }

    return idx_min;
}