#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <lvgl.h>

struct VictronDevice { String mac; String key; bool enabled; float voltage; float current; float soc; float power; };
struct EcoWorthyDevice { String mac; bool enabled; float voltage; float current; float soc; float temp; };
struct RuuviTagDevice { String mac; bool enabled; float temperature; float humidity; };
struct XiaomiMijiaDevice { String mac; bool enabled; float temperature; float humidity; float battery_level; };
struct ShellyDevice { String ip; bool enabled; bool current_status; bool schedule_active; int on_hour; int on_minute; int off_hour; int off_minute; };

struct RelaySchedule { bool schedule_active; int on_hour; int off_hour; };
struct ElegooRelaySystem {
    bool relay4_states[4]; bool relay8_states[8];
    bool enabled_4ch; bool enabled_8ch;
    RelaySchedule schedule4[4]; RelaySchedule schedule8[8];
};

struct DiscoveredDevice { String mac; String type; int rssi; bool supported; };

// Temporär ändring för att verifiera att USB-porten överlever
const int RELAY_4CH_PINS[] = {99, 99, 99, 99}; 
const int RELAY_8CH_PINS[] = {99, 99, 99, 99, 99, 99, 99, 99};


extern VictronDevice shunt, mppt, ip22;
extern EcoWorthyDevice ecoBatt;
extern RuuviTagDevice ruuvi;
extern XiaomiMijiaDevice mijia;
extern ShellyDevice shelly;
extern ElegooRelaySystem elegoo;
extern DiscoveredDevice discoveryList[20];
extern int discoveredCount;

extern String wifi_ssid, wifi_pass;
extern int update_interval;
extern int display_brightness;

extern lv_obj_t * lbl_clock; extern lv_obj_t * lbl_wifi; extern lv_obj_t * lbl_status_temp;
extern lv_obj_t * menu_sidebar; extern lv_obj_t * main_keyboard;
extern lv_obj_t * page_dashboard; extern lv_obj_t * page_overview; extern lv_obj_t * page_shunt;
extern lv_obj_t * page_mppt; extern lv_obj_t * page_eco; extern lv_obj_t * page_shelly;
extern lv_obj_t * page_elegoo_4ch; extern lv_obj_t * page_elegoo_8ch;
extern lv_obj_t * page_ble_discovery; extern lv_obj_t * page_system_status; extern lv_obj_t * page_touch_cal;
extern lv_obj_t * list_ble_devices; extern lv_obj_t * lbl_sys_diag; extern lv_obj_t * table_device_status;
extern lv_obj_t * lbl_dc_value; extern lv_obj_t * cap_brightness; extern lv_obj_t * slider_brightness;
extern lv_timer_t * brightness_timer;

#endif
