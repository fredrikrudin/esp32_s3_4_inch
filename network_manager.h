#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h> 
#include <HTTPClient.h>
#include "config.h"

AsyncWebServer server(80);
bool isWebServerStarted = false;

void handleJsonRequest(AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"system\":{\"brightness\":" + String(display_brightness) + "},";
    json += "\"victron\":{";
    json += "\"shunt\":{\"name\":\"" + shunt.cfg.name + "\",\"v\":" + String(shunt.voltage) + ",\"a\":" + String(shunt.current) + ",\"soc\":" + String(shunt.soc) + ",\"p\":" + String(shunt.power) + ",\"en\":" + String(shunt.cfg.enabled) + "},";
    json += "\"mppt\":{\"name\":\"" + mppt.cfg.name + "\",\"v\":" + String(mppt.voltage) + ",\"a\":" + String(mppt.current) + ",\"soc\":" + String(mppt.soc) + ",\"p\":" + String(mppt.power) + ",\"en\":" + String(mppt.cfg.enabled) + "},";
    json += "\"ip22\":{\"name\":\"" + ip22.cfg.name + "\",\"v\":" + String(ip22.voltage) + ",\"a\":" + String(ip22.current) + ",\"soc\":" + String(ip22.soc) + ",\"p\":" + String(ip22.power) + ",\"en\":" + String(ip22.cfg.enabled) + "}";
    json += "},";
    json += "\"sensors\":{";
    json += "\"ruuvi\":{\"name\":\"" + ruuvi.cfg.name + "\",\"t\":" + String(ruuvi.temperature) + ",\"h\":" + String(ruuvi.humidity) + ",\"en\":" + String(ruuvi.cfg.enabled) + "},";
    json += "\"xiaomi\":{\"name\":\"" + mijia.cfg.name + "\",\"t\":" + String(mijia.temperature) + ",\"h\":" + String(mijia.humidity) + ",\"bat\":" + String(mijia.battery_level) + ",\"en\":" + String(mijia.cfg.enabled) + "}";
    json += "},";
    json += "\"shelly\":{";
    json += "\"pro1\":{\"name\":\"" + shellyPro1.cfg.name + "\",\"ip\":\"" + shellyPro1.cfg.mac_or_ip + "\",\"ch0\":" + String(shellyPro1.channel_states[0]) + ",\"en\":" + String(shellyPro1.cfg.enabled) + "},";
    json += "\"pro2\":{\"name\":\"" + shellyPro2.cfg.name + "\",\"ip\":\"" + shellyPro2.cfg.mac_or_ip + "\",\"ch0\":" + String(shellyPro2.channel_states[0]) + ",\"ch1\":" + String(shellyPro2.channel_states[1]) + ",\"en\":" + String(shellyPro2.cfg.enabled) + "}";
    json += "}";
    json += "}";

    request->send(200, "application/json", json);
}

void startWebServer() {
    if (isWebServerStarted) return;
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "📊 VenusOS GUI v2 Klon - Server Aktiv. Gå till /data");
    });
    server.on("/data", HTTP_GET, handleJsonRequest);
    server.begin();
    isWebServerStarted = true;
    Serial.println("[Network] Asynkron webbserver startad på port 80!");
}

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

void initNetworkManager(String ssid, String pass) {
    if (ssid == "" || ssid == "DITT_WIFI_SSID") {
        Serial.println("[Network] Wi-Fi saknas, kör offline.");
        return;
    }
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("[Network] Wi-Fi initierat.");
}

void updateNetworkData() {
    if (WiFi.status() == WL_CONNECTED && !isWebServerStarted) {
        Serial.print("[Network] Wi-Fi Anslutet! IP: ");
        Serial.println(WiFi.localIP());
        startWebServer();
    }
}
// Sätts i din AsyncWebServer-uppstart (t.ex. server.on)
server.on("/api/schedule", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    
    // Använd ArduinoJson för att packa upp data från webbläsaren
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (!error) {
        bool is8Channel = doc["is8ch"] | false;
        int ch = doc["channel"] | 0;

        if (is8Channel && ch >= 0 && ch < 8) {
            elegoo.enabled_8ch = true;
            elegoo.schedule8[ch].isEnabled = doc["enabled"];
            elegoo.schedule8[ch].startHour = doc["startH"];
            elegoo.schedule8[ch].startMinute = doc["startM"];
            elegoo.schedule8[ch].endHour = doc["endH"];
            elegoo.schedule8[ch].endMinute = doc["endM"];
            JsonArray daysArray = doc["days"];
            for(int i = 0; i < 7; i++) {
                elegoo.schedule8[ch].days[i] = daysArray[i];
            }
        } 
        else if (!is8Channel && ch >= 0 && ch < 4) {
            elegoo.enabled_4ch = true;
            elegoo.schedule4[ch].isEnabled = doc["enabled"];
            elegoo.schedule4[ch].startHour = doc["startH"];
            elegoo.schedule4[ch].startMinute = doc["startM"];
            elegoo.schedule4[ch].endHour = doc["endH"];
            elegoo.schedule4[ch].endMinute = doc["endM"];
            JsonArray daysArray = doc["days"];
            for(int i = 0; i < 7; i++) {
                elegoo.schedule4[ch].days[i] = daysArray[i];
            }
        }

        saveScheduleToNVS(); // Skriv direkt till flash-minnet via lagringsmodulen
        request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        request->send(400, "application/json", "{\"status\":\"json error\"}");
    }
});

#endif // NETWORK_MANAGER_H
