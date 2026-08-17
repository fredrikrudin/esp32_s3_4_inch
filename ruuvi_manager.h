#ifndef RUUVI_MANAGER_H
#define RUUVI_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Avkodar RuuviTags öppna BLE-reklampaket (Data Format 5, RAWv2)
void handleRuuviTagBLE(String addr, uint8_t* data, int len) {
    if (!ruuvi.enabled || !addr.equalsIgnoreCase(ruuvi.mac)) return;
    
    // Kontrollera Ruuvis tillverkarkod och att det är Data Format 5 (0x05)
    if (len > 7 && data[0] == 0x05) {
        // Temperatur: bit 24-39 (16-bitars signed integer, multiplicerat med 0.005)
        int16_t raw_temp = (data[1] << 8) | data[2];
        ruuvi.temperature = raw_temp * 0.005;
        
        // Luftfuktighet: bit 40-55 (16-bitars unsigned integer, multiplicerat med 0.0025)
        uint16_t raw_hum = (data[3] << 8) | data[4];
        ruuvi.humidity = raw_hum * 0.0025;
    }
}

#endif
