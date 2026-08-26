#include "elegoo_relay_manager.h"
#include <Wire.h>

// Skapa en dedikerad I2C-instans för externa enheter (Buss 1)
TwoWire ExternalI2C = TwoWire(1);

#define EXT_I2C_SDA 15
#define EXT_I2C_SCL 7
#define PCF8574_ADDRESS 0x20 // Standardadress enligt README

// Håller koll på nuvarande relästatus (bitmask för 8 reläer)
uint8_t relayState = 0xFF; // Inverterad logik är standard (0xFF = alla reläer AV)

void initRelays() {
    Serial.println("[Relay] Initierar extern I2C-buss för reläer (100kHz)...");
    
    // Starta externa bussen på GPIO 15 och 7 i 100kHz
    ExternalI2C.begin(EXT_I2C_SDA, EXT_I2C_SCL, 100000);
    
    // Sätt alla reläer till AV vid uppstart
    setAllRelaysOff();
}

void setRelay(uint8_t relayChannel, bool turnOn) {
    if (relayChannel > 7) return; // Stöder max 8 kanaler

    // PCF8574 är ofta aktivt låg (LOW = Relä drar/slår på)
    if (turnOn) {
        relayState &= ~(1 << relayChannel); // Sätt biten till 0 (PÅ)
    } else {
        relayState |= (1 << relayChannel);  // Sätt biten till 1 (AV)
    }

    // Skriv bitmasken till PCF8574 över externa bussen
    ExternalI2C.beginTransmission(PCF8574_ADDRESS);
    ExternalI2C.write(relayState);
    if (ExternalI2C.endTransmission() != 0) {
        Serial.println("[ERROR] Kunde inte kommunicera med extern PCF8574-relämodul!");
    }
}

void setAllRelaysOff() {
    relayState = 0xFF; // 0xFF stänger av alla kanaler (aktivt låg)
    ExternalI2C.beginTransmission(PCF8574_ADDRESS);
    ExternalI2C.write(relayState);
    ExternalI2C.endTransmission();
    Serial.println("[Relay] Alla externa reläer avstängda.");
}
