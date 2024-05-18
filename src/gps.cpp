#include <gps.h>
#include <ArduinoNmeaParser.h>
#include <io.h>
#include <ui.h>

uint32_t gps_t_last = 0; // timestamp of last byte read from the GPS module

/* GPS geolocation fix (RMC) data */
volatile uint32_t gps_rmc_update_timestamp = 0; // timestamp (millis()) of last RMC update
float gps_rmc_lat = NAN, gps_rmc_lon = NAN; // GPS latitude and longitude (in radians)

/* helper function to toggle GPS LED */
static bool gps_led_state = false;
static void gps_toggle_led() {
    gps_led_state = !gps_led_state;
    digitalWrite(LED_GPS_PIN, (gps_led_state) ? LED_ACTIVE : LED_INACTIVE);
}

/* NMEA parser library instance */
extern float gps_cos_lat; // from gps_dist.cpp
static ArduinoNmeaParser gps_parser([](nmea::RmcData const rmc) {
    /* RMC update handler */
    gps_toggle_led();

    gps_rmc_update_timestamp = millis();
    gps_rmc_lat = (rmc.is_valid) ? (rmc.latitude * PI / 180.0) : NAN;
    gps_rmc_lon = (rmc.is_valid) ? (rmc.longitude * PI / 180.0) : NAN;

    if(isnan(gps_cos_lat) && !isnan(gps_rmc_lat)) gps_cos_lat = cos(gps_rmc_lat); // calculate cosine of latitude (we only need to do it once since we don't move that far anyway)
}, NULL); // we don't care about GPGGA sentences

/* check for data from GPS module and return whether there's new data */
#ifdef GPS_LOC_FROM_SERIAL
bool gps_loop() {
    LED_GPS_ON; // turn on LED to indicate the device is waiting for data
    Serial.printf_P(PSTR("Current GPS coordinates (rad): %f,%f (cos(lat) = %f)\n"), gps_rmc_lat, gps_rmc_lon, gps_cos_lat); // show current coordinates

    /* prompt for coordinates */
    Serial.print(F("Enter new coordinates copied from Google Maps (or press Enter to skip): "));
    char buf[65]; buf[Serial.readBytesUntil('\n', (char*)buf, 64)] = '\0'; // TODO: find a safer way to do this
    if(buf[0] && buf[strlen(buf) - 1] == '\r') buf[strlen(buf) - 1] = '\0'; // CRLF
    Serial.println(buf); // echo back
    if(!buf[0]) { // short for strlen(buf) == 0
        LED_GPS_OFF;
        return false; // no new data - skip
    }
    sscanf(buf, "%f, %f", &gps_rmc_lat, &gps_rmc_lon);
    gps_rmc_lat = gps_rmc_lat * PI / 180.0; // convert to radians
    gps_rmc_lon = gps_rmc_lon * PI / 180.0; 
    if(isnan(gps_cos_lat)) gps_cos_lat = cos(gps_rmc_lat);

    LED_GPS_OFF; // done
    return true;
}
#else
bool gps_loop() {
    /* check if we have anything to read off the module */
    if(!GPS_SERIAL.available()) {
        if(millis() - gps_t_last > GPS_TIMEOUT) {
            /* timeout - display error message and lock up until module is working again */
            gps_led_state = false; LED_GPS_OFF;

            io_buzz_error();

            uint8_t disp_buf[(OLED_WIDTH * OLED_HEIGHT + 7) / 8]; // display buffer to keep display contents while we display our own stuff - this is only possible with the ESP32's large RAM
            memcpy(disp_buf, oled.getBuffer(), sizeof(disp_buf));

            ui_title("Hardware error");
            ui_clear_content();
            oled.print  (F("The device's MCU is  ")); // the OLED can display up to 21 chars in one row
            oled.print  (F("not receiving data   "));
            oled.print  (F("from the GPS module. "));
            oled.print  (F("Ensure the GPS module"));
            oled.print  (F("is connected properly"));
            oled.print  (F("and try restarting   "));
            oled.print  (F("without the antenna. "));
            oled.display();

            while(!GPS_SERIAL.available()); // wait until GPS module sends data again

            /* GPS module is back now */
            io_buzz();
            memcpy(oled.getBuffer(), disp_buf, sizeof(disp_buf)); // restore display buffer
            oled.display(); // never forget to update!
        } else return false; // if not timing out then we return to do other stuff
    }

    gps_t_last = millis(); // update timestamp
    uint32_t last_timestamp = gps_rmc_update_timestamp; // to check if there's new data
    gps_parser.encode(GPS_SERIAL.read()); // feed character into parser
    return (gps_rmc_update_timestamp != last_timestamp);
}
#endif

/* update GPS data if needed */
void gps_update() {
    if(isnan(gps_rmc_lat) || (millis() - gps_rmc_update_timestamp > GPS_TIMEOUT)) {
        /* data is invalid or it has been a bit too long since we last updated our data */
        do {
            while(!gps_loop());
        } while(isnan(gps_rmc_lat));
        LED_GPS_OFF;
    }
}
