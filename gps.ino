#include <TinyGPS.h>
#include <ds3231.h>
#include <SoftwareSerial.h>

#include "globals.h"

void gps_setup(SoftwareSerial& ss)
{
    Serial.print("[-] Using TinyGPS library v. ");
    Serial.println(TinyGPS::library_version());
    ss.begin(9600);
}

static void get_gps_time_and_update_RTC(TinyGPS& gps)
{
    static int got_GPS_time = 0;
    int year;
    byte month, day, hour, minute, second, hundredths;
    unsigned long age;
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    if (age != TinyGPS::GPS_INVALID_AGE)
    {
        char sz[32];
        sprintf(sz, "[-] GPS date: %02d/%02d/%02d %02d:%02d:%02d",
                month, day, year, hour, minute, second);
        Serial.println(sz);

        struct ts t;
        if (!got_GPS_time) {
            Serial.println("[-] Setting DS3231 time from GPS... done.");
            got_GPS_time = 1;
            t.hour = hour;
            t.min  = minute;
            t.sec  = second;
            t.mday = day;
            t.mon  = month;
            t.year = year;
            DS3231_set(t); 
        }
    }
}

void gps_feed_and_update(LiquidCrystal_I2C& lcd, SoftwareSerial& ss)
{
    // GPS
    unsigned long chars;
    unsigned short sentences, failed;

    // For one second we parse GPS data and report some key values
    while (ss.available()) {
        char c = ss.read();
#ifdef DEBUG_NMEA
        Serial.write(c); // uncomment this line if you want to see the GPS data flowing
#endif
        if (gps.encode(c)) { // Did a new valid sentence come in?
            float flat, flon;
            unsigned long age;
            static char msg[16];
            gps.f_get_position(&flat, &flon, &age);
            Serial.print("[-] Position found: LAT=");
            Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
            Serial.print(" LON=");
            Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
            Serial.print(" SAT=");
            Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
            Serial.print(" PREC=");
            Serial.println(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
            if (g_state == SHOW_TIME) {
                lcd.setCursor(0, 1);
                dtostrf(flat, 7, 4, msg);
                lcd.print(msg);
                lcd.print(", ");
                dtostrf(flon, 7, 4, msg);
                lcd.print(msg);
            }
            get_gps_time_and_update_RTC(gps);
        }
    }

    gps.stats(&chars, &sentences, &failed);
    if (chars == 0) {
        static unsigned long last_msg_millis;
        if (millis() - last_msg_millis > 60000) {
            Serial.println("[x] No characters received from GPS: check wiring! **********");
            last_msg_millis = millis();
        }
    }
#ifdef REPORT_STATISTICS
    else {
        Serial.print("[-] GPS data statistics: CHARS=");
        Serial.print(chars);
        Serial.print(" SENTENCES=");
        Serial.print(sentences);
        Serial.print(" CSUM ERR=");
        Serial.println(failed);
    }
#endif
}
