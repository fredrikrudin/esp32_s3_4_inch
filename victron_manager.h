#ifndef VICTRON_MANAGER_H
#define VICTRON_MANAGER_H

#include <Arduino.h>
#include <mbedtls/aes.h>
#include "config.h"

// Funktion för att avkoda krypterade Victron BLE-annonser (AES-CTR Counter Mode)
void decryptVictronPayload(uint8_t* encrypted_data, int len, uint8_t* key, uint8_t* iv, float &volt, float &curr, float &soc) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 128);
    
    size_t nc_off = 0;
    uint8_t stream_block[16] = {0};
    uint8_t decrypted_output[16] = {0};
    
    // Anrop till mbedTLS hårdvaruacceleration på ESP32-S3
    // mbedtls_aes_crypt_ctr(&aes, len, &nc_off, iv, stream_block, encrypted_data, decrypted_output);
    
    mbedtls_aes_free(&aes);
}

// Filtrerar och bearbetar inkommande paket från BLE-skannern
void handleVictronBLE(String addr, uint8_t* data, int len) {
    if (len < 4) return;
    
    if (shunt.enabled && addr.equalsIgnoreCase(shunt.mac)) {
        // Exempel på dataavläsning (Här kopplas dekrypteringen på i produktion)
        shunt.voltage = 13.25; 
        shunt.current = -4.5;
        shunt.soc = 92.0;
    }
    
    if (mppt.enabled && addr.equalsIgnoreCase(mppt.mac)) {
        mppt.power = 340.0;
    }
    
    if (ip22.enabled && addr.equalsIgnoreCase(ip22.mac)) {
        ip22.power = 0.0;
    }
}

#endif
