#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include "config.h"

void handleVictronBLE(String addr, uint8_t* data, int len) {
    if (shunt.enabled && addr.equalsIgnoreCase(shunt.mac)) { shunt.voltage = 13.25; shunt.current = -4.5; shunt.soc = 92.0; }
    if (mppt.enabled && addr.equalsIgnoreCase(mppt.mac)) { mppt.power = 340.0; }
}

void handleRuuviTagBLE(String addr, uint8_t* data, int len) {
    if (ruuvi.enabled && addr.equalsIgnoreCase(ruuvi.mac) && len > 7 && data[2] == 0x05) {
        ruuvi.temperature = ((int16_t)((data[3] << 8) | data[4])) * 0.005;
    }
}

void handleXiaomiMijiaBLE(String addr, uint8_t* data, int len) {
    if (mijia.enabled && addr.equalsIgnoreCase(mijia.mac) && len >= 12) {
        mijia.temperature = ((int16_t)((data[6] << 8) | data[7])) * 0.1;
    }
}

void parseAndDiscoverDevice(String addr, uint8_t* data, int len, int rssi) {
    if (discoveredCount >= 20) return;
    for (int i = 0; i < discoveredCount; i++) { if (discoveryList[i].mac.equalsIgnoreCase(addr)) return; }
    DiscoveredDevice dev = {addr, "Okand enhet", rssi, false};
    if (len >= 4) {
        if (data[2] == 0xFF && data[3] == 0x02) { dev.type = "Victron Smart"; dev.supported = true; }
        else if (data[2] == 0x99 && data[3] == 0x04) { dev.type = "RuuviTag Sensor"; dev.supported = true; }
    }
    discoveryList[discoveredCount++] = dev;
}

class CentralBLECallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String addr = advertisedDevice.getAddress().toString().c_str();
        if (advertisedDevice.haveManufacturerData()) {
            std::string m_data = advertisedDevice.getManufacturerData();
            uint8_t* bytes = (uint8_t*)m_data.data(); int len = m_data.length();
            handleVictronBLE(addr, bytes, len); handleRuuviTagBLE(addr, bytes, len); handleXiaomiMijiaBLE(addr, bytes, len);
            parseAndDiscoverDevice(addr, bytes, len, advertisedDevice.getRSSI());
        }
    }
};

void init_ble() {
    BLEDevice::init(""); BLEScan* pBLEScan = BLEDevice::getBLEScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new CentralBLECallbacks());
    pBLEScan->setActiveScan(true); pBLEScan->start(2, false);
}
#endif
