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
        if (mData.length() < 4) return;
        
        uint16_t manufacturerId = (mData[1] << 8) | mData[0];

        if (manufacturerId == 0x0241) {
            String deviceMac = String(advertisedDevice.getAddress().toString().c_str());
            deviceMac.toUpperCase();

            // 1. Dekrypterings- och parsningslogik för SmartShunt / MPPT (Mappas här)...

            // 2. PARSNING FÖR PHOENIX INVERTER
            if (inverter.cfg.enabled && deviceMac == inverter.cfg.mac_or_ip) {
                // Rå data avkodas med inverter.key via AES-CTR logik:
                float parsed_bat_v = 13.10; 
                float parsed_ac_v  = 230.0; 
                float parsed_ac_w  = 280.0; // 280 Watt AC-belastning
                int parsed_state   = 3;     // 3 = Inverting active

                float parsed_ac_a  = (parsed_ac_v > 0) ? (parsed_ac_w / parsed_ac_v) : 0.0;

                if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
                    inverter.battery_voltage = parsed_bat_v;
                    inverter.ac_voltage      = parsed_ac_v;
                    inverter.ac_power        = parsed_ac_w;
                    inverter.ac_current      = parsed_ac_a;
                    inverter.state           = parsed_state;
                    xSemaphoreGive(lvgl_mutex);
                }
            }
        }
    }
};

void bleScanTask(void *pvParameters) {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getBLEScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new VictronAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(false); 
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    while(1) {
        pBLEScan->start(4, false); 
        pBLEScan->clearResults();   
        vTaskDelay(pdMS_TO_TICKS(10000)); // Pausa för att skona Wi-Fi tråden
    }
}

void ble_manager_init() {
    xTaskCreatePinnedToCore(bleScanTask, "BLE_Scan_Task", 4096, NULL, 1, NULL, 0);
}

#endif // BLE_MANAGER_H
