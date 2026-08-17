#ifndef ELEGOO_RELAY_MANAGER_H
#define ELEGOO_RELAY_MANAGER_H

#include <Arduino.h>
#include "config.h"

void init_elegoo_relays() {
    int max_pins = elegoo.elegoo_channels;
    if (max_pins == 0) return; // Reläkort avstängt

    for (int i = 0; i < max_pins; i++) {
        // Välj pin-uppsättning baserat på om det är 4 eller 8 kanalskortet som är valt
        int current_pin = (max_pins == 4) ? RELAY_4CH_PINS[i] : RELAY_8CH_PINS[i];
        
        pinMode(current_pin, OUTPUT);
        digitalWrite(current_pin, HIGH); // Active Low -> HIGH vid start (AV)
        elegoo.relay_states[i] = false;
    }
}

void setElegooRelayState(int ch, bool turn_on) {
    if (elegoo.elegoo_channels == 0 || ch < 0 || ch >= elegoo.elegoo_channels) return;
    
    elegoo.relay_states[ch] = turn_on;
    int target_pin = (elegoo.elegoo_channels == 4) ? RELAY_4CH_PINS[ch] : RELAY_8CH_PINS[ch];
    
    digitalWrite(target_pin, turn_on ? LOW : HIGH);
    Serial.printf("[Elegoo IO] Kanal %d satt till: %s\n", ch + 1, turn_on ? "PÅ" : "AV");
}

void processElegooSchedules() {
    if (elegoo.elegoo_channels == 0) return;
    
    time_t now; struct tm ti; 
    if (!getLocalTime(&ti)) return;

    for (int i = 0; i < elegoo.elegoo_channels; i++) {
        if (!elegoo.schedules[i].schedule_active) continue;
        
        if (ti.tm_hour == elegoo.schedules[i].on_hour && !elegoo.relay_states[i]) {
            setElegooRelayState(i, true);
        }
        if (ti.tm_hour == elegoo.schedules[i].off_hour && elegoo.relay_states[i]) {
            setElegooRelayState(i, false);
        }
    }
}
#endif
