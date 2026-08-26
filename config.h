#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Universell enhetskonfiguration (Används för inställningssidan)
struct DeviceConfig {
    String mac_or_ip;
    String name;
    bool enabled;
};

// Generisk Victron-struktur som täcker alla enhetstyper (SmartShunt, MPPT, IP22 mm)
struct VictronDevice {
    DeviceConfig cfg;
    String key;         // AES-krypteringsnyckel för Instant Readout
    float voltage;
    float current;
    float soc;
    float power;
    int deviceType;     // Identifierare för ikon/enhetstyp
};

struct RuuviTagDevice {
    DeviceConfig cfg;
    float temperature;
    float humidity;
};

struct XiaomiMijiaDevice {
    DeviceConfig cfg;
    float temperature;
    float humidity;
    float battery_level;
};

// Shelly-struktur uppdaterad för Pro 1 (1 kanal) och Pro 2 (2 kanaler)
struct ShellyDevice {
    DeviceConfig cfg;   // cfg.mac_or_ip sparar enhetens lokala IP-adress
    int total_channels; // 1 för Pro 1, 2 för Pro 2
    bool channel_states[2]; // Tillstånd för respektive reläkanal
};

// Externa deklarationer av dina valbara enheter
extern VictronDevice shunt, mppt, ip22;
extern RuuviTagDevice ruuvi;
extern XiaomiMijiaDevice mijia;
extern ShellyDevice shellyPro1, shellyPro2;

// Externa I2C-inställningar för det fysiska reläkortet
const uint8_t RELAY_I2C_ADDRESS = 0x20;
const int I2C_SDA_PIN = 15;
const int I2C_SCL_PIN = 7;
const int RELAY_8CH_PINS[] = {0, 1, 2, 3, 4, 5, 6, 7};

extern int display_brightness;

#endif // CONFIG_H
