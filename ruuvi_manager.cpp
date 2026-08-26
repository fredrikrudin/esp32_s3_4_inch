#include "ruuvi_manager.h"
#include <Arduino.h>

void processRuuviBLE(BLEAdvertisedDevice& device) {
    std::string data = device.getManufacturerData();
    
    // Säkerställ att vi har tillräckligt med bytes och parsa array-index korrekt
    if (data.length() >= 26) {
        // Castar till uint8_t för att kunna göra korrekt bit-shifting på specifika bytes
        const uint8_t* rawData = (const uint8_t*)data.data();
        
        int16_t raw_temp = (rawData[4] << 8) | rawData[5];
        uint16_t raw_hum = (rawData[6] << 8) | rawData[7];
        
        float temperature = raw_temp * 0.005;
        float humidity = raw_hum * 0.0025;
    }
}

