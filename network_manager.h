#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h> // Kräver biblioteket ESPAsyncWebServer och AsyncTCP
#include <HTTPClient.h>
#include "config.h"

// Initiera webbservern på standardport 80
AsyncWebServer server(80);

// Flagga för att hålla koll på om servern har startats
bool isWebServerStarted = false;

/**
 * Genererar en realtids-JSON med all data från systemet.
 * Denna endpoint nås via http://<esp32-ip>/data
 */
void handleJsonRequest(AsyncWebServerRequest *request) {
    // Bygg upp en ren JSON-sträng manuellt för att spara RAM-minne (undviker fragmentation)
    String json = "{";
    
    // 1. Systeminfo
    json += "\"system\":{\"brightness\":" + String(display_brightness) + "},";
    
    // 2. Victron BLE-enheter
    json += "\"victron\":{";
    json += "\"shunt\":{\"name\":\"" + shunt.cfg.name + "\",\"v\":" + String(shunt.voltage) + ",\"a\":" + String(shunt.current) + ",\"soc\":" + String(shunt.soc) + ",\"p\":" + String(shunt.power) + ",\"en\":" + String(shunt.cfg.enabled) + "},";
    json += "\"mppt\":{\"name\":\"" + mppt.cfg.name + "\",\"v\":" + String(mppt.voltage) + ",\"a\":" + String(mppt.current) + ",\"soc\":" + String(mppt.soc) + ",\"p\":" + String(mppt.power) + ",\"en\":" + String(mppt.cfg.enabled) + "},";
    json += "\"ip22\":{\"name\":\"" + ip22.cfg.name + "\",\"v\":" + String(ip22.voltage) + ",\"a\":" + String(ip22.current) + ",\"soc\":" + String(ip22.soc) + ",\"p\":" + String(ip22.power) + ",\"en\":" + String(ip22.cfg.enabled) + "}";
    json += "},";

    // 3. Sensorer (Ruuvi & Xiaomi)
    json += "\"sensors\":{";
    json += "\"ruuvi\":{\"name\":\"" + ruuvi.cfg.name + "\",\"t\":" + String(ruuvi.temperature) + ",\"h\":" + String(ruuvi.humidity) + ",\"en\":" + String(ruuvi.cfg.enabled) + "},";
    json += "\"xiaomi\":{\"name\":\"" + mijia.cfg.name + "\",\"t\":" + String(mijia.temperature) + ",\"h\":" + String(mijia.humidity) + ",\"bat\":" + String(mijia.battery_level) + ",\"en\":" + String(mijia.cfg.enabled) + "}";
    json += "},";

    // 4. Shelly Pro-reläer
    json += "\"shelly\":{";
    json += "\"pro1\":{\"name\":\"" + shellyPro1.cfg.name + "\",\"ip\":\"" + shellyPro1.cfg.mac_or_ip + "\",\"ch0\":" + String(shellyPro1.channel_states[0]) + ",\"en\":" + String(shellyPro1.cfg.enabled) + "},";
    json += "\"pro2\":{\"name\":\"" + shellyPro2.cfg.name + "\",\"ip\":\"" + shellyPro2.cfg.mac_or_ip + "\",\"ch0\":" + String(shellyPro2.channel_states[0]) + ",\"ch1\":" + String(shellyPro2.channel_states[1]) + ",\"en\":" + String(shellyPro2.cfg.enabled) + "}";
    json += "}";
    
    json += "}";

    // Skicka svaret som applikation/json
    request->send(200, "application/json", json);
}

/**
 * Startar webbserverns endpoints.
 */
void startWebServer() {
    if (isWebServerStarted) return;

    // Index-sida (enkel statusrapport)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "📊 VenusOS GUI v2 Klon - Servern är aktiv! Gå till /data för realtids-JSON.");
    });

    // JSON API-endpoint för din dator/mobil
    server.on("/data", HTTP_GET, handleJsonRequest);

    // Starta lyssnaren
    server.begin();
    isWebServerStarted = true;
    Serial.println("[Network] Asynkron webbserver har startat på port 80!");
}

/**
 * Initierar Wi-Fi-anslutningen asynkront.
 */
void initNetworkManager(String ssid, String pass) {
    if (ssid == "" || ssid == "DITT_WIFI_SSID") {
        Serial.println("[Network] Wi-Fi-konfiguration saknas (körs i offline-läge).");
        return;
    }
    
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.print("[Network] Ansluter till Wi-Fi: ");
    Serial.println(ssid);
}

/**
 * Skickar RPC-kommandon till Shelly Pro 1 & 2.
 */
void controlShellyProRelay(ShellyDevice &device, int channel, bool state) {
    if (!device.cfg.enabled || device.cfg.mac_or_ip == "" || device.cfg.mac_or_ip == "0.0.0.0") return;
    if (channel >= device.total_channels) return;

    String url = "http://" + device.cfg.mac_or_ip + "/rpc/Switch.Set";
    
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(1500);

    String jsonPayload = "{\"id\":" + String(channel) + ",\"on\":" + (state ? "true" : "false") + "}";
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode == 200) {
        device.channel_states[channel] = state;
        Serial.printf("[Shelly] RPC OK: %s (Ch%d -> %s)\n", device.cfg.mac_or_ip.c_str(), channel, state ? "ON" : "OFF");
    } else {
        Serial.printf("[Shelly] RPC Fel mot %s: %d\n", device.cfg.mac_or_ip.c_str(), httpResponseCode);
    }
    http.end();
}

/**
 * Håller koll på Wi-Fi-statusen och aktiverar servern vid lyckad anslutning.
 */
void updateNetworkData() {
    // Om Wi-Fi precis anslöt, hämta IP och starta webbservern automatiskt
    if (WiFi.status() == WL_CONNECTED && !isWebServerStarted) {
        Serial.print("[Network] Wi-Fi Anslutet! IP-adress: ");
        Serial.println(WiFi.localIP());
        
        // Starta webbservern nu när vi har ett nätverk
        startWebServer();
    }
}

#endif // NETWORK_MANAGER_H
