#ifndef ELEGOO_RELAY_MANAGER_H
#define ELEGOO_RELAY_MANAGER_H

#include "config.h"

void setExternalRelay(uint8_t relayPin, bool state) {
    static uint8_t currentRegisterState = 0xFF; 
    uint8_t previousState = currentRegisterState;

    if (state) {
        currentRegisterState &= ~(1 << relayPin); // Aktivt låg (LOW = PÅ)
    } else {
        currentRegisterState |= (1 << relayPin);  // HIGH = AV
    }

    // Buss-optimering: Skriv endast till hårdvaran om tillståndet förändrats
    if (currentRegisterState != previousState) {
        ExternalI2C.beginTransmission(RELAY_I2C_ADDRESS);
        ExternalI2C.write(currentRegisterState);
        
        uint8_t error = ExternalI2C.endTransmission();
        if (error != 0) {
            Serial.printf("[Relä Fel] Det gick inte att nå PCF8574. Kod: %d\n", error);
        }
    }
}

#endif // ELEGOO_RELAY_MANAGER_H
