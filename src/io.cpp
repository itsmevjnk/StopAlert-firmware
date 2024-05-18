#include <io.h>

/* initialise devices */
void io_init() {
    Serial.setRxBufferSize(1024); // increase UART buffer size so we can receive large data blocks from host
    Serial.begin(SERIAL_BAUD); // initialise UART for communication with host computer
    Serial.setDebugOutput(false); // disable debug output on UART, which can interfere with host communication

    GPS_SERIAL.begin(GPS_BAUD, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN); // initialise UART for GPS module

    /* initialise pins */
    pinMode(BUZZER_PIN, OUTPUT); digitalWrite(BUZZER_PIN, BUZZER_INACTIVE); // initialise buzzer
    pinMode(LED_GPS_PIN, OUTPUT); digitalWrite(LED_GPS_PIN, LED_INACTIVE); // initialise GPS data indicator LED
    pinMode(LED_STATUS_PIN, OUTPUT); digitalWrite(LED_STATUS_PIN, LED_INACTIVE); // initialise device status indicator LED
    pinMode(LED_HOST_PIN, OUTPUT); digitalWrite(LED_HOST_PIN, LED_INACTIVE); // initialise host interface status indicator LED
    pinMode(BTN_UP_PIN, INPUT_PULLDOWN); pinMode(BTN_DOWN_PIN, INPUT_PULLDOWN); pinMode(BTN_OK_PIN, INPUT_PULLDOWN); // initialise button pins to input with internal pulldown resistor
}

/* wait for a button to be pressed and released, complete with debounce */
void io_wait_button(int pin) {
    while(!digitalRead(pin)); // wait until button is pressed
    io_buzz(); // make sound for feedback
#if BTN_DEBOUNCE_DURATION > 100
    delay(BTN_DEBOUNCE_DURATION - 100); // simple debounce - just wait for long enough until we get a definitive state
#endif
    while(digitalRead(pin)); // wait until button is released
}

/* wait for keypress from one of the specified buttons, then return the buttons' status (non-blocking) */
bool io_wait_buttons_nb(bool* up, bool* down, bool* ok) {
    /* wait for one of the specified buttons to be pressed */
    while(!(
        (up && (*up = digitalRead(BTN_UP_PIN))) ||
        (down && (*down = digitalRead(BTN_DOWN_PIN))) ||
        (ok && (*ok = digitalRead(BTN_OK_PIN)))
    )) return false;
    
    io_buzz(); // make sound for feedback
#if BTN_DEBOUNCE_DURATION > 100
    delay(BTN_DEBOUNCE_DURATION - 100); // simple debounce - just wait for long enough until we get a definitive state
#endif

    /* wait until the pressed button(s) are released */
    while(
        (up && *up && digitalRead(BTN_UP_PIN)) ||
        (down && *down && digitalRead(BTN_DOWN_PIN)) ||
        (ok && *ok && digitalRead(BTN_OK_PIN))
    );

    return true;
}

/* wait for keypress from one of the specified buttons, then return the buttons' status (blocking) */
bool io_wait_buttons(bool* up, bool* down, bool* ok, uint32_t timeout) {
    uint32_t t_start = millis();
    bool result; // result of io_wait_buttons_nb
    while(!(result = io_wait_buttons_nb(up, down, ok)) && (timeout <= 0 || millis() - t_start < timeout));
    return result;
}

/* sound the buzzer for the specified duration */
void io_buzz(int duration) {
    BUZZER_ON;
    delay(duration);
    BUZZER_OFF;
}

/* sound the buzzer to indicate normal error */
void io_buzz_error() {
    io_buzz(250); delay(250);
    io_buzz(250); delay(250);
}

/* sound the buzzer to indicate fatal error */
void io_buzz_fatal() {
    io_buzz(500); delay(250);
    io_buzz(500); delay(250);
    io_buzz(500); delay(250);
}

/* sound the buzzer to indicate success/to confirm option */
void io_buzz_success() {
    io_buzz(350);
}