#ifndef DISCOVERY_MANAGER_H
#define DISCOVERY_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <WiFi.h>
#include "config.h"

// Funktion från ui_manager.h för att mata in rader i popup-listan live
extern void ui_add_discovered_device(const char* name, const char* addr);

class DiscoveryBLECallbacks: public BLEAdvertisedDeviceCallbacks {
    String _filterType;
public:
    DiscoveryBLECallbacks(String filterType) : _filterType(filterType) {}

    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String mac = advertisedDevice.getAddress().toString().c_str();
        String name = advertisedDevice.haveName() ? advertisedDevice.getName().c_str() : "";

        // 1. Sök efter Victron (Manufacturer Data ID: 0x02E1)
        if (_filterType == "VICTRON" && advertisedDevice.haveManufacturerData()) {
            std::string mData = advertisedDevice.getManufacturerData();
            uint8_t* payload = (uint8_t*)mData.data();
            if (mData.length() > 2 && payload[0] == 0xE1 && payload[1] == 0x02) {
                ui_add_discovered_device("Victron Energi Enhet", mac.c_str());
            }
        }
        // 2. Sök efter RuuviTag (Manufacturer Data ID: 0x0499)
        else if (_filterType == "RUUVI" && advertisedDevice.haveManufacturerData()) {
            std::string mData = advertisedDevice.getManufacturerData();
            uint8_t* payload = (uint8_t*)mData.data();
            if (mData.length() > 2 && payload[0] == 0x99 && payload[1] == 0x04) {
                ui_add_discovered_device("RuuviTag Sensor", mac.c_str());
            }
        }
        // 3. Sök efter Xiaomi Mijia (Känner igen runda/fyrkantiga Bluetooth-termometrar på namnet)
        else if (_filterType == "XIAOMI" && name.startsWith("LYWSD03")) {
            ui_add_discovered_device("Xiaomi Mijia Temp", mac.c_str());
        }
    }
};

/**
 * Startar en fokuserad maskinvaruskanning i 5 sekunder baserat på vald enhetstyp
 */
void triggerManualDiscovery(const char* typeFilter) {
    String filter = String(typeFilter);
    Serial.printf("[Discovery] Startar aktiv genomsökning efter enhetstyp: %s\n", typeFilter);

    if (filter == "VICTRON" || filter == "RUUVI" || filter == "XIAOMI") {
        // Pausa den ordinarie passiva bakgrundsskanningen under söksekvensen
        BLEDevice::getScan()->stop();

        BLEScan* pScan = BLEDevice::getScan();
        pScan->setAdvertisedDeviceCallbacks(new DiscoveryBLECallbacks(filter));
        pScan->setActiveScan(true); // Aktiv skanning tvingar sensorerna att svara med namn
        pScan->start(5, nullptr, false); // Skanna i exakt 5 sekunder (blockerar tråden kort under skärmpopupen)
        
        Serial.println("[Discovery] Sökning avslutad.");
    } 
    else if (filter == "SHELLY") {
        // NÄTVERKSSKANNING: Shelly Pro-enheter söks över det lokala nätverket via Wi-Fi.
        // För högsta kompatibilitet och snabbhet skjuter vi ut standard IP-förslag i nätverkssegmentet.
        Serial.println("[Discovery] Genererar lokala nätverksförslag för Shelly Pro RPC...");
        
        // Hämta ESP32:s nuvarande nätverksserie (t.ex. 192.168.1.X) för att gissa Shelly-IPn
        String localIP = WiFi.localIP().toString();
        int lastDot = localIP.lastIndexOf('.');
        if (lastDot != -1) {
            String subnet = localIP.substring(0, lastDot + 1);
            // Skicka ut tre vanliga fasta adresser i subnätet som valbara snabbknappar i listan
            ui_add_discovered_device("Shelly Pro Enhet (Standard 1)", (subnet + "50").c_str());
            ui_add_discovered_device("Shelly Pro Enhet (Standard 2)", (subnet + "51").c_str());
            ui_add_discovered_device("Shelly Pro Enhet (Standard 3)", (subnet + "100").c_str());
        } else {
            ui_add_discovered_device("Shelly Pro 1 (Fallback IP)", "192.168.1.50");
            ui_add_discovered_device("Shelly Pro 2 (Fallback IP)", "192.168.1.51");
        }
    }
}

#endif // DISCOVERY_MANAGER_H
