#include "ble_manager.h"
#include "victron_manager.h"
#include "ruuvi_manager.h"
#include "xiaomi_manager.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

#define SCAN_TIME_SECONDS 0 // 0 betyder att den skannar kontinuerligt i bakgrunden

BLEScan* pBLEScan;

// Callback-funktion som triggas varje gång en BLE-enhet skickar ut data
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        std::string name = advertisedDevice.getName();
        std::string address = advertisedDevice.getAddress().toString();
        
        // Kontrollera om det är en Victron-enhet (SmartShunt, MPPT, IP22)
        if (advertisedDevice.haveServiceData()) {
            // Skicka vidare till victron_manager för AES-dekryptering
            processVictronBLE(advertisedDevice);
        }
        
        // Kontrollera om det är en RuuviTag
        if (advertisedDevice.haveManufacturerData()) {
            std::string manufData = advertisedDevice.getManufacturerData();
            if (manufData.length() >= 2 && manufData[0] == 0x99 && manufData[1] == 0x04) { // 0x0499 = Ruuvi Innovations
                processRuuviBLE(advertisedDevice);
            }
        }

        // Kontrollera om det är en Xiaomi Mijia-sensor
        if (name.find("LYWSD03MMC") != std::string::npos || name.find("XIAOMI") != std::string::npos) {
            processXiaomiBLE(advertisedDevice);
        }
    }
};

void initBLE() {
    Serial.println("[BLE] Initierar passiv BLE-skanner på Core 0...");
    
    BLEDevice::init("VenusOS-ESP32-Klon");
    pBLEScan = BLEDevice::getBLEScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    
    // Passiv skanning krävs enligt README för att läsa krypterad data i luften
    pBLEScan->setActiveScan(false); 
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void startBLEScan() {
    Serial.println("[BLE] Startar kontinuerlig bakgrundsskanning...");
    // Startar skanningen asynkront (icke-blockerande) så FreeRTOS Core 0 kan göra annat
    pBLEScan->start(SCAN_TIME_SECONDS, nullptr, false);
}
