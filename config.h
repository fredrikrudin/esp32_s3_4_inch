#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <lvgl.h>

const int RELAY_4CH_PINS[] = {1, 2, 42, 41}; 
const int RELAY_8CH_PINS[] = {4, 5, 6, 7, 15, 16, 17, 18};

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
