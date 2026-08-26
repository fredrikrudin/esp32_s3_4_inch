#include "storage_manager.h"
#include <LittleFS.h>

bool initStorage() {
    Serial.println("[Storage] Initierar LittleFS...");
    
    // Starta filsystemet (formatera om det kraschar)
    if (!LittleFS.begin(true)) {
        Serial.println("[ERROR] LittleFS kunde inte monteras!");
        return false;
    }
    
    Serial.println("[Storage] LittleFS monterat utan problem.");
    return true;
}

String loadWifiSSID() {
    File file = LittleFS.open("/wifi_ssid.txt", "r");
    if (!file) return "";
    String ssid = file.readStringUntil('\n');
    ssid.trim();
    file.close();
    return ssid;
}

String loadWifiPass() {
    File file = LittleFS.open("/wifi_pass.txt", "r");
    if (!file) return "";
    String pass = file.readStringUntil('\n');
    pass.trim();
    file.close();
    return pass;
}
