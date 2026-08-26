#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include <time.h> 

// --- I2C Busskonfiguration ---
// Intern buss: Waveshare Touch, Skärm och Bakgrundsbelysning (TCA9554)
#define INTERNAL_SDA 8
#define INTERNAL_SCL 9
#define INTERNAL_I2C_FREQ 400000

// Extern buss: Reläterminal på baksidan (PCF8574)
#define EXTERNAL_SDA 15
#define EXTERNAL_SCL 7
#define EXTERNAL_I2C_FREQ 100000 
const uint8_t RELAY_I2C_ADDRESS = 0x20;

const int RELAY_4CH_PINS[] = {0, 1, 2, 3};
const int RELAY_8CH_PINS[] = {0, 1, 2, 3, 4, 5, 6, 7};

// --- Datastrukturer för enheter & sensorer ---
struct DeviceConfig {
    String mac_or_ip;  
    String name;       
    bool enabled;      
};

struct VictronDevice {
    DeviceConfig cfg;
    String key;         
    float voltage;
    float current;
    float soc;
    float power;
    int deviceType;     
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

struct ShellyDevice {
    DeviceConfig cfg;
    int total_channels;    
    bool channel_states[2]; 
};

// Uppgraderad flexibel struktur för Tid- och Datumschemaläggning
struct RelaySchedule {
    int startHour;      // 0-23
    int startMinute;    // 0-59
    int endHour;        // 0-23
    int endMinute;      // 0-59
    bool days[7];       // [0]=Mån, [1]=Tis... [6]=Sön
    bool isEnabled;     
};

struct ElegooRelaySystem {
    bool relay4_states[4]; 
    bool relay8_states[8];
    bool enabled_4ch; 
    bool enabled_8ch;
    RelaySchedule schedule4[4]; 
    RelaySchedule schedule8[8];
};

// Globala instanser och systemvariabler
extern VictronDevice shunt, mppt, ip22;
extern RuuviTagDevice ruuvi;
extern XiaomiMijiaDevice mijia;
extern ShellyDevice shellyPro1, shellyPro2;
extern ElegooRelaySystem elegoo;

extern String wifi_ssid, wifi_pass;
extern int update_interval;
extern int display_brightness;

// FreeRTOS Delade resurser & Bussar
extern SemaphoreHandle_t lvgl_mutex;
extern TwoWire InternalI2C;
extern TwoWire ExternalI2C;

// Globala LVGL-objekt
extern lv_obj_t * main_keyboard;
extern lv_obj_t * lbl_footer_clock;
extern lv_obj_t * btn_hamburger;
extern lv_obj_t * page_settings_container; 
extern lv_obj_t * page_overview_container; 

#endif // CONFIG_H
