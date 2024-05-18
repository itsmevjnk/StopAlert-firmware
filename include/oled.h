/* SSD1306 OLED DISPLAY */

#ifndef _OLED_H
#define _OLED_H

#include <Adafruit_SSD1306.h>

/* OLED display resolution in pixels */
#define OLED_WIDTH                              128         // OLED display width
#define OLED_HEIGHT                             64          // OLED display height

/* OLED display font dimensions - do not change this either! */
#define OLED_FONT_WIDTH                         6
#define OLED_FONT_HEIGHT                        8

#define OLED_CHARS_PER_LINE                     (OLED_WIDTH / OLED_FONT_WIDTH) // number of characters per line
#define OLED_TEXT_LINES                         (OLED_HEIGHT / OLED_FONT_HEIGHT) // number of text lines (size 1) can be displayed

extern Adafruit_SSD1306 oled; // OLED display object (declared and initialised in oled.cpp)

#define OLED_INIT_CHECKERBOARD // uncomment to display alternating white/black pixels after initialising OLED display

/* initialise OLED display */
void oled_init();

/* helper macro to set text XY coordinates */
#define oled_set_text_xy(x, y)                  oled.setCursor((x) * OLED_FONT_WIDTH, (y) * OLED_FONT_HEIGHT)

#endif
