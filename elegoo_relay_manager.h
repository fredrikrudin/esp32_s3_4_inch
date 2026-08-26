/**
 * ============================================================================
 * 📊 VenusOS GUI v2 Klon för Waveshare ESP32-S3-Touch-LCD-4
 * ============================================================================
 * Huvudfil utvecklad för säker exekvering utan hårdvarukrockar.
 * Relästyrningen har flyttats helt till I2C för att rädda USB-C-porten.
 * 
 * Hårdvaruinställningar (Tools i Arduino IDE):
 * - Board: ESP32S3 Dev Module
 * - Flash Size: 16MB (128Mb)
 * - Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
 * - PSRAM: OPI PSRAM
 * - USB CDC On Boot: ENABLED 🟢 (Kritiskt för USB-kommunikation)
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include "config.h"

// Inkludera alla modulhanterare
#include "ui_manager.h"
#include "elegoo_relay_manager.h"
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
DiscoveredDevice discoveryList[20];
int discoveredCount = 0;

// Nätverk- och systeminställningar
String wifi_ssid = "DITT_WIFI_SSID";
String wifi_pass = "DITT_WIFI_LÖSENORD";
int update_interval = 1000;
int display_brightness = 127; // PWM-styrning (0-255)

// Tidshantering för schemaläggning (Hämtas via NTP eller RTC)
int currentHour = 12;
int currentMinute = 0;

// Tidtagare för asynkrona loopar (Förhindrar blockering)
unsigned long lastNetworkUpdate = 0;
unsigned long lastScheduleCheck = 0;
unsigned long lastDiagUpdate = 0;

// ----------------------------------------------------------------------------
// 2. SETUP: Initieringssekvens (Körs en gång vid uppstart)
// ----------------------------------------------------------------------------
void setup() {
    // Starta seriell kommunikation direkt för loggning.
    // Tvinga ALDRIG en "while(!Serial);" här eftersom det blockerar extern drift.
    Serial.begin(115200);
    delay(500); // Kort paus för att låta spänningen stabiliseras
    Serial.println("\n[SYSTEM] Startar VenusOS GUI v2 Klon...");

    // Steg A: Initiera Waveshare LCD-skärmen, Touch-gränssnittet och LVGL (v8.3)
    // Detta måste göras FÖRST eftersom skärmbiblioteket sätter upp intern hårdvara.
    Serial.println("[SYSTEM] Initierar display och LVGL...");
    initDisplayAndUI(); 

    // Steg B: Läs in konfiguration (Sätt grundtillstånd för dina reläsystem)
    elegoo.enabled_4ch = false; // Sätt till true om du använder 4-kanalsmodulen
    elegoo.enabled_8ch = true;  // Ditt primära 8-kanals reläkort över I2C

    // Steg C: Initiera det nya I2C-reläsystemet via PCF8574
    // Denna startar Wire på de säkra externa stiften GPIO 15 (SDA) och GPIO 7 (SCL).
    initElegooRelays();

    // Steg D: Starta trådlösa nätverkstjänster asynkront
    Serial.println("[SYSTEM] Initierar Wi-Fi och asynkron webbserver...");
    initNetworkManager(wifi_ssid, wifi_pass);

    // Steg E: Starta passiv BLE-avläsning (SmartShunt, RuuviTag, Xiaomi)
    Serial.println("[SYSTEM] Startar bakgrundsskanning för BLE-sensorer...");
    initBLEManager();

    Serial.println("[SYSTEM] Setup slutförd utan hårdvarukrockar. Loop startad.");
}

// ----------------------------------------------------------------------------
// 3. LOOP: Huvudloop (Körs kontinuerligt)
// ----------------------------------------------------------------------------
void loop() {
    unsigned long currentMillis = millis();

    // Tvingande uppdatering: LVGL-grafikmotor och touch-inmatning
    // Detta hanterar svepgester, flödesanimationer och skärmuppdateringar.
    lv_timer_handler();

    // Uppdatering A: Kontrollera reläscheman en gång i minuten (60000 ms)
    if (currentMillis - lastScheduleCheck >= 60000) {
        lastScheduleCheck = currentMillis;
        
        // Här synkroniseras klockan i bakgrunden (exempelvis från network_manager eller RTC)
        // currentHour = getNetworkHour();
        // currentMinute = getNetworkMinute();
        
        updateRelaySchedules(currentHour, currentMinute);
    }

    // Uppdatering B: Hantera JSON-synk och webbserver-trafik (Varje sekund)
    if (currentMillis - lastNetworkUpdate >= (unsigned long)update_interval) {
        lastNetworkUpdate = currentMillis;
        
        // Uppdatera Victron-data, Shelly-status och Wi-Fi-indikatorn i GUI:t
        updateNetworkData();
        updateVictronData();
    }

    // Uppdatering C: Systemdiagnostik och minnesövervakning (Var 5:e sekund)
    if (currentMillis - lastDiagUpdate >= 5000) {
        lastDiagUpdate = currentMillis;
        runSystemDiagnostics(); 
    }

    // ------------------------------------------------------------------------
    // KRITISKT FÖR NATIVE USB: Systemets andningspaus
    // ------------------------------------------------------------------------
    // Utan detta lilla delay kommer loop() att svälta ut bakgrundskärnan (Core 0)
    // där ESP32-S3:s inbyggda USB CDC-drivrutin körs. 5 ms förhindrar att 
    // datorn tappar anslutningen eller visar "Enheten kändes inte igen".
    delay(5); 
}
