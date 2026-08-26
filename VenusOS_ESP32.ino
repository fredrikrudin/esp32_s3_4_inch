/**
 * ============================================================================
 * 📊 VenusOS GUI v2 Klon för Waveshare ESP32-S3-Touch-LCD-4
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include "config.h"

// Inkludera alla hanterare
#include "storage_manager.h"
#include "ui_manager.h"
#include "backlight_manager.h"
#include "elegoo_relay_manager.h"
#include "network_manager.h" // Innehåller den nya webbservern
#include "ble_manager.h"

// Allokering av globala variabler
VictronDevice shunt, mppt, ip22;
EcoWorthyDevice ecoBatt;
RuuviTagDevice ruuvi;
XiaomiMijiaDevice mijia;
ShellyDevice shellyPro1, shellyPro2;

// Ditt lokala nätverk
String wifi_ssid = "DITT_WIFI_SSID";
String wifi_pass = "DITT_WIFI_LÖSENORD";
int update_interval = 1000;
int display_brightness = 127;

int currentHour = 12;
int currentMinute = 0;
unsigned long lastNetworkUpdate = 0;
unsigned long lastScheduleCheck = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[SYSTEM] Initierar VenusOS Core...");

    // Ladda sparade namn och adresser från NVS Flash
    loadAllSettings();

    // Sätt upp Shelly-kanaler (Pro 1 har 1, Pro 2 har 2)
    shellyPro1.total_channels = 1;
    shellyPro2.total_channels = 2;

    shunt.deviceType = 1; 
    mppt.deviceType = 2;  
    ip22.deviceType = 3;  

    // Initiera skärm, touch och belysning (v1-v4 kompatibel)
    initDisplayAndUI(); 
    initBacklight();

    // Initiera lokala I2C-reläer (PCF8574)
    elegoo.enabled_8ch = true;
    initElegooRelays();

    // Starta Wi-Fi och BLE asynkront
    initNetworkManager(wifi_ssid, wifi_pass);
    initBLEManager();

    Serial.println("[SYSTEM] Systemstart klar.");
}

void loop() {
    unsigned long currentMillis = millis();

    // Kör LVGL-grafikmotor och touch-avkänning (Tvingande varje loopvarv)
    lv_timer_handler();

    // Kontrollera Wi-Fi-status och hantera webbserver/JSON-synk (Varje sekund)
    if (currentMillis - lastNetworkUpdate >= (unsigned long)update_interval) {
        lastNetworkUpdate = currentMillis;
        updateNetworkData(); // Det är denna som tänder servern vid anslutning!
    }

    // Lokala reläscheman (En gång i minuten)
    if (currentMillis - lastScheduleCheck >= 60000) {
        lastScheduleCheck = currentMillis;
        updateRelaySchedules(currentHour, currentMinute);
    }

    // Kritiskt delay (5ms) för att ge Native USB-C tid att hålla kontakten stabil
    delay(5); 
}
