#ifndef ELEGOO_RELAY_MANAGER_H
#define ELEGOO_RELAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <PCF8574.h>
#include "config.h"

// Allokera PCF8574-objektet
PCF8574 relayExpander(RELAY_I2C_ADDRESS);
bool isRelayBoardConnected = false;

void initElegooRelays() {
    Serial.println("[Relay] Initierar det isolerade I2C-reläkortet...");
    
    // Tvinga reläerna att köra på ESP32-S3:s andra helt separata hårdvarubuss (Wire1)
    // Detta förhindrar helt att vi skriver över eller stör skärmen på Wire(8,9)!
    Wire1.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000); 

    // Skicka med Wire1-referensen till expanderbiblioteket
    if (relayExpander.begin(&Wire1)) {
        Serial.println("[Relay] PCF8574 relä-expander hittad på Wire1!");
        isRelayBoardConnected = true;

        if (elegoo.enabled_4ch) {
            for (int i = 0; i < 4; i++) {
                relayExpander.pinMode(RELAY_4CH_PINS[i], OUTPUT);
                relayExpander.write(RELAY_4CH_PINS[i], elegoo.relay4_states[i] ? HIGH : LOW);
            }
        }
        if (elegoo.enabled_8ch) {
            for (int i = 0; i < 8; i++) {
                relayExpander.pinMode(RELAY_8CH_PINS[i], OUTPUT);
                relayExpander.write(RELAY_8CH_PINS[i], elegoo.relay8_states[i] ? HIGH : LOW);
            }
        }
    } else {
        Serial.printf("[Relay] FEL: Kunde inte hitta reläkortet på Wire1 (Adress: 0x%02X)\n", RELAY_I2C_ADDRESS);
        isRelayBoardConnected = false;
    }
}

void setElegooRelay4Ch(int relayNum, bool state) {
    if (relayNum < 0 || relayNum >= 4) return;
    elegoo.relay4_states[relayNum] = state;
    if (isRelayBoardConnected) {
        relayExpander.write(RELAY_4CH_PINS[relayNum], state ? HIGH : LOW);
    }
}

void setElegooRelay8Ch(int relayNum, bool state) {
    if (relayNum < 0 || relayNum >= 8) return;
    elegoo.relay8_states[relayNum] = state;
    if (isRelayBoardConnected) {
        relayExpander.write(RELAY_8CH_PINS[relayNum], state ? HIGH : LOW);
    }
}

void updateRelaySchedules(int currentHour, int currentMinute) {
    if (!isRelayBoardConnected) return;

    if (elegoo.enabled_4ch) {
        for (int i = 0; i < 4; i++) {
            if (elegoo.schedule4[i].schedule_active) {
                if (currentHour == elegoo.schedule4[i].on_hour && currentMinute == 0) setElegooRelay4Ch(i, true);
                else if (currentHour == elegoo.schedule4[i].off_hour && currentMinute == 0) setElegooRelay4Ch(i, false);
            }
        }
    }
    if (elegoo.enabled_8ch) {
        for (int i = 0; i < 8; i++) {
            if (elegoo.schedule8[i].schedule_active) {
                if (currentHour == elegoo.schedule8[i].on_hour && currentMinute == 0) setElegooRelay8Ch(i, true);
                else if (currentHour == elegoo.schedule8[i].off_hour && currentMinute == 0) setElegooRelay8Ch(i, false);
            }
        }
    }
}

#endif // ELEGOO_RELAY_MANAGER_H
