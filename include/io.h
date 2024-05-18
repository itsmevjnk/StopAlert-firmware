/* ESP32 ON-BOARD I/O */

#ifndef _IO_H
#define _IO_H

#include <Arduino.h>

#define SERIAL_BAUD                             115200      // baud rate for communication with host (normally 115200)

/* OLED display is connected to hardware I2C pins (SCL/GPIO22, SDA/GPIO21) */
#define OLED_ADDRESS                            0x3C        // OLED display I2C address (0x3C or 0x3D depending on module used)

/* GPS module */
#define GPS_RX_PIN                              26          // TX to board
#define GPS_TX_PIN                              27          // RX to board
#define GPS_SERIAL                              Serial2     // UART interface to be used for GPS (possible options: Serial1, Serial2)
#define GPS_BAUD                                9600        // UART baud rate for GPS module (normally 9600)

/* active buzzer */
#define BUZZER_PIN                              4           // active buzzer output pin
#define BUZZER_ACTIVE_LOW // uncomment to specify that buzzer is active low

/* LEDs */
#define LED_GPS_PIN                             33          // GPS output indicator (flashes when the GPS module sends data over serial to ESP32)
#define LED_STATUS_PIN                          32          // status LED (lights up when tracking location, flashes when approaching stop)
#define LED_HOST_PIN                            25          // host interface indicator (lights up when device is in host interface, turns off while device is receiving/processing commands)
// #define LED_ACTIVE_LOW // uncomment to specify that LEDs are active low

/* buttons - active high (i.e. connected to VCC at one end) */
#define BTN_UP_PIN                              18          // Up button
#define BTN_DOWN_PIN                            19          // Down button
#define BTN_OK_PIN                              23          // OK button

/* active states - do not modify! */
#ifdef BUZZER_ACTIVE_LOW
#define BUZZER_ACTIVE                           LOW
#define BUZZER_INACTIVE                         HIGH
#else
#define BUZZER_ACTIVE                           HIGH
#define BUZZER_INACTIVE                         LOW
#endif
#ifdef LED_ACTIVE_LOW
#define LED_ACTIVE                              LOW
#define LED_INACTIVE                            HIGH
#else
#define LED_ACTIVE                              HIGH
#define LED_INACTIVE                            LOW
#endif

/* macros for controlling LEDs and buzzers */
#define LED_GPS_ON                              digitalWrite(LED_GPS_PIN, LED_ACTIVE)
#define LED_GPS_OFF                             digitalWrite(LED_GPS_PIN, LED_INACTIVE)
#define LED_STATUS_ON                           digitalWrite(LED_STATUS_PIN, LED_ACTIVE)
#define LED_STATUS_OFF                          digitalWrite(LED_STATUS_PIN, LED_INACTIVE)
#define LED_HOST_ON                             digitalWrite(LED_HOST_PIN, LED_ACTIVE)
#define LED_HOST_OFF                            digitalWrite(LED_HOST_PIN, LED_INACTIVE)
#define BUZZER_ON                               digitalWrite(BUZZER_PIN, BUZZER_ACTIVE)
#define BUZZER_OFF                              digitalWrite(BUZZER_PIN, BUZZER_INACTIVE)

#define BTN_DEBOUNCE_DURATION                   50          // button debounce duration in milliseconds

/* initialise onboard I/O devices */
void io_init();

/* wait for a button to be pressed and released, complete with debounce */
void io_wait_button(int pin);

/* sound the buzzer for a specified duration */
void io_buzz(int duration = 100); // default: feedback beep

/* sound the buzzer to indicate normal error */
void io_buzz_error();

/* sound the buzzer to indicate fatal error */
void io_buzz_fatal();

/* sound the buzzer to indicate success/to confirm option */
void io_buzz_success();

/* wait for keypress from one of the specified buttons, then return the buttons' status */
bool io_wait_buttons_nb(bool* up, bool* down, bool* ok); // non-blocking (returns true if button has been pressed)
bool io_wait_buttons(bool* up, bool* down, bool* ok, uint32_t timeout = 0); // blocking

#endif
