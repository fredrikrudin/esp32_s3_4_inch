#ifndef ELEGOO_RELAY_MANAGER_H
#define ELEGOO_RELAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <PCF8574.h>
#include "config.h"

// Initiera PCF8574-objektet med adressen från config.h
// Standardadressen för PCF8574 är oftast 0x20 eller 0x38 beroende på krets/byglar
PCF8574 relayExpander(RELAY_I2C_ADDRESS);

// Flagga för att hålla koll på om I2C-expandern svarar
bool isRelayBoardConnected = false;

/**
 * Initierar I2C-bussen och sätter upp alla virtuella relä-pinnar som utgångar.
 * Denna funktion anropas i setup() i din huvudfil.
 */
void initElegooRelays() {
    Serial.println("[Relay] Initierar I2C Reläsystem...");

    // Starta I2C-bussen på de säkra externa pinnarna (SDA: 15, SCL: 7)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Försök att kommunicera med PCF8574-chippet
    if (relayExpander.begin()) {
        Serial.println("[Relay] SÄNDNINGSLÄGE: PCF8574 hittad på I2C-bussen!");
        isRelayBoardConnected = true;

        // Initiera 4-kanalsportarna på expandern (0 till 3) om aktiverat
        if (elegoo.enabled_4ch) {
            for (int i = 0; i < 4; i++) {
                relayExpander.pinMode(RELAY_4CH_PINS[i], OUTPUT);
                // Sätt reläet till dess sparade boot-tillstånd
                relayExpander.write(RELAY_4CH_PINS[i], elegoo.relay4_states[i] ? HIGH : LOW);
            }
            Serial.println("[Relay] 4-kanals reläkonfiguration inladdad.");
        }

        // Initiera 8-kanalsportarna på expandern (0 till 7) om aktiverat
        if (elegoo.enabled_8ch) {
            for (int i = 0; i < 8; i++) {
                relayExpander.pinMode(RELAY_8CH_PINS[i], OUTPUT);
                // Sätt reläet till dess sparade boot-tillstånd
                relayExpander.write(RELAY_8CH_PINS[i], elegoo.relay8_states[i] ? HIGH : LOW);
            }
            Serial.println("[Relay] 8-kanals reläkonfiguration inladdad.");
        }
    } else {
        Serial.print("[Relay] FEL: Kunde INTE hitta reläkortet på I2C-adress: 0x");
        Serial.println(RELAY_I2C_ADDRESS, HEX);
        isRelayBoardConnected = false;
    }
}

/**
 * Ändrar tillstånd på ett specifikt relä i 4-kanalssystemet över I2C.
 */
void setElegooRelay4Ch(int relayNum, bool state) {
    if (relayNum < 0 || relayNum >= 4) return;
    
    // Spara det nya tillståndet i din globala struktur
    elegoo.relay4_states[relayNum] = state;

    if (isRelayBoardConnected) {
        // Skriv direkt till expandern (ingen risk för skärmkrasch!)
        relayExpander.write(RELAY_4CH_PINS[relayNum], state ? HIGH : LOW);
        Serial.printf("[Relay] 4CH Relä %d ändrat till %s\n", relayNum, state ? "ON" : "OFF");
    }
}

/**
 * Ändrar tillstånd på ett specifikt relä i 8-kanalssystemet över I2C.
 */
void setElegooRelay8Ch(int relayNum, bool state) {
    if (relayNum < 0 || relayNum >= 8) return;

    // Spara det nya tillståndet i din globala struktur
    elegoo.relay8_states[relayNum] = state;

    if (isRelayBoardConnected) {
        // Skriv direkt till expandern (ingen risk för skärmkrasch!)
        relayExpander.write(RELAY_8CH_PINS[relayNum], state ? HIGH : LOW);
        Serial.printf("[Relay] 8CH Relä %d ändrat till %s\n", relayNum, state ? "ON" : "OFF");
    }
}

/**
 * Uppdaterar reläscheman baserat på klockan (NTP/RTC)
 * Denna funktion anropas i din loop() med jämna mellanrum.
 */
void updateRelaySchedules(int currentHour, int currentMinute) {
    if (!isRelayBoardConnected) return;

    // Hantera schema för 4-kanals relä
    if (elegoo.enabled_4ch) {
        for (int i = 0; i < 4; i++) {
            if (elegoo.schedule4[i].schedule_active) {
                if (currentHour == elegoo.schedule4[i].on_hour && currentMinute == 0) {
                    setElegooRelay4Ch(i, true);
                } else if (currentHour == elegoo.schedule4[i].off_hour && currentMinute == 0) {
                    setElegooRelay4Ch(i, false);
                }
            }
        }
    }

    // Hantera schema för 8-kanals relä
    if (elegoo.enabled_8ch) {
        for (int i = 0; i < 8; i++) {
            if (elegoo.schedule8[i].schedule_active) {
                if (currentHour == elegoo.schedule8[i].on_hour && currentMinute == 0) {
                    setElegooRelay8Ch(i, true);
                } else if (currentHour == elegoo.schedule8[i].off_hour && currentMinute == 0) {
                    setElegooRelay8Ch(i, false);
                }
            }
        }
    }
}

#endif // ELEGOO_RELAY_MANAGER_H
