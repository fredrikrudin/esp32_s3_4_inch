#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "mbedtls/aes.h" 
#include "config.h"

// --- HJÄLPFUNKTIONER FÖR VICTRON ---
bool hexStringToBytes(String hexStr, uint8_t* byteArr, int maxLen) {
    if (hexStr.length() != maxLen * 2) return false;
    for (int i = 0; i < maxLen; i++) {
        String byteString = hexStr.substring(i * 2, i * 2 + 2);
        byteArr[i] = (uint8_t) strtol(byteString.c_str(), NULL, 16);
    }
    return true;
}

bool decryptVictronPayload(uint8_t* encryptedData, int dataLen, uint8_t* key, uint16_t nonce, uint8_t* decryptedOutput) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_enc(&aes, key, 128) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }
    uint8_t stream_block[16] = {0};
    uint8_t iv[16] = {0};
    iv[0] = nonce & 0xFF;
    iv[1] = (nonce >> 8) & 0xFF;
    size_t nc_off = 0;
    int res = mbedtls_aes_crypt_ctr(&aes, dataLen, &nc_off, iv, stream_block, encryptedData, decryptedOutput);
    mbedtls_aes_free(&aes);
    return (res == 0);
}

void decodeVictronMetrics(VictronDevice* target, uint8_t* data, int len) {
    if (len < 4) return; 
    if (target->deviceType == 1) { 
        int16_t raw_current = (data[3] << 8) | data[2]; 
        uint16_t raw_voltage = (data[1] << 8) | data[0];
        uint16_t raw_soc = (data[5] << 8) | data[4];
        target->voltage = (float)raw_voltage / 100.0;     
        target->current = (float)raw_current / 100.0;     
        target->soc = (float)(raw_soc & 0x3FFF) / 10.0;   
        target->power = target->voltage * target->current; 
    } 
    else if (target->deviceType == 2) { 
        uint16_t raw_voltage = (data[1] << 8) | data[0];
        int16_t raw_current = (data[3] << 8) | data[2];
        uint16_t raw_power = (data[5] << 8) | data[4];
        target->voltage = (float)raw_voltage / 100.0;
        target->current = (float)raw_current / 10.0;
        target->power = (float)raw_power;
        target->soc = 100.0; 
    }
}

void parseVictronAdvertisement(String mac, uint8_t* payload, int length) {
    VictronDevice* target = nullptr;
    if (shunt.cfg.enabled && shunt.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &shunt;
    else if (mppt.cfg.enabled && mppt.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &mppt;
    else if (ip22.cfg.enabled && ip22.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &ip22;

    if (target == nullptr || length < 12) return;

    uint8_t aesKey[16];
    if (!hexStringToBytes(target->key, aesKey, 16)) return;

    uint16_t packetNonce = (payload[5] << 8) | payload[4];
    int encryptedDataLen = length - 6;
    uint8_t* encryptedDataBlock = &payload[6];
    uint8_t decryptedPayload[16] = {0};

    if (decryptVictronPayload(encryptedDataBlock, encryptedDataLen, aesKey, packetNonce, decryptedPayload)) {
        decodeVictronMetrics(target, decryptedPayload, encryptedDataLen);
    }
}

// --- 🍏 NYTT BLOCK: AVKODNING FÖR RUUVI-TAG (Dataformat 5) ---
void parseRuuviTag(uint8_t* data, int length) {
    // Ruuvi-data sänds som Manufacturer Data. Byte 2 (efter ID) anger format (0x05)
    if (length < 24 || data[2] != 0x05) return; 

    // Avkoda temperatur (Tvåkomplement,0.005°C per enhet)
    int16_t raw_temp = (data[3] << 8) | data[4];
    ruuvi.temperature = (float)raw_temp * 0.005;

    // Avkoda luftfuktighet (0.0025% per enhet)
    uint16_t raw_humid = (data[5] << 8) | data[6];
    ruuvi.humidity = (float)raw_humid * 0.0025;

    Serial.printf("[BLE] Ruuvi '%s' uppdaterad: %.2f°C, %.2f%%\n", 
                  ruuvi.cfg.name.c_str(), ruuvi.temperature, ruuvi.humidity);
}

// --- 🍋 NYTT BLOCK: AVKODNING FÖR XIAOMI MIJIA (ATC / PVVX Custom Firmware) ---
void parseXiaomiMijia(uint8_t* serviceData, int length) {
    // Stöder ATC/PVVX standard BLE-annonser (Service Data UUID 0x181A eller liknande Mi-Beacons)
    // Detta är det absolut vanligaste och mest stabila sättet att läsa Xiaomi-mätare i hemautomation
    if (length < 15) return;

    // Avkodning av temperatur (16-bit Little Endian, i steg om 0.01°C eller 0.1°C)
    int16_t raw_temp = (serviceData[11] << 8) | serviceData[10];
    mijia.temperature = (float)raw_temp / 100.0;

    // Avkodning av luftfuktighet (16-bit, steg om 0.01%)
    uint16_t raw_humid = (serviceData[13] << 8) | serviceData[12];
    mijia.humidity = (float)raw_humid / 100.0;

    // Batterinivå (%)
    mijia.battery_level = (float)serviceData[14];

    Serial.printf("[BLE] Xiaomi '%s' uppdaterad: %.1f°C, %.1f%% (Batteri: %.0f%%)\n", 
                  mijia.cfg.name.c_str(), mijia.temperature, mijia.humidity, mijia.battery_level);
}

// --- 🕵️‍♂️ HUVUD-CALLBACK FÖR SKANNERN (Hanberar alla sensorer) ---
class MyBLEAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String macAddress = advertisedDevice.getAddress().toString().c_str();
        
        // 1. Hantera VICTRON eller RUUVI (Kräver Manufacturer Data)
        if (advertisedDevice.haveManufacturerData()) {
            std::string mData = advertisedDevice.getManufacturerData();
            uint8_t* payload = (uint8_t*)mData.data();
            uint16_t manufID = (payload[1] << 8) | payload[0]; // Läs ut ID
            
            // Kolla Victron (0x02E1, sparat som E1 02 i Little Endian)
            if (manufID == 0x02E1) {
                parseVictronAdvertisement(macAddress, payload, mData.length());
                return;
            }
            // Kolla RuuviTag (0x0499, aktiverad av användaren i inställningar)
            else if (ruuvi.cfg.enabled && ruuvi.cfg.mac_or_ip.equalsIgnoreCase(macAddress) && manufID == 0x0499) {
                parseRuuviTag(payload, mData.length());
                return;
            }
        }
        
        // 2. Hantera XIAOMI MIJIA (Använder oftast Service Data UUID 0x181A eller 0xFE95)
        if (mijia.cfg.enabled && mijia.cfg.mac_or_ip.equalsIgnoreCase(macAddress)) {
            // Kontrollera om enheten skickar Service Data
            if (advertisedDevice.haveServiceData()) {
                std::string sData = advertisedDevice.getServiceData();
                uint8_t* payload = (uint8_t*)sData.data();
                
                parseXiaomiMijia(payload, sData.length());
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
    Serial.println("[BLE] Universell BLE-skanning aktiv för Victron, Ruuvi & Xiaomi.");
}

#endif // BLE_MANAGER_H
