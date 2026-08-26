#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "storage_manager.h"

lv_obj_t * main_keyboard = nullptr;

static void ta_event_handler(lv_event_t * e) {
    lv_obj_t * ta = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (main_keyboard == nullptr) {
            main_keyboard = lv_keyboard_create(lv_scr_act());
            lv_obj_set_size(main_keyboard, LV_PCT(100), LV_PCT(45));
            lv_obj_align(main_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        lv_keyboard_set_textarea(main_keyboard, ta);
    }
    
    if (code == LV_EVENT_READY) {
        DeviceConfig *cfg = (DeviceConfig*)lv_event_get_user_data(e);
        cfg->name = lv_textarea_get_text(ta);
        saveAllSettings();
        if (main_keyboard != nullptr) {
            lv_obj_del(main_keyboard);
            main_keyboard = nullptr;
        }
    }
}

static void switch_event_handler(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    DeviceConfig *cfg = (DeviceConfig*)lv_event_get_user_data(e);
    cfg->enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    saveAllSettings();
}

void add_device_to_settings_list(lv_obj_t * list, DeviceConfig &cfg, const char* labelText) {
    lv_list_add_text(list, labelText);
    lv_obj_t * row = lv_obj_create(list);
    lv_obj_set_size(row, LV_PCT(100), 50);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * sw = lv_switch_create(row);
    lv_obj_align(sw, LV_ALIGN_LEFT_MID, 5, 0);
    if (cfg.enabled) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_VALUE_CHANGED, &cfg);

    lv_obj_t * ta = lv_textarea_create(row);
    lv_obj_set_size(ta, 180, 40);
    lv_obj_align(ta, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, cfg.name.c_str());
    lv_obj_add_event_cb(ta, ta_event_handler, LV_EVENT_ALL, &cfg);
}

void create_settings_page(lv_obj_t * parent) {
    lv_obj_t * list = lv_list_create(parent);
    lv_obj_set_size(list, 440, 380);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);

    add_device_to_settings_list(list, shunt.cfg, "Victron SmartShunt");
    add_device_to_settings_list(list, mppt.cfg, "Victron MPPT Solceller");
    add_device_to_settings_list(list, ip22.cfg, "Victron IP22 Laddare");
    add_device_to_settings_list(list, ruuvi.cfg, "RuuviTag Sensor");
    add_device_to_settings_list(list, mijia.cfg, "Xiaomi Mijia Sensor");
    add_device_to_settings_list(list, shellyPro1.cfg, "Shelly Pro 1 (1-Kanal)");
    add_device_to_settings_list(list, shellyPro2.cfg, "Shelly Pro 2 (2-Kanal)");
}

void initDisplayAndUI() {
    // Din Waveshare-drivrutinsinitiering för LCD/Touch
}

#endif // UI_MANAGER_H
