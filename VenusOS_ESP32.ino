/**
 * VenusOS GUI v2 Klon för Waveshare ESP32-S3-Touch-LCD-4 (Revision 4)
 * Arkitektur: Asynkron FreeRTOS-skedulering med dubbla I2C-bussar.
 */

// 1. KRITISKT: Inkluderingsordning enligt README för länkning/kompilering
#include "storage_manager.h"   // Måste ligga först!
#include "network_manager.h"   // Läser Wi-Fi via storage_manager
#include "backlight_manager.h" // Hanterar v4 TCA9554 IO-expander
#include "ui_manager.h"        // Startar LVGL på Core 1
#include "elegoo_relay_manager.h" // Styr extern I2C-buss (Buss 1)
#include "discovery_manager.h" // Automation och tidsskedulering
#include "ble_manager.h"       // Passiv BLE-läsning på Core 0

// Definitioner för FreeRTOS tasks på Core 0
TaskHandle_t automationTaskHandle = NULL;

// FreeRTOS Task: Hanterar all tung radiotrafik och automation på Core 0 
// Detta håller Core 1 helt fri för flytande 200Hz grafik!
void core0_automation_task(void *pvParameters) {
    Serial.println("[System] Startar Core 0 bakgrunds-task...");
    
    // Initiera BLE-skannern (Körs passivt på Core 0)
    initBLE();
    startBLEScan();

    while (1) {
        // Kör tidsskedulering, Shelly RPC-anrop och automation
        processAutomation();
        
        // Ge FreeRTOS tid att andas för att förhindra watchdog-triggers
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void setup() {
    // Starta seriell kommunikation (Kritiskt: Kräver USB CDC On Boot: ENABLED i IDE)
    Serial.begin(115200);
    delay(1000); 
    Serial.println("\n=== Startar VenusOS GUI v2 Klon (Rev v4) ===");

    // 1. Initiera filsystemet först (laddar Wi-Fi-inställningar)
    if (!initStorage()) {
        Serial.println("[CRITICAL] Filsystem fel! Systemet fortsätter med standardvärden.");
    }

    // 2. Initiera bakgrundsbelysning för v4-kort (startar interna I2C-bussen 400kHz)
    initBacklight();
    setBacklightBrightness(100); // Sätt skärmen till full styrka

    // 3. Initiera externa I2C-bussen (Buss 1, 100kHz) för Elegoo/PCF8574-reläer
    initRelays();

    // 4. Anslut till nätverk och starta den asynkrona webbservern (port 80)
    initNetwork();

    // 5. Initiera bakgrundsautomationen
    initDiscovery();

    // 6. Initiera LVGL och lås fast grafikprocessen till Core 1 (~200Hz)
    initUI();

    // 7. Skapa en FreeRTOS task och lås fast den till Core 0 för BLE och reläer
    xTaskCreatePinnedToCore(
        core0_automation_task,    // Funktionsnamn
        "Automation_Task",        // Task-namn
        4096,                     // Stackstorlek (4KB)
        NULL,                     // Parametrar
        1,                        // Prioritet (Lägre än grafik)
        &automationTaskHandle,    // Task handle
        0                         // Kärna 0
    );

    Serial.println("[System] Setup slutförd utan fel! Systemet körs.");
}

void loop() {
    // Tom! Eftersom vi använder en ren FreeRTOS-arkitektur körs allt i dedikerade 
    // tasks fastlåsta på Core 0 och Core 1. loop() lämnas tom för att spara resurser.
    vTaskDelete(NULL); 
}
