#include <filesystem.h>

/* get stop information given stop index in network */
stop_t* fs_get_stop(network_t* network, size_t idx) {
    if(idx >= network->stops_count) return NULL;
    return (stop_t*) ((uintptr_t) network->stops + network->stops_off[idx]);
}

/* get stop index given stop information struct in network */
int fs_get_stop_index(network_t* network, stop_t* stop) {
    for(size_t i = 0; i < network->stops_count; i++) {
        if(fs_get_stop(network, i) == stop) return i;
    }

    return -1;
}

/* get stop sequence list */
uint16_t* fs_get_seq(route_t* route, bool dir, size_t& count) {
    count = (dir) ? route->seq1_count : route->seq0_count;
    return (dir) ? route->seq1 : route->seq0;
}

/* convert stop sequence index to network stop index */
int fs_seqidx_to_idx(route_t* route, bool dir, int seq_idx) {
    size_t seq_count; uint16_t* seq = fs_get_seq(route, dir, seq_count);
    if(seq_idx >= seq_count) return -1;
    return seq[seq_idx];
}
