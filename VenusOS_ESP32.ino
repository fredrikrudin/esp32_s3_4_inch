/**
 * ============================================================================
 * 📊 VenusOS GUI v2 Klon för Waveshare ESP32-S3-Touch-LCD-4
 * ============================================================================
 * Fullständig huvudfil optimerad för stabilitet över alla kortrevisioner.
 * Innehåller fixar för Native USB, I2C-reläer samt automatisk v4-belysning.
 * 
 * Hårdvaruinställningar (Tools i Arduino IDE):
 * - Board: ESP32S3 Dev Module
 * - Flash Size: 16MB (128Mb)
 * - Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
 * - PSRAM: OPI PSRAM
 * - USB CDC On Boot: ENABLED 🟢 (Tvingande för att inte blockera USB-C)
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include "config.h"

// Inkludera alla dina modulhanterare
#include "ui_manager.h"
#include "backlight_manager.h"   // Inkluderar det nya belysningssystemet
#include "elegoo_relay_manager.h" // Inkluderar det nya I2C-reläsystemet
#include "network_manager.h"
#include "ble_manager.h"
#include "victron_manager.h"
#include "ruuvi_manager.h"
#include "xiaomi_manager.h"
#include "system_diagnostics.h"

// ----------------------------------------------------------------------------
// 1. Definition av globala variabler (Deklarerade som 'extern' i config.h)
// ----------------------------------------------------------------------------
VictronDevice shunt, mppt, ip22;
EcoWorthyDevice ecoBatt;
RuuviTagDevice ruuvi;
XiaomiMijiaDevice mijia;
ShellyDevice shelly;
ElegooRelaySystem elegoo;
DiscoveredDevice discoveryList;
int discoveredCount = 0;

// Nätverks- och skärminställningar
String wifi_ssid = "DITT_WIFI_SSID";
String wifi_pass = "DITT_WIFI_LÖSENORD";
int update_interval = 1000;
int display_brightness = 127; // Globalt startvärde för ljusstyrka (0-255)

// Tidshantering för schemaläggning (NTP/RTC)
int currentHour = 12;
int currentMinute = 0;

// Tidtagare för asynkrona loopar (Ersätter blockerande delay)
unsigned long lastNetworkUpdate = 0;
unsigned long lastScheduleCheck = 0;
unsigned long lastDiagUpdate = 0;

// ----------------------------------------------------------------------------
// 2. SETUP: Initieringssekvens (Körs en gång vid uppstart)
// ----------------------------------------------------------------------------
void setup() {
    // Starta seriell loggning direkt. 
    // Ingen "while(!Serial);" här eftersom kortet måste kunna boota utan dator!
    Serial.begin(115200);
    delay(500); 
    Serial.println("\n[SYSTEM] Startar VenusOS GUI v2 Klon...");

    // Steg A: Initiera Waveshare LCD-skärmen, Touch och LVGL (v8.3)
    // Detta sätter upp skärmens grundläggande hårdvaruregister.
    Serial.println("[SYSTEM] Initierar display och LVGL-grafikmotor...");
    initDisplayAndUI(); 

    // Steg B: Initiera bakgrundsbelysningen (Detekterar automatiskt v1-v3 eller v4)
    // Denna funktion läser av den interna expandern och sätter startljusstyrkan.
    Serial.println("[SYSTEM] Konfigurerar bakgrundsbelysning...");
    initBacklight();

    // Steg C: Läs in reläkonfigurationen i systemminnet
    elegoo.enabled_4ch = false; // Ändra till true om du kör det mindre kortet
    elegoo.enabled_8ch = true;  // Ditt primära 8-kanals reläkort över I2C

    // Steg D: Initiera det nya, säkra I2C-reläsystemet via PCF8574
    // Öppnar I2C-anslutningen mot de externa stiften (GPIO 15 och 7)
    initElegooRelays();

    // Steg E: Starta trådlösa nätverkstjänster (Wi-Fi & Webbserver) asynkront
    Serial.println("[SYSTEM] Startar nätverkshanterare och webbserver...");
    initNetworkManager(wifi_ssid, wifi_pass);

    // Steg F: Starta passiv BLE-avläsning (SmartShunt, Ruuvi, Xiaomi)
    Serial.println("[SYSTEM] Aktiverar bakgrundsskanning för Bluetooth-sensorer...");
    initBLEManager();

    Serial.println("[SYSTEM] Systemstart slutförd! Huvudloopen är nu aktiv.");
}

// ----------------------------------------------------------------------------
// 3. LOOP: Huvudloop (Körs kontinuerligt)
// ----------------------------------------------------------------------------
void loop() {
    unsigned long currentMillis = millis();

    // Tvingande uppdatering: Hanterar touch-inmatning, animationer och LVGL-gränssnittet
    lv_timer_handler();

    // Uppdatering A: Kontrollera reläscheman asynkront en gång i minuten (60000 ms)
    if (currentMillis - lastScheduleCheck >= 60000) {
        lastScheduleCheck = currentMillis;
        
        // Här kan du lägga till kod för att uppdatera klockan från NTP/RTC innan kontroll:
        // currentHour = getNetworkHour();
        // currentMinute = getNetworkMinute();
        
        updateRelaySchedules(currentHour, currentMinute);
    }

    // Uppdatering B: Hantera JSON-synk, Shelly och Victron-data (Varje sekund)
    if (currentMillis - lastNetworkUpdate >= (unsigned long)update_interval) {
        lastNetworkUpdate = currentMillis;
        
        updateNetworkData();
        updateVictronData();
    }

    // Uppdatering C: Systemdiagnostik och RAM-övervakning (Var 5:e sekund)
    if (currentMillis - lastDiagUpdate >= 5000) {
        lastDiagUpdate = currentMillis;
        runSystemDiagnostics(); 
    }

    // ------------------------------------------------------------------------
    // KRITISKT FÖR NATIVE USB-C: Systemets andningspaus
    // ------------------------------------------------------------------------
    // Denna korta paus på 5 ms är livsviktig för ESP32-S3. Den ger chippets 
    // bakgrundskärna tid att köra USB CDC-drivrutinen. Detta förhindrar helt
    // att datorn tappar COM-porten eller kastar USB-anslutningsfel.
    delay(5); 
}
