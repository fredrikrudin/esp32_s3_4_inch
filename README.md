# 📊 VenusOS GUI v2 Klon för Waveshare ESP32-S3

Ett modulärt och högpresterande styrsystem i **Victron GUI v2-stil** utvecklat för **Waveshare ESP32-S3-Touch-LCD-4**. Systemet samlar in data trådlöst och styr reläer lokalt samt via Wi-Fi.

## ✨ Funktioner
- **Mörkt GUI v2 Tema:** Minimalistiskt och modernt användargränssnitt byggt i LVGL (v8.3).
- **Trådlös Data:** Passiv BLE-avläsning av Victron SmartShunt (AES-krypterad), RuuviTag och Xiaomi Mijia sensorer.
- **Dubbel Reläkontroll:** Realtidsstyrning och schemaläggning av trådlösa Shelly Plus 1 samt lokala fysiska Elegoo (4/8-kanaliga) reläkort.
- **Asynkront Webbgränssnitt:** Inbyggd webbserver med realtids JSON-synk mot din mobil/dator.
- **Smarta Gester:** Dra vertikalt på touchskärmen för att tona in ett dolt ljusstyrkoreglage (PWM).

## 🖥️ Gränssnittsarkitektur (Bilder & Design)
Nedan visas exempel på hur färgvalen och kapsel-layouterna i gränssnittet är strukturerade:

| Dashboard (Energiflöde) | Enhetskontroll (v2) | Statusrad (NTP) |
|---|---|---|
| ![Dashboard Layout](https://unsplash.com) | ![Kapsel Design](https://unsplash.com) | ![Statusrad](https://unsplash.com) |

## 🚀 Kompileringsinställningar i Arduino IDE
För att koden ska få plats i chippet, konfigurera **Tools**-menyn enligt följande:
- **Board:** `ESP32S3 Dev Module`
- **Flash Size:** `16MB (128Mb)`
- **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)`
- **PSRAM:** `OPI PSRAM`

## 👥 Bidra & Open-Source
Detta projekt är helt Open-Source. Skapa gärna en *Pull Request* eller öppna en *Issue* om du vill lägga till stöd för fler hårdvarukällor!
