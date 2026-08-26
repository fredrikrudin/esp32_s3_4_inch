#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

// HTML-sida inbakad i koden som ett gränssnitt för att styra scheman direkt via IP-adressen (/)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <title>VenusOS GUI v2 - Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
</head>
<body style="background: #1a1a1a; color: #fff; font-family: sans-serif; padding: 20px; display: flex; justify-content: center;">
    <div class="card" style="padding: 20px; border-radius: 6px; background: #2b2b2b; max-width: 450px; width: 100%; box-shadow: 0 4px 6px rgba(0,0,0,0.3);">
        <h3 style="margin-top:0; border-bottom: 1px solid #444; padding-bottom: 8px; color: #f39c12;">📆 VenusOS Reläskedulering</h3>
        <form id="relayScheduleForm" onsubmit="sendRelaySchedule(event)">
            <div style="margin-bottom: 12px;">
                <label>Välj reläkort: </label>
                <select id="board_type" onchange="updateChannelOptions()" style="background:#444; color:#fff; border:1px solid #666; padding:5px; border-radius:4px;">
                    <option value="4ch">4-Kanalskort (Elegoo)</option>
                    <option value="8ch">8-Kanalskort (Elegoo)</option>
                </select>
                <label style="margin-left: 10px;">Kanal: </label>
                <select id="relay_channel" style="background:#444; color:#fff; border:1px solid #666; padding:5px; border-radius:4px;"></select>
            </div>
            <div style="margin-bottom: 12px;">
                <label><input type="checkbox" id="sched_enabled" checked> Aktivera detta schema</label>
            </div>
            <div style="margin-bottom: 12px;">
                <label>Starttid: </label>
                <input type="time" id="sched_start" value="08:00" style="background:#444; color:#fff; border:1px solid #666; padding:4px; border-radius:4px;">
                <label style="margin-left: 10px;">Stopptid: </label>
                <input type="time" id="sched_end" value="17:00" style="background:#444; color:#fff; border:1px solid #666; padding:4px; border-radius:4px;">
            </div>
            <div style="margin-bottom: 15px; line-height: 24px; background: #333; padding: 10px; border-radius: 4px;">
                <label style="font-weight: bold; color: #27ae60;">Aktiva veckodagar:</label><br>
                <input type="checkbox" id="day0" checked> Mån 
                <input type="checkbox" id="day1" checked> Tis 
                <input type="checkbox" id="day2" checked> Ons 
                <input type="checkbox" id="day3" checked> Tors <br>
                <input type="checkbox" id="day4" checked> Fre 
                <input type="checkbox" id="day5"> Lör 
                <input type="checkbox" id="day6"> Sön 
            </div>
            <button type="submit" style="background: #27ae60; color: #fff; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; width: 100%; font-weight: bold; font-size: 15px;">
                Spara Schema i Systemet
            </button>
        </form>
    </div>
    <script>
    function updateChannelOptions() {
        const type = document.getElementById('board_type').value;
        const channelSelect = document.getElementById('relay_channel');
        channelSelect.innerHTML = '';
        const maxChannels = (type === '8ch') ? 8 : 4;
        for(let i=0; i<maxChannels; i++) {
            let opt = document.createElement('option');
            opt.value = i; opt.innerText = 'Relä ' + (i+1);
            channelSelect.appendChild(opt);
        }
    }
    updateChannelOptions();
    function sendRelaySchedule(e) {
        e.preventDefault();
        const startTime = document.getElementById('sched_start').value.split(':');
        const endTime = document.getElementById('sched_end').value.split(':');
        const payload = {
            is8ch: document.getElementById('board_type').value === '8ch',
            channel: parseInt(document.getElementById('relay_channel').value),
            enabled: document.getElementById('sched_enabled').checked,
            startH: parseInt(startTime[0]),
            startM: parseInt(startTime[1]),
            endH: parseInt(endTime[0]),
            endM: parseInt(endTime[1]),
            days: [
                document.getElementById('day0').checked, document.getElementById('day1').checked,
                document.getElementById('day2').checked, document.getElementById('day3').checked,
                document.getElementById('day4').checked, document.getElementById('day5').checked,
                document.getElementById('day6').checked
            ]
        };
        fetch('/api/schedule', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        })
        .then(res => res.json())
        .then(data => {
            if(data.status === "success") alert('Schemat sparades framgångsrikt i din ESP32!');
            else alert('Ett fel uppstod: ' + data.status);
        })
        .catch(err => alert('Nätverksfel vid kommunikation: ' + err));
    }
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGES_H
