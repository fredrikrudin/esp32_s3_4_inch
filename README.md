# 📊 VenusOS GUI v2 Klon för Waveshare ESP32-S3

Ett modernt GUI v2-styrsystem för **Waveshare ESP32-S3-Touch-LCD-4**. Systemet hanterar trådlös data (BLE/Victron) och styrsystem via en ny, asynkron och stabil FreeRTOS-arkitektur.

## ✨ Funktioner
* **GUI v2:** Högpresterande LVGL (v8.3) gränssnitt som emulerar Victron Venus OS v2.
* **Kärnisolerad skedulering:** Grafik och touch körs dedikerat i ~200Hz på Core 1, medan radiotrafik och reläautomation hanteras på Core 0.
* **Trådsäkerhet:** Mutex-skydd (`lvgl_mutex`) förhindrar krascher vid asynkrona datauppdateringar från BLE eller Wi-Fi.
* **Dubbla I2C-bussar:** Separerad intern buss (skärm/touch i 400kHz) och extern buss (reläer i 100kHz) för att eliminera störningar i grafiken.
* **Data:** Passiv BLE-avläsning med AES-dekryptering (Victron SmartShunt, MPPT, IP22, Ruuvi, Xiaomi).
* **Styrning & Automation:** Tids- och datumstyrd reläautomation synkad mot NTP-klocka samt trådlös RPC-styrning av Shelly Pro-enheter.
* **v4-stöd:** Automatisk hantering av både äldre kort och nya v4-revisioner (TCA9554).
* **Asynkront Webbgränssnitt:** HTML-inställningssida med JSON-synk och REST API för smidig tidsskedulering.

---

## 🔌 Fysisk I2C-inkoppling (PCF8574)
Använd 3.5mm-terminalen på baksidan för att styra externa reläer. Detta körs på en helt egen hårdvaruinstans (`ExternalI2C`) för att frigöra och skydda skärmens RGB-pinnar.

| Waveshare | PCF8574 Relä | Beskrivning |
| :--- | :--- | :--- |
| **GND** | **GND** | Gemensam jord |
| **5V/VCC** | **VCC** | Strömförsörjning |
| **SDA** (GPIO 15) | **SDA** | Extern I2C Data (Buss 1) |
| **SCL** (GPIO 7) | **SCL** | Extern I2C Klocka (Buss 1) |

*Standardadress: `0x20` (A0-A2 på GND). Körs stabilt i 100kHz.*
*Interna komponenter (Touch/TCA9554) använder GPIO 8 (SDA) och GPIO 9 (SCL) i 400kHz (Buss 0).*

---

## 🌐 Webbgränssnitt & Reläskedulering
Systemet startar en asynkron webbserver på port 80 så fort Wi-Fi är anslutet.
* **`/` (Rotkatalogen):** Öppnar det inbakade HTML-formuläret i din webbläsare där du grafiskt kan ställa in starttid, stopptid och aktiva veckodagar för dina 4- eller 8-kanals Elegoo-reläkort.
* **`/data`:** Levererar live-JSON med systemdiagnostik, aktuella mätvärden från SmartShunt/MPPT samt relästatus.

---

## 📋 Biblioteksberoenden
Säkerställ att följande bibliotek är installerade innan du kompilerar:
1. **LVGL (v8.3.x)** – För det grafiska gränssnittet.
2. **ArduinoJson (v6.x eller v7.x)** – Krävs för hantering och parsningslogik av tidur i nätverksmodulen.
3. **ESP32_IO_Expander** – Krävs för bakgrundsbelysning på Rev v4-kort.

---

## 🚀 Kompileringsinställningar
**Viktigt i Arduino IDE eller PlatformIO för att undvika "bricked" port:**
* **Flash Size:** 16MB
* **Partition Scheme:** Huge APP (3MB No OTA/1MB SPIFFS eller motsvarande stort schema)
* **PSRAM:** OPI PSRAM
* **USB CDC On Boot:** **ENABLED** 🟢 (Kritiskt för att bibehålla seriell kommunikation)

---

## 🛠 Felsökning
Om skärmen är svart eller USB-porten inte svarar:
1. Håll in **BOOT**-knappen på baksidan, tryck snabbt på **RST**, och släpp sedan **BOOT** för att tvinga in kretsen i bootloader-läge.

## Waveshare wiki för hårdvaran
https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4?srsltid=AfmBOoqFlqT4-1Bk7TU7iULHEJSVhwUwnNRhCOGrgfJHa1z8DEn0Dcy5
2. Kontrollera inkluderingsordningen i `VenusOS_ESP32.ino`: `storage_manager.h` **måste** ligga ovanför `network_manager.h` för att alla länkningar ska fungera vid kompilering.
