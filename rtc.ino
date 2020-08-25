#include <ds3231.h>

#include "globals.h"

#define BUFF_MAX 128


void rtc_setup()
{
    // I2C setup
    Wire.begin();
    DS3231_init(DS3231_CONTROL_INTCN);
}

void rtc_show_clock(LiquidCrystal_I2C& lcd)
{
    const unsigned long interval_LCD = 249;
    const unsigned long interval_Serial = 5000;
    static unsigned long prev_LCD;
    static unsigned long prev_Serial;
    static char msg[BUFF_MAX];
    struct ts t = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    unsigned long now = millis();

    if (now - prev_LCD > interval_LCD && g_state == SHOW_TIME) {
        // Get time from RTC chip
        DS3231_get(&t);
        t.hour += (g_timezone == 0) ? 2 : 3;
        if (t.hour >= 24) {
            t.hour -= 24;
            t.mday ++;
        }

        // Display on 16x2 LCD screen
        snprintf(msg, BUFF_MAX, "%02d/%02d   %02d:%02d:%02d",
                 //t.year,  // Can't fit!
                 t.mon, t.mday, t.hour, t.min, t.sec);
        lcd.setCursor(0,0);
        lcd.print(msg);
        prev_LCD = now;
    }
    if (now - prev_Serial > interval_Serial) {
        // Get time from RTC chip
        DS3231_get(&t);

        // Also display in serial output
        snprintf(msg, BUFF_MAX, "[-] DS3231 time: %02d/%02d %02d:%02d:%02d",
                 //t.year,  // Can't fit!
             t.mon, t.mday, t.hour, t.min, t.sec);
        Serial.println(msg);
        prev_Serial = now;
    }
}
