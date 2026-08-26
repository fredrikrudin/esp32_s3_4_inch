#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "config.h"

class VictronAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (!advertisedDevice.haveManufacturerData()) return;

        std::string mData = advertisedDevice.getManufacturerData();
        if (mData.length() < 2) return;
        
        uint16_t manufacturerId = (mData << 8) | mData;

        // Filtrera på Victron Energy ID
        if (manufacturerId == 0x0241) {
            // Här lägger du till din befintliga dekrypterings- och parsningslogik
            // som använder mData och skiftar in värdena i shunt, mppt eller ip22
            
            if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
                shunt.voltage = 13.25; // Exempelvärde, mappa mot dina variabler
                shunt.current = -4.20;
                xSemaphoreGive(lvgl_mutex);
            }
        }
    }
};

void bleScanTask(void *pvParameters) {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getBLEScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new VictronAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(false); // Passiv skanning för att skydda Wi-Fi anslutningen
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    while(1) {
        pBLEScan->start(4, false); // Skanna i 4 sekunder
        pBLEScan->clearResults();   
        vTaskDelay(pdMS_TO_TICKS(10000)); // Pausa i 10 sekunder
    }
}

void ble_manager_init() {
    xTaskCreatePinnedToCore(bleScanTask, "BLE_Scan_Task", 4096, NULL, 1, NULL, 0);
}

#endif // BLE_MANAGER_H
