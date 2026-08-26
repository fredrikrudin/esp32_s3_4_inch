# 📊 VenusOS GUI v2 Klon för Waveshare ESP32-S3

Ett modernt GUI v2-styrsystem för **Waveshare ESP32-S3-Touch-LCD-4**. Systemet hanterar trådlös data (BLE/Victron) och styrsystem via en ny, stabilare arkitektur.

## ✨ Funktioner
* **GUI v2:** LVGL (v8.3) gränssnitt.
* **Data:** BLE-avläsning (SmartShunt, Ruuvi, Xiaomi).
* **Styrning:** Lokal I2C-relä och trådlösa Shelly Plus 1.
* **v4-stöd:** Automatisk hantering av både äldre kort och nya v4-revisioner (TCA9554).
* **Asynkront Webbgränssnitt:** JSON-synk.

---

## 🔌 Fysisk I2C-inkoppling (PCF8574)
Använd 3.5mm terminalen på baksidan för att styra reläer, vilket frigör RGB-pinnar.

| Waveshare | PCF8574 Relä |
| :--- | :--- |
| **GND** | **GND** |
| **5V/VCC** | **VCC** |
| **SDA** (GPIO 15) | **SDA** |
| **SCL** (GPIO 7) | **SCL** |

*Standardadress: `0x20` (A0-A2 på GND).*

---

## 🚀 Kompileringsinställningar
**Viktigt i Arduino IDE för att undvika "bricked" port:**
* **Flash Size:** 16MB
* **Partition Scheme:** Huge APP
* **PSRAM:** OPI PSRAM
* **USB CDC On Boot:** **ENABLED** 🟢

---

## 🛠 Felsökning
Om skärmen är svart eller USB-porten inte svarar:
1. Håll **BOOT**, tryck **RST**, släpp **BOOT** för att tvinga in bootloader-läge.
2. För Rev v4: Säkerställ att `ESP32_IO_Expander` är installerat för bakgrundsbelysning.
