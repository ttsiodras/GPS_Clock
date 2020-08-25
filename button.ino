#include <EEPROM.h>

#include "globals.h"

int g_pin;
volatile int g_btn_voltage = HIGH;
volatile unsigned long g_last_btn_change_millis = 0;

const int DEBOUNCE_DELAY_IN_MS = 10;
const unsigned long LONG_KEYPRESS_IN_MS = 1000;

static const char *g_countries[] = {
    "Netherlands     ",
    "Greece          "
};

void ISR_btn()
{
    g_btn_voltage = digitalRead(g_pin);
    g_last_btn_change_millis = millis();
}

void button_setup(int pin)
{
    g_pin = pin;
    pinMode(g_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(g_pin), ISR_btn, CHANGE);
    g_timezone = EEPROM.read(0) & 1;
}

void clear_LCD()
{
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
}

void displayTimeZoneMenu(size_t menu_idx)
{
    for(size_t i=0; i<sizeof(g_countries)/sizeof(g_countries[0]); i++) {
        lcd.setCursor(0, i);
        lcd.print((i == menu_idx) ? "> " : "  ");
        lcd.print(g_countries[i]);
    }
}

void update_state_machine(int btn_voltage, unsigned long current_millis)
{
    static unsigned long old_btn_voltage_millis = 0;
    bool long_key_press = false;
    static uint8_t menu_idx = g_timezone;

    Serial.print("Button is ");
    Serial.println(btn_voltage ? "UP" : "DOWN");
    
    if (btn_voltage == LOW) {
        // We just had a DOWN keypress.
        // Wait for the UP, timing it
        old_btn_voltage_millis = current_millis;
        return;
    } 
    // We just got back to UP
    // How much time passed?
    // (detect long keypress - more than 1 second)
    long_key_press = (current_millis - old_btn_voltage_millis) > LONG_KEYPRESS_IN_MS;

    switch(g_state) {
    case SHOW_TIME:
        g_state = SELECT_TIMEZONE;
        displayTimeZoneMenu(menu_idx);
        break;
    case SELECT_TIMEZONE:
        if (long_key_press) {
            g_state = SHOW_TIME;
            g_timezone = menu_idx;
            clear_LCD();
            lcd.setCursor(0, 1);
            Serial.println(g_countries[g_timezone]);
            lcd.print(g_countries[g_timezone]);
            Serial.print("Saving selection: ");
            Serial.println(menu_idx);
            EEPROM.write(0, menu_idx);
        } else {
            menu_idx = menu_idx ^ 1;
            displayTimeZoneMenu(menu_idx);
        }
        break;
    }
}

void button_process()
{
    static int btn_voltage = HIGH;

    noInterrupts();
    if (btn_voltage != g_btn_voltage &&
        millis() - g_last_btn_change_millis > DEBOUNCE_DELAY_IN_MS)
    {
        btn_voltage = g_btn_voltage;
        interrupts();
        update_state_machine(btn_voltage, millis());
    } else {
        interrupts();
    }
}
