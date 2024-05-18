#include <oled.h>
#include <io.h>

Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT); // OLED display object

/* initialise OLED display */
void oled_init() {
    if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) Serial.println(F("[E] OLED init failed")); // display error to serial     
    oled.clearDisplay(); // clear framebuffer (which defaults to Adafruit logo)
#ifdef OLED_INIT_CHECKERBOARD
    for(int y = 0; y < OLED_HEIGHT; y++) {
        for(int x = 0; x < OLED_WIDTH; x++) {
            if(x % 2 + y % 2 == 1) oled.drawPixel(x, y, SSD1306_WHITE); // display pixel if 
        }
    }
#endif
    oled.display(); // send framebuffer to screen
}
