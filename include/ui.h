/* user interface functions */

#ifndef _UI_H
#define _UI_H

#include <oled.h> // one would need the OLED library for UI anyway
#include <filesystem.h>

/* print UI title bar */
void ui_title(const char* str);

/* clear UI content area and set text cursor to top left of it */
void ui_clear_content();

/* display menu and allow user to select items */
#define UI_ITEM_SCROLL_PERIOD                           500 // period in milliseconds between character scrolls in selected item
int ui_display_menu(int from, int to, const char* (*name_cb)(void*, int), void* name_cb_ctx, int initial_idx = INT_MIN, int start_line = 1, int lines = (OLED_TEXT_LINES - 1)); // name_cb is the callback for menu item name, and name_cb_ctx is a context pointer that would be passed into name_cb (workaround for passing capturing lambdas as function pointers)

/* truncate string to fit */
char* ui_truncate_line(char* str, const char* ellipsis = "...", size_t maxlen = OLED_CHARS_PER_LINE);

/* display GPS coordinates */
void ui_display_coordinates(float lat, float lon);
void ui_display_coordinates(stop_t* stop); // for convenience

/* run menu to select network */
network_t* ui_menu_network(bool back = false, const char* title = "Select network");

/* run menu to select route */
route_t* ui_menu_route(network_t* network, bool back = false, const char* title = "Select route"); // if back = true and user chooses to go back, this will return NULL

/* run menu to select direction of a route */
int ui_menu_route_dir(route_t* route, bool back = false, const char* title = "Select dir. (towards)"); // if back = true and user chooses to go back, this will return -1

/* run menu to select stop in network */
stop_t* ui_menu_stop(network_t* network, bool back = false, size_t* idx_out = NULL, int idx_start = 0, const char* title = "Select stop");

/* run menu to select stop in specified route direction */
stop_t* ui_menu_stop(network_t* network, route_t* route, bool dir, bool back = false, size_t* seq_idx_out = NULL, int idx_start = 0, const char* title = "Select stop");

/* display splash screen */
void ui_display_splash();

/* display about screen */
void ui_display_about();

/* display stop countdown screen */
#define UI_STOP_CDOWN_SIZE                              3
#define UI_STOP_CDOWN_ALERT                             1 // minimum number of stops away to switch to alert mode (TODO: pass this from some type of central configuration)
void ui_display_stop_cdown(network_t* network, route_t* route, bool dir, size_t seq_dest, size_t seq_nearest);

/* display information about a stop */
void ui_display_stop_info(network_t* network, size_t idx, bool display_routes = true);

#endif