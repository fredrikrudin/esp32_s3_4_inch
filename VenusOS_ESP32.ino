/**
 * ============================================================================
 * 📊 VenusOS GUI v2 Klon för Waveshare ESP32-S3-Touch-LCD-4
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include "config.h"

#include "storage_manager.h"
#include "ui_manager.h"
#include "backlight_manager.h"
#include "elegoo_relay_manager.h"
#include "network_manager.h"
#include "ble_manager.h"

VictronDevice shunt, mppt, ip22;
EcoWorthyDevice ecoBatt;
RuuviTagDevice ruuvi;
XiaomiMijiaDevice mijia;
ShellyDevice shellyPro1, shellyPro2;
ElegooRelaySystem elegoo;

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
    Serial.println("\n[SYSTEM] Startar VenusOS Core...");

    loadAllSettings();

    shellyPro1.total_channels = 1;
    shellyPro2.total_channels = 2;

    shunt.deviceType = 1; 
    mppt.deviceType = 2;  
    ip22.deviceType = 3;  

    initDisplayAndUI(); 
    initBacklight();

    elegoo.enabled_8ch = true;
    initElegooRelays();

    initNetworkManager(wifi_ssid, wifi_pass);
    initBLEManager();

    Serial.println("[SYSTEM] Startsekvens klar.");
}

void loop() {
    unsigned long currentMillis = millis();

    lv_timer_handler();

    if (currentMillis - lastNetworkUpdate >= (unsigned long)update_interval) {
        lastNetworkUpdate = currentMillis;
        updateNetworkData(); 
    }

    if (currentMillis - lastScheduleCheck >= 60000) {
        lastScheduleCheck = currentMillis;
        updateRelaySchedules(currentHour, currentMinute);
    }

    delay(5); 
}
