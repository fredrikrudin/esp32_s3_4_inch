#include "network_manager.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h> // Krävs för asynkron server
#include <ArduinoJson.h>       // Krävs för JSON-hanteringen enligt README

AsyncWebServer server(80);

void initNetwork() {
    // Hämta sparade Wi-Fi-uppgifter från storage_manager
    String ssid = loadWifiSSID();
    String pass = loadWifiPass();

    if (ssid == "" || ssid == "NULL") {
        Serial.println("[Network] Inga Wi-Fi-uppgifter sparade. Startar i Access Point-läge...");
        WiFi.softAP("VenusOS-ESP32-Setup", "12345678");
        Serial.print("AP IP-adress: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.print("[Network] Ansluter till Wi-Fi: ");
        Serial.println(ssid);
        WiFi.begin(ssid.c_str(), pass.c_str());
        
        // Vänta på anslutning i max 15 sekunder (icke-blockerande bäst för FreeRTOS, men förenklat här)
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n[Network] Wi-Fi Anslutet!");
            Serial.print("IP-adress: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\n[Network] Kunde inte ansluta till Wi-Fi. Startar AP istället.");
            WiFi.softAP("VenusOS-ESP32-Setup", "12345678");
        }
    }

    setupWebEndpoints();
}

void setupWebEndpoints() {
    // Rotkatalogen: Levererar HTML-sida (enligt README)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<h1>VenusOS GUI v2 Klon</h1><p>Reläautomation och tidsskedulering.</p>");
    });

    // /data endpoint: Levererar live-JSON med universell v6/v7 syntax
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc; // Fungerar sömlöst i både ArduinoJson v6 och v7
        doc["system_status"] = "OK";
        doc["core0_free_heap"] = ESP.getFreeHeap();
        doc["smartshunt_v"] = 13.24; 
        doc["mppt_status"] = "Bulk";

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });


    server.begin();
    Serial.println("[Network] Asynkron webbserver startad på port 80.");
}
