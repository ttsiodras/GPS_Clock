#include <Wire.h>
#include <TinyGPS.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define DEFINE_GLOBALS
#include "globals.h"

const int BUTTON_PIN = 2;
const int GPS_RX_PIN = 4;
const int GPS_TX_PIN = 3; // unused

// GPS engine
TinyGPS gps;

// ...which depends on a SW serial - the TX pin of the GPS module
// is connected to the GPS_RX_PIN of the Arduino.
SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);

// Finally, the LCD lives at I2C address 0x3F, and controls a 16x2 display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup()
{
    // Setup:

    // Serial port
    Serial.begin(9600);

    // LCD
    lcd.init();
    lcd.backlight();

    // RTC
    rtc_setup();

    // GPS
    gps_setup(ss);

    // Button
    button_setup(BUTTON_PIN);
}

void loop()
{
    extern void button_process();
    button_process();

    // show time once in a while
    extern void rtc_show_clock(LiquidCrystal_I2C&);
    rtc_show_clock(lcd);

    extern void gps_feed_and_update(LiquidCrystal_I2C& lcd, SoftwareSerial& ss);
    gps_feed_and_update(lcd, ss);
}
