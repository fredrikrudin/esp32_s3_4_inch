#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

inline void saveVictronDeviceConfig(const char* namespaceKey, const VictronDevice &dev) {
    Preferences prefs;
    prefs.begin(namespaceKey, false);
    prefs.putString("name", dev.cfg.name);
    prefs.putString("addr", dev.cfg.mac_or_ip);
    prefs.putBool("en", dev.cfg.enabled);
    prefs.putString("v_key", dev.key);
    prefs.end();
}

inline void loadVictronDeviceConfig(const char* namespaceKey, VictronDevice &dev, String defaultAddr, String defaultKey, String defaultName) {
    Preferences prefs;
    prefs.begin(namespaceKey, true);
    dev.cfg.name = prefs.getString("name", defaultName);
    dev.cfg.mac_or_ip = prefs.getString("addr", defaultAddr);
    dev.cfg.enabled = prefs.getBool("en", false);
    dev.key = prefs.getString("v_key", defaultKey);
    prefs.end();
}

inline void saveVictronInverterConfig(const char* namespaceKey, const VictronInverter &dev) {
    Preferences prefs;
    prefs.begin(namespaceKey, false);
    prefs.putString("name", dev.cfg.name);
    prefs.putString("addr", dev.cfg.mac_or_ip);
    prefs.putBool("en", dev.cfg.enabled);
    prefs.putString("v_key", dev.key);
    prefs.end();
}

inline void loadVictronInverterConfig(const char* namespaceKey, VictronInverter &dev, String defaultAddr, String defaultKey, String defaultName) {
    Preferences prefs;
    prefs.begin(namespaceKey, true);
    dev.cfg.name = prefs.getString("name", defaultName);
    dev.cfg.mac_or_ip = prefs.getString("addr", defaultAddr);
    dev.cfg.enabled = prefs.getBool("en", false);
    dev.key = prefs.getString("v_key", defaultKey);
    prefs.end();
}

inline void saveDevice(const char* key, const DeviceConfig &cfg) {
    Preferences prefs;
    prefs.begin(key, false);
    prefs.putString("name", cfg.name);
    prefs.putString("addr", cfg.mac_or_ip);
    prefs.putBool("en", cfg.enabled);
    prefs.end();
}

inline void loadDevice(const char* key, DeviceConfig &cfg, String defaultAddr, String defaultName) {
    Preferences prefs;
    prefs.begin(key, true);
    cfg.name = prefs.getString("name", defaultName);
    cfg.mac_or_ip = prefs.getString("addr", defaultAddr);
    cfg.enabled = prefs.getBool("en", false);
    prefs.end();
}

inline void saveScheduleToNVS() {
    Preferences prefs;
    prefs.begin("venus_sched", false);
    prefs.putBytes("sched_data", &elegoo, sizeof(ElegooRelaySystem));
    prefs.putInt("ui_style", ui_style_version);
    prefs.putInt("brightness", display_brightness);
    prefs.end();
    Serial.println("[Storage] Reläscheman, ljusstyrka och GUI-stil synkade till Flash.");
}

inline void loadScheduleFromNVS() {
    Preferences prefs;
    prefs.begin("venus_sched", true);
    ui_style_version = prefs.getInt("ui_style", 2); 
    display_brightness = prefs.getInt("brightness", 255);
    
    if (prefs.isKey("sched_data")) {
        prefs.getBytes("sched_data", &elegoo, sizeof(ElegooRelaySystem));
        Serial.println("[Storage] Reläscheman och GUI-stil laddade från Flash.");
    } else {
        Serial.println("[Storage] Inga scheman hittades. Initierar tomma strukturer.");
        elegoo.enabled_4ch = false;
        elegoo.enabled_8ch = false;
        for(int i=0; i<4; i++) elegoo.schedule4[i].isEnabled = false;
        for(int i=0; i<8; i++) elegoo.schedule8[i].isEnabled = false;
    }
    prefs.end();
}

inline void saveAllSettings() {
    saveVictronDeviceConfig("shunt", shunt);
    saveVictronDeviceConfig("mppt", mppt);
    saveVictronDeviceConfig("ip22", ip22);
    saveVictronInverterConfig("inverter", inverter);
    saveDevice("ruuvi", ruuvi.cfg);
    saveDevice("mijia", mijia.cfg);
    saveDevice("sh_pro1", shellyPro1.cfg);
    saveDevice("sh_pro2", shellyPro2.cfg);
    saveScheduleToNVS();
    Serial.println("[Storage] Absolut alla systeminställningar sparade till Flash!");
}

inline void loadAllSettings() {
    loadVictronDeviceConfig("shunt", shunt, "00:11:22:33:44:55", "00000000000000000000000000000000", "Huvudshunt");
    loadVictronDeviceConfig("mppt", mppt, "66:77:88:99:AA:BB", "00000000000000000000000000000000", "Solceller MPPT");
    loadVictronDeviceConfig("ip22", ip22, "CC:DD:EE:FF:00:11", "00000000000000000000000000000000", "Landström IP22");
    loadVictronInverterConfig("inverter", inverter, "DD:EE:FF:11:22:33", "00000000000000000000000000000000", "Phoenix Inverter 230V");
    
    loadDevice("ruuvi", ruuvi.cfg, "11:22:33:44:55:66", "Kylskåp Temp");
    loadDevice("mijia", mijia.cfg, "AA:BB:CC:DD:EE:FF", "Salong Temp");
    loadDevice("sh_pro1", shellyPro1.cfg, "192.168.1.50", "Shelly Pro 1 Varmvatten");
    loadDevice("sh_pro2", shellyPro2.cfg, "192.168.1.51", "Shelly Pro 2 Belysning");
    
    loadScheduleFromNVS();
    Serial.println("[Storage] Samtliga krypteringskonfigurationer och scheman inlästa.");
}

#endif // STORAGE_MANAGER_H
