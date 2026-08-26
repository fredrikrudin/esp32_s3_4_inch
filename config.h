#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <lvgl.h>

// Gemensam struktur för enhetshantering på inställningssidan
struct DeviceConfig {
    String mac_or_ip;  // MAC-adress för BLE, IP-adress för Shelly
    String name;       // Anpassat namn valt av användaren
    bool enabled;      // True = enheten skannas/används, False = avstängd
};

// Universell Victron-struktur för BLE Instant Readout
struct VictronDevice {
    DeviceConfig cfg;
    String key;         // 16-bytes AES-krypteringsnyckel för dekryptering
    float voltage;
    float current;
    float soc;
    float power;
    int deviceType;     // 1 = Shunt, 2 = MPPT, 3 = Laddare (IP22)
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

// Shelly-struktur anpassad för Gen 2/Pro RPC (stöder upp till 2 kanaler)
struct ShellyDevice {
    DeviceConfig cfg;
    int total_channels;    // 1 för Pro 1, 2 för Pro 2
    bool channel_states[2]; // Sparar status för respektive reläkanal
};

struct RelaySchedule { 
    bool schedule_active; 
    int on_hour; 
    int off_hour; 
};

struct ElegooRelaySystem {
    bool relay4_states[4]; 
    bool relay8_states[8];
    bool enabled_4ch; 
    bool enabled_8ch;
    RelaySchedule schedule4[4]; 
    RelaySchedule schedule8[8];
};

// Globala inställningar och enhetsinstanser
extern VictronDevice shunt, mppt, ip22;
extern RuuviTagDevice ruuvi;
extern XiaomiMijiaDevice mijia;
extern ShellyDevice shellyPro1, shellyPro2;
extern ElegooRelaySystem elegoo;

extern String wifi_ssid, wifi_pass;
extern int update_interval;
extern int display_brightness;

// Externa I2C-inställningar för det fysiska reläkortet (PCF8574)
const uint8_t RELAY_I2C_ADDRESS = 0x20;
const int I2C_SDA_PIN = 15;
const int I2C_SCL_PIN = 7;
const int RELAY_4CH_PINS[] = {0, 1, 2, 3};
const int RELAY_8CH_PINS[] = {0, 1, 2, 3, 4, 5, 6, 7};

// Globala LVGL-objekt för inställningssidan och tangentbord
extern lv_obj_t * main_keyboard;

#endif // CONFIG_H
