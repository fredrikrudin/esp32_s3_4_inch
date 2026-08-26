#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

Preferences prefs;

void saveDevice(const char* key, const DeviceConfig &cfg) {
    prefs.begin(key, false);
    prefs.putString("name", cfg.name);
    prefs.putString("addr", cfg.mac_or_ip);
    prefs.putBool("en", cfg.enabled);
    prefs.end();
}

void loadDevice(const char* key, DeviceConfig &cfg, String defaultAddr, String defaultName) {
    prefs.begin(key, true);
    cfg.name = prefs.getString("name", defaultName);
    cfg.mac_or_ip = prefs.getString("addr", defaultAddr);
    cfg.enabled = prefs.getBool("en", false);
    prefs.end();
}

void saveAllSettings() {
    saveDevice("shunt", shunt.cfg);
    saveDevice("mppt", mppt.cfg);
    saveDevice("ip22", ip22.cfg);
    saveDevice("ruuvi", ruuvi.cfg);
    saveDevice("mijia", mijia.cfg);
    saveDevice("sh_pro1", shellyPro1.cfg);
    saveDevice("sh_pro2", shellyPro2.cfg);
    Serial.println("[Storage] Inställningar sparades permanent i NVS Flash!");
}

void loadAllSettings() {
    loadDevice("shunt", shunt.cfg, "00:11:22:33:44:55", "Huvudshunt");
    loadDevice("mppt", mppt.cfg, "66:77:88:99:AA:BB", "Solceller MPPT");
    loadDevice("ip22", ip22.cfg, "CC:DD:EE:FF:00:11", "Landström IP22");
    loadDevice("ruuvi", ruuvi.cfg, "11:22:33:44:55:66", "Kylskåp Temp");
    loadDevice("mijia", mijia.cfg, "AA:BB:CC:DD:EE:FF", "Salong Temp");
    loadDevice("sh_pro1", shellyPro1.cfg, "192.168.1.50", "Shelly Pro 1 Varmvatten");
    loadDevice("sh_pro2", shellyPro2.cfg, "192.168.1.51", "Shelly Pro 2 Belysning");
    Serial.println("[Storage] Enhetskonfigurationer inlästa från NVS Flash.");
}

#endif // STORAGE_MANAGER_H
