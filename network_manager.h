#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include "config.h"

extern Preferences prefs;

void setShellyState(bool turn_on) {
    if (!shelly.enabled || WiFi.status() != WL_CONNECTED || shelly.ip == "") return;
    HTTPClient http;
    http.begin("http://" + shelly.ip + "/rpc/Switch.Set?id=0&on=" + (turn_on ? "true" : "false"));
    if (http.GET() > 0) shelly.current_status = turn_on;
    http.end();
}

void setElegooRelayState(bool is_8ch, int ch, bool turn_on) {
    if (is_8ch) {
        if (!elegoo.enabled_8ch || ch < 0 || ch >= 8) return;
        elegoo.relay8_states[ch] = turn_on; digitalWrite(RELAY_8CH_PINS[ch], turn_on ? LOW : HIGH);
    } else {
        if (!elegoo.enabled_4ch || ch < 0 || ch >= 4) return;
        elegoo.relay4_states[ch] = turn_on; digitalWrite(RELAY_4CH_PINS[ch], turn_on ? LOW : HIGH);
    }
}

void processShellySchedule() {
    if (!shelly.enabled || !shelly.schedule_active) return;
    time_t now; struct tm ti; if (!getLocalTime(&ti)) return;
    if (ti.tm_hour == shelly.on_hour && ti.tm_min == shelly.on_minute && !shelly.current_status) setShellyState(true);
    if (ti.tm_hour == shelly.off_hour && ti.tm_min == shelly.off_minute && shelly.current_status) setShellyState(false);
}

void init_elegoo_relays() {
    for (int i=0; i<4; i++) { pinMode(RELAY_4CH_PINS[i], OUTPUT); digitalWrite(RELAY_4CH_PINS[i], HIGH); }
    for (int i=0; i<8; i++) { pinMode(RELAY_8CH_PINS[i], OUTPUT); digitalWrite(RELAY_8CH_PINS[i], HIGH); }
}

void processElegooSchedules() {
    time_t now; struct tm ti; if (!getLocalTime(&ti)) return;
    for(int i=0; i<4; i++) {
        if (!elegoo.schedule4[i].schedule_active) continue;
        if (ti.tm_hour == elegoo.schedule4[i].on_hour && !elegoo.relay4_states[i]) setElegooRelayState(false, i, true);
        if (ti.tm_hour == elegoo.schedule4[i].off_hour && elegoo.relay4_states[i]) setElegooRelayState(false, i, false);
    }
}

const char relays_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>VenusOS v2 - Relays</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
    body { font-family: Arial, sans-serif; background-color: #0B0C0E; color: #fff; margin: 0; }
    .status-bar { background: #000; padding: 12px; display: flex; align-items: center; color: #8A92A6; font-weight: bold; }
    .container { padding: 20px; display: flex; flex-direction: column; align-items: center; }
    .capsule { background: #181A1F; border: 1px solid #282C34; border-radius: 18px; padding: 15px; width: 100%; max-width: 420px; margin: 8px 0; }
    .row { display: flex; justify-content: space-between; align-items: center; }
    .switch { position: relative; display: inline-block; width: 46px; height: 22px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #353b45; transition: .3s; border-radius: 22px; }
    .slider:before { position: absolute; content: ""; height: 16px; width: 16px; left: 3px; bottom: 3px; background-color: white; transition: .3s; border-radius: 50%; }
    input:checked + .slider { background-color: #2196F3; }
    input:checked + .slider:before { transform: translateX(24px); }
</style>
</head><body>
<div class="status-bar"><div>🎛️ Webbkontroll Panel (GUI v2)</div></div>
<div class="container" id="relay-zone"></div>
<script>
    function remoteToggle(type, ch, state) { fetch(`/control_relay?type=${type}&ch=${ch}&state=${state ? 1 : 0}`); }
    function syncEngine() {
        fetch('/relay_status_json').then(res => res.json()).then(data => {
            let zone = document.getElementById("relay-zone");
            let html = `<div class='capsule'><div class='row'><span>Shelly Plus 1 (Wi-Fi)</span><label class='switch'><input type='checkbox' ${data.shl_on ? 'checked' : ''} onchange='remoteToggle("shelly", 99, this.checked)'><span class='slider'></span></label></div></div>`;
            for(let i=0; i<4; i++) {
                html += `<div class='capsule'><div class='row'><span>Elegoo Utgang ${i+1}</span><label class='switch'><input type='checkbox' ${data.r4[i] ? 'checked' : ''} onchange='remoteToggle("4ch", ${i}, this.checked)'><span class='slider'></span></label></div></div>`;
            }
            zone.innerHTML = html;
        });
    }
    setInterval(syncEngine, 1000); window.onload = syncEngine;
</script></body></html>
)rawliteral";

void register_relay_web_routes(AsyncWebServer &server) {
    server.on("/relays", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_200(relays_html, "text/html"); });
    server.on("/relay_status_json", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(2048);
        doc["shl_on"] = shelly.current_status; doc["shl_sch"] = shelly.schedule_active;
        JsonArray r4_arr = doc.createNestedArray("r4");
        for(int i=0; i<4; i++) { r4_arr.add(elegoo.relay4_states[i]); }
        String out; serializeJson(doc, out); request->send(200, "application/json", out);
    });
    server.on("/control_relay", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("type") && request->hasParam("ch") && request->hasParam("state")) {
            String type = request->getParam("type")->value(); int ch = request->getParam("ch")->value().toInt();
            bool state = request->getParam("state")->value().toInt() == 1;
            if (type == "shelly") setShellyState(state); if (type == "4ch") setElegooRelayState(false, ch, state);
        }
        request->send(200, "text/plain", "OK");
    });
}
#endif
