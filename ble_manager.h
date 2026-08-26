#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "mbedtls/aes.h" // ESP32 inbyggda hårdvaru-kryptobibliotek
#include "config.h"

/**
 * Hjälpfunktion för att konvertera Victrons 32-teckens hex-nyckel från 
 * inställningssidan till en rå 16-bytes array för kryptomotorn.
 */
bool hexStringToBytes(String hexStr, uint8_t* byteArr, int maxLen) {
    if (hexStr.length() != maxLen * 2) return false;
    for (int i = 0; i < maxLen; i++) {
        String byteString = hexStr.substring(i * 2, i * 2 + 2);
        byteArr[i] = (uint8_t) strtol(byteString.c_str(), NULL, 16);
    }
    return true;
}

/**
 * Utför AES-128-CTR dekryptering på Victrons datablock.
 * Protokollet använder ett Little-Endian salt/counter baserat på paket-id (Nonce).
 */
bool decryptVictronPayload(uint8_t* encryptedData, int dataLen, uint8_t* key, uint16_t nonce, uint8_t* decryptedOutput) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    // Sätt AES-nyckeln för dekryptering
    if (mbedtls_aes_setkey_enc(&aes, key, 128) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }

    // Bygg Victrons 16-bytes Counter Block (IV / Nonce)
    uint8_t stream_block[16] = {0};
    uint8_t iv[16] = {0};
    
    // Victron specifikt: Nonce sparas i byte 0 och 1 i Little-Endian format
    iv[0] = nonce & 0xFF;
    iv[1] = (nonce >> 8) & 0xFF;
    // Resterande bytes i IV lämnas som 0 enligt Victrons specifikation

    size_t nc_off = 0;
    
    // Utför AES-CTR dekryptering (In-place eller till utgångs-array)
    int res = mbedtls_aes_crypt_ctr(&aes, dataLen, &nc_off, iv, stream_block, encryptedData, decryptedOutput);
    
    mbedtls_aes_free(&aes);
    return (res == 0);
}

/**
 * Avkodar den råa, dekrypterade datan baserat på enhetstyp (Shunt eller MPPT)
 */
void decodeVictronMetrics(VictronDevice* target, uint8_t* data, int len) {
    if (len < 4) return; // För kort datablock

    if (target->deviceType == 1) { // 1 = SmartShunt / Batterimonitor
        // Avkodning enligt Victron specifikation för BMV/SmartShunt
        int16_t raw_current = (data[1] << 8) | data[0]; 
        uint16_t raw_voltage = (data[3] << 8) | data[2];
        uint16_t raw_soc = (data[5] << 8) | data[4];

        target->voltage = (float)raw_voltage / 100.0;     // Volt (t.ex. 1320 -> 13.20V)
        target->current = (float)raw_current / 100.0;     // Amperere
        target->soc = (float)(raw_soc & 0x3FFF) / 10.0;   // State of Charge (t.ex. 985 -> 98.5%)
        target->power = target->voltage * target->current; // Beräkna Watt lokalt
    } 
    else if (target->deviceType == 2) { // 2 = MPPT Solcellsregulator
        // Avkodning enligt Victron specifikation för MPPT
        uint16_t raw_voltage = (data[1] << 8) | data[0];
        int16_t raw_current = (data[3] << 8) | data[2];
        uint16_t raw_power = (data[5] << 8) | data[4];

        target->voltage = (float)raw_voltage / 100.0;
        target->current = (float)raw_current / 10.0;
        target->power = (float)raw_power;
        target->soc = 100.0; // MPPT har ingen SoC, sätt till statiskt värde för GUI:t
    }
}

/**
 * Filtrerar och hanterar inkommande Victron BLE-paket (Instant Readout)
 */
void parseVictronAdvertisement(String mac, uint8_t* payload, int length) {
    VictronDevice* target = nullptr;

    // Sök efter aktiverad enhet med matchande MAC
    if (shunt.cfg.enabled && shunt.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &shunt;
    else if (mppt.cfg.enabled && mppt.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &mppt;
    else if (ip22.cfg.enabled && ip22.cfg.mac_or_ip.equalsIgnoreCase(mac)) target = &ip22;

    if (target == nullptr) return; // Enheten inaktiverad eller okänd

    // Kontrollera att paketet är tillräckligt långt för att innehålla krypteringstaggarna
    // Victron Manufacturer Header är 4 bytes. Krypterad data börjar vid byte 4.
    if (length < 12) return;

    // Hämta krypteringsnyckeln (Konvertera från användarens sparade HEX-sträng)
    uint8_t aesKey[16];
    if (!hexStringToBytes(target->key, aesKey, 16)) {
        Serial.printf("[BLE] Felaktig nyckellängd i config för enhet: %s\n", target->cfg.name.c_str());
        return;
    }

    // Extrahera Nonce/Counter-id från paketet (ligger efter tillverkarens ID)
    uint16_t packetNonce = (payload[5] << 8) | payload[4];

    // Det krypterade datablocket börjar vid byte 6
    int encryptedDataLen = length - 6;
    uint8_t* encryptedDataBlock = &payload[6];
    
    // Array för att spara det upplåsta resultatet
    uint8_t decryptedPayload[32] = {0};

    // Kör dekrypteringen via hårdvaran
    if (decryptVictronPayload(encryptedDataBlock, encryptedDataLen, aesKey, packetNonce, decryptedPayload)) {
        // Dekryptering lyckades! Gå vidare till att parsa ut spänning och ström
        decodeVictronMetrics(target, decryptedPayload, encryptedDataLen);
        
        Serial.printf("[BLE] %s avkodad: %.2fV, %.2fA, %.0fW, SoC: %.1f%%\n", 
                      target->cfg.name.c_str(), target->voltage, target->current, target->power, target->soc);
    } else {
        Serial.printf("[BLE] Dekrypteringsfel för %s. Kontrollera din App-nyckel.\n", target->cfg.name.c_str());
    }
}

/**
 * Huvud-callback för BLE-skannern
 */
class MyBLEAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String macAddress = advertisedDevice.getAddress().toString().c_str();
        
        if (advertisedDevice.haveManufacturerData()) {
            std::string mData = advertisedDevice.getManufacturerData();
            uint8_t* payload = (uint8_t*)mData.data();
            
            // Kontrollera Victrons unika tillverkarsignatur (0xE1, 0x02)
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
    pBLEScan->setActiveScan(false); // Passiv skanning sparar ström
    pBLEScan->start(0, nullptr, false); // Skanna kontinuerligt i bakgrunden
    Serial.println("[BLE] Krypterad Victron BLE-hanterare startad.");
}

#endif // BLE_MANAGER_H
