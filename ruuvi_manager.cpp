#include "ruuvi_manager.h"
#include <Arduino.h>

void processRuuviBLE(BLEAdvertisedDevice& device) {
    std::string data = device.getManufacturerData();
    
    // RuuviTag Data Format 5 (Rawv2) tolkning:
    if (data.length() >= 26) {
        int16_t raw_temp = (data[4] << 8) | data[5];
        uint16_t raw_hum = (data[6] << 8) | data[7];
        
        float temperature = raw_temp * 0.005;
        float humidity = raw_hum * 0.0025;
        
        // Här uppdateras värdena som visas på skärmen
        // Serial.printf("[Ruuvi] Temp: %.2f C, Fukt: %.2f %%\n", temperature, humidity);
    }
}
