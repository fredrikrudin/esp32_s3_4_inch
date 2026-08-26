#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <lvgl.h>

// Ändra relä-pinnarna till virtuella kanaler på I2C-expandern (0-7)
const int RELAY_4CH_PINS[] = {0, 1, 2, 3}; 
const int RELAY_8CH_PINS[] = {0, 1, 2, 3, 4, 5, 6, 7};

// Lägg till I2C-inställningar för ditt reläkort
const uint8_t RELAY_I2C_ADDRESS = 0x20; // Justera efter dina hårdvarubrytare (Address-pinnar)
const int I2C_SDA_PIN = 15;
const int I2C_SCL_PIN = 7;

struct RelaySchedule { bool schedule_active; int on_hour; int off_hour; };

struct ElegooRelaySystem {
    bool relay_states[8];       // Gemensam array för upp till 8 reläer
    int elegoo_channels;       // NYTT: Kan vara 0, 4, eller 8 kanaler
    RelaySchedule schedules[8]; // Gemensam schema-array
};

extern ElegooRelaySystem elegoo;

// Gemensamma skärmobjekt för relähantering
extern lv_obj_t * page_elegoo_relays;
extern lv_obj_t * list_elegoo_relays;

#endif
