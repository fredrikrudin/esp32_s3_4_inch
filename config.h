#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include <time.h> // För inbyggda klockan och tidszoner

// ==========================================
// 1. I2C- BUSS & HÅRDVARUKONFIGURATION
// ==========================================
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

// ==========================================
// 2. DATASTRUKTURER FÖR ENHETER & SENSORER
// ==========================================
struct DeviceConfig {
    String mac_or_ip;  // MAC-adress för BLE, IP-adress för Shelly
    String name;       // Anpassat namn valt av användaren
    bool enabled;      // True = enheten skannas/används, False = avstängd
};

struct VictronDevice {
    DeviceConfig cfg;
    String key;         // 32-teckens AES-krypteringsnyckel
    float voltage;
    float current;
    float soc;
    float power;
    int deviceType;     // 1 = Shunt, 2 = MPPT, 3 = IP22
};

struct VictronInverter {
    DeviceConfig cfg;
    String key;         // 32-teckens AES-krypteringsnyckel för växelriktaren
    float battery_voltage;
    float ac_voltage;   // Utgående AC-spänning (t.ex. 230V)
    float ac_current;   // Utgående AC-ström
    float ac_power;     // Utgående effekt i Watt (W)
    int state;          // Status: 0=Av, 3=Invertering, 4=Eco-läge, 5=Fel
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
    bool channel_states[2]; // Stöder upp till 2 kanaler
};

// Uppgraderad flexibel struktur för Tid- och Datumschemaläggning (Victron-stil)
struct RelaySchedule {
    int startHour;      // Starttimme (0-23)
    int startMinute;    // Startminut (0-59)
    int endHour;        // Stopptimme (0-23)
    int endMinute;      // Stoppminut (0-59)
    bool days[7];       // Måndag till Söndag. true = aktiv den dagen.
    bool isEnabled;     // Är detta schema aktiverat?
};

struct ElegooRelaySystem {
    bool relay4_states[4]; 
    bool relay8_states[8];
    bool enabled_4ch; 
    bool enabled_8ch;
    RelaySchedule schedule4[4]; // Direkt matchad mot dina kanaler
    RelaySchedule schedule8[8]; // Direkt matchad mot dina kanaler
};

// ==========================================
// 3. GLOBALA INSTANSER & SYSTEMVARIABLER
// ==========================================
extern VictronDevice shunt, mppt, ip22;
extern VictronInverter inverter;
extern RuuviTagDevice ruuvi;
extern XiaomiMijiaDevice mijia;
extern ShellyDevice shellyPro1, shellyPro2;
extern ElegooRelaySystem elegoo;

extern String wifi_ssid, wifi_pass;
extern int update_interval;
extern int display_brightness;
extern int ui_style_version; // 1 = Klassisk v1, 2 = Nya cirkulära GUI-v2

// ==========================================
// 4. FREERTOS DELADE RESURSER & BUSSAR
// ==========================================
extern SemaphoreHandle_t lvgl_mutex;
extern TwoWire InternalI2C;
extern TwoWire ExternalI2C;

// ==========================================
// 5. GLOBALA LVGL-OBJEKT
// ==========================================
extern lv_obj_t * main_keyboard;
extern lv_obj_t * lbl_footer_clock;
extern lv_obj_t * btn_hamburger;
extern lv_obj_t * page_settings_container; 
extern lv_obj_t * page_overview_container; 
extern lv_obj_t * page_environment_container;

extern lv_obj_t * lbl_inverter_pwr;

#endif // CONFIG_H
