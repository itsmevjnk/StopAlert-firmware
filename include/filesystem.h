/* SPIFFS INITIALISATION AND HELPER FUNCTIONS */

#ifndef _FSYS_H
#define _FSYS_H

#include <SPIFFS.h>

/* stop information entry struct - this follows the stops file format */
typedef struct {
    float lat, lon; // latitude and longitude
    char name[]; // stop name (this member does not take up any space)
} __attribute__((packed)) stop_t; // the packed attribute ensures that members do not get jostled around

/* route information struct */
typedef struct {
    char num[6]; // route number

    /* name fields (pointers to strings in the struct's buf array) */
    const char* name; // route_name
    const char* dir0; // dir0_name
    const char* dir1; // dir1_name

    /* stop sequences (also pointers into buf) */
    uint16_t* seq0; // seq0 (read directly from file)
    size_t seq0_count; // number of stops in seq0
    uint16_t* seq1; // seq1 (read directly from file)
    size_t seq1_count; // number of stops in seq1

    uint8_t buf[]; // route data buffer
} route_t;

/* network information struct */
typedef struct {
    char id[3]; // network_id
    uint16_t year; uint8_t month, date; // datestamp
    char name[22]; // network_name
    uint8_t* stops; // stops data buffer
    uint32_t* stops_off; // offsets into stops buffer
    size_t stops_count; // number of stops
    route_t** routes; // ptr to array of ptrs to routes
    size_t routes_count; // number of routes
} network_t;

extern network_t* fs_networks; // array of network information
extern size_t fs_networks_count; // number of networks

/* mount SPIFFS */
void fs_mount();

/* read SPIFFS partition and check for data validity - to be called AFTER fs_mount() */
void fs_init();

/* reformat SPIFFS partition */
bool fs_format();

/* count number of newline characters in a file */
size_t fs_count_newlines(File file);

/* get stop information given stop index in network */
stop_t* fs_get_stop(network_t* network, size_t idx);

/* get stop sequence list */
uint16_t* fs_get_seq(route_t* route, bool dir, size_t& count);

/* get stop index given stop information struct in network */
int fs_get_stop_index(network_t* network, stop_t* stop);

/* convert stop sequence index to network stop index */
int fs_seqidx_to_idx(route_t* route, bool dir, int seq_idx);

#endif
