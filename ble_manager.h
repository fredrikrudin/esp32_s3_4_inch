#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "config.h"

void parseVictronAdvertisement(String mac, uint8_t* payload, int length) {
    VictronDevice* target = nullptr;

    if (shunt.cfg.enabled && shunt.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &shunt;
    else if (mppt.cfg.enabled && mppt.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &mppt;
    else if (ip22.cfg.enabled && ip22.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &ip22;

    if (target == nullptr) return; 

    Serial.printf("[BLE] Data hittad för '%s' (%s), Typ: %d\n", target->cfg.name.c_str(), mac.c_str(), target->deviceType);
    // Dekrypteringsrutiner körs här med target->key...
}

class MyBLEAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String macAddress = advertisedDevice.getAddress().toString().c_str();
        
        if (advertisedDevice.haveManufacturerData()) {
            std::string mData = advertisedDevice.getManufacturerData();
            uint8_t* payload = (uint8_t*)mData.data();
            
            if (mData.length() > 2 && payload[0] == 0xE1 && payload[1] == 0x02) {
                parseVictronAdvertisement(macAddress, payload, mData.length());
                return;
            }
        }
    }
};

void initBLEManager() {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyBLEAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(false); 
    pBLEScan->start(0, nullptr, false); 
    Serial.println("[BLE] Universell BLE-skanning aktiv.");
}

#endif // BLE_MANAGER_H
