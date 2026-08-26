#include "discovery_manager.h"
#include "elegoo_relay_manager.h"
#include <Arduino.h>

void initDiscovery() {
    Serial.println("[Discovery] Initierar bakgrundsautomation på Core 0...");
}

void processAutomation() {
    // Den här funktionen anropas från loop() eller en FreeRTOS-task på Core 0.
    // Här läggs logiken för NTP-synkad tidsskedulering och Shelly RPC-anrop till.
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 1000) { // Körs en gång i sekunden
        lastCheck = millis();
        
        // Exempel: Kontrollera om tidur matchar aktuell tid (NTP)
        // Om match: anropa setRelay() eller skicka Shelly-kommando
    }
}
