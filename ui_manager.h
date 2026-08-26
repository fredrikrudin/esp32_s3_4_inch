#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "storage_manager.h"

extern void triggerManualDiscovery(const char* typeFilter); 
extern void create_overview_page(lv_obj_t *parent); 
extern void create_classic_overview_page(lv_obj_t *parent); 
extern void create_environment_page(lv_obj_t *parent);

extern lv_obj_t *main_keyboard;
extern lv_obj_t *lbl_footer_clock;
extern lv_obj_t *btn_hamburger;
extern lv_obj_t *page_settings_container;
extern lv_obj_t *page_overview_container;
extern lv_obj_t *page_environment_container;
extern lv_obj_t *discovery_popup;
extern lv_obj_t *discovery_list;
extern DeviceConfig *active_discovery_target;

struct UiEventContext { DeviceConfig *cfg; VictronDevice *victronDev; bool isKeyField; };
struct ScanButtonContext { DeviceConfig *cfg; const char* typeFilter; };

static void ta_event_handler(lv_event_t * e) {
    lv_obj_t * ta = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    UiEventContext *ctx = (UiEventContext*)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        if (main_keyboard == nullptr) {
            main_keyboard = lv_keyboard_create(lv_scr_act());
            lv_obj_set_size(main_keyboard, LV_PCT(100), LV_PCT(40)); 
            lv_obj_align(main_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        lv_keyboard_set_textarea(main_keyboard, ta);
        lv_keyboard_set_mode(main_keyboard, ctx->isKeyField ? LV_KEYBOARD_MODE_NUMBER : LV_KEYBOARD_MODE_TEXT_LOWER);
    }
    if (code == LV_EVENT_READY) {
        if (ctx->isKeyField && ctx->victronDev != nullptr) ctx->victronDev->key = lv_textarea_get_text(ta);
        else if (ctx->cfg != nullptr) ctx->cfg->name = lv_textarea_get_text(ta);
        saveAllSettings(); 
        if (main_keyboard != nullptr) { lv_obj_del(main_keyboard); main_keyboard = nullptr; }
    }
    if (code == LV_EVENT_DELETE) { if (ctx != nullptr) delete ctx; }
}

static void switch_event_handler(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    DeviceConfig *cfg = (DeviceConfig*)lv_event_get_user_data(e);
    if (code == LV_EVENT_VALUE_CHANGED) { cfg->enabled = lv_obj_has_state(sw, LV_STATE_CHECKED); saveAllSettings(); }
}

static void discovery_select_event_handler(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    const char * selected_addr = lv_list_get_btn_text(discovery_list, btn);
    if (active_discovery_target != nullptr && selected_addr != nullptr) {
        String addr_str = String(selected_addr);
        int start_idx = addr_str.indexOf('['); int end_idx = addr_str.indexOf(']');
        if (start_idx != -1 && end_idx != -1) active_discovery_target->mac_or_ip = addr_str.substring(start_idx + 1, end_idx);
        else active_discovery_target->mac_or_ip = addr_str;
        saveAllSettings();
    }
    if (discovery_popup != nullptr) { lv_obj_del(discovery_popup); discovery_popup = nullptr; discovery_list = nullptr; active_discovery_target = nullptr; }
}

static void close_popup_event_handler(lv_event_t * e) {
    if (discovery_popup != nullptr) { lv_obj_del(discovery_popup); discovery_popup = nullptr; discovery_list = nullptr; active_discovery_target = nullptr; }
}

static void ui_style_toggle_event_handler(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_style_version = (ui_style_version == 2) ? 1 : 2;
        lv_label_set_text(lv_obj_get_child(lv_event_get_target(e), 0), ui_style_version == 2 ? "Gränssnitt: Cirkulär v2" : "Gränssnitt: Klassisk v1");
        saveScheduleToNVS(); 
    }
}

static void ui_brightness_event_handler(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if (display_brightness < 64)       display_brightness = 128;
        else if (display_brightness < 128)  display_brightness = 192;
        else if (display_brightness < 192)  display_brightness = 255;
        else                                display_brightness = 64;
        extern void setDisplayBrightness(int brightness);
        setDisplayBrightness(display_brightness);
        lv_label_set_text_fmt(lv_obj_get_child(lv_event_get_target(e), 0), "Ljusstyrka: %d%%", (display_brightness * 100) / 255);
        saveScheduleToNVS();
    }
}

static void ui_manual_relay_event_handler(lv_event_t * e) {
    uint32_t relayPin = (uint32_t)lv_event_get_user_data(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        extern void setExternalRelay(uint8_t relayPin, bool state);
        setExternalRelay((uint8_t)relayPin, lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
    }
}

static void hamburger_click_event_handler(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if (lv_obj_has_flag(page_settings_container, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(page_overview_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(page_settings_container, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(lv_obj_get_child(btn_hamburger, 0), LV_SYMBOL_HOME); 
        } else {
            lv_obj_add_flag(page_settings_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clean(page_overview_container); 
            if (ui_style_version == 1) create_classic_overview_page(page_overview_container);
            else create_overview_page(page_overview_container);
            lv_obj_clear_flag(page_overview_container, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(lv_obj_get_child(btn_hamburger, 0), LV_SYMBOL_LIST); 
        }
    }
}

void add_device_to_settings_list(lv_obj_t * list, DeviceConfig &cfg, const char* labelText, const char* typeFilter, VictronDevice *victronDev = nullptr) {
    lv_list_add_text(list, labelText);
    int row_height = (victronDev != nullptr) ? 140 : 100;
    lv_obj_t * row = lv_obj_create(list); lv_obj_set_size(row, LV_PCT(100), row_height);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * sw = lv_switch_create(row); lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 5, 5);
    if (cfg.enabled) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_ALL, &cfg);

    lv_obj_t * ta_name = lv_textarea_create(row); lv_obj_set_size(ta_name, 150, 40); lv_obj_align(ta_name, LV_ALIGN_TOP_RIGHT, -5, 0);
    lv_textarea_set_one_line(ta_name, true); lv_textarea_set_text(ta_name, cfg.name.c_str());
    UiEventContext *nameCtx = new UiEventContext { &cfg, victronDev, false };
    lv_obj_add_event_cb(ta_name, ta_event_handler, LV_EVENT_ALL, nameCtx);

    lv_obj_t * scan_btn = lv_btn_create(row); lv_obj_set_size(scan_btn, 110, 35); lv_obj_align(scan_btn, LV_ALIGN_TOP_LEFT, 5, 45);
    lv_obj_set_style_bg_color(scan_btn, lv_color_make(0, 120, 215), 0);
    lv_obj_t * scan_lbl = lv_label_create(scan_btn); lv_label_set_text(scan_lbl, "Sök enhet"); lv_obj_center(scan_lbl);
    ScanButtonContext *scanCtx = new ScanButtonContext { &cfg, typeFilter };
    lv_obj_add_event_cb(scan_btn, scan_trigger_callback, LV_EVENT_ALL, scanCtx);

    lv_obj_t * lbl_addr = lv_label_create(row); lv_obj_align(lbl_addr, LV_ALIGN_TOP_RIGHT, -5, 50);
    lv_label_set_text_fmt(lbl_addr, "ID: %s", cfg.mac_or_ip.length() > 0 ? cfg.mac_or_ip.c_str() : "Saknas");

    if (victronDev != nullptr) {
        lv_obj_t * ta_key = lv_textarea_create(row); lv_obj_set_size(ta_key, 330, 40); lv_obj_align(ta_key, LV_ALIGN_BOTTOM_MID, 0, -5);
        lv_textarea_set_one_line(ta_key, true); lv_textarea_set_text(ta_key, victronDev->key.c_str());
        UiEventContext *keyCtx = new UiEventContext { &cfg, victronDev, true };
        lv_obj_add_event_cb(ta_key, ta_event_handler, LV_EVENT_ALL, keyCtx);
    }
}

inline void add_ui_style_selector_to_settings(lv_obj_t * list) {
    lv_list_add_text(list, "Systemkonfiguration");
    lv_obj_t * row = lv_obj_create(list); lv_obj_set_size(row, LV_PCT(100), 120);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * toggle_btn = lv_btn_create(row); lv_obj_set_size(toggle_btn, 160, 40); lv_obj_align(toggle_btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(toggle_btn, lv_color_make(0, 120, 215), 0);
    lv_obj_t * btn_lbl = lv_label_create(toggle_btn); lv_label_set_text(btn_lbl, ui_style_version == 2 ? "Gränssnitt: Cirkulär" : "Gränssnitt: Klassisk");
    lv_obj_center(btn_lbl); lv_obj_add_event_cb(toggle_btn, ui_style_toggle_event_handler, LV_EVENT_CLICKED, nullptr);

    lv_obj_t * bright_btn = lv_btn_create(row); lv_obj_set_size(bright_btn, 160, 40); lv_obj_align(bright_btn, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_bg_color(bright_btn, lv_color_make(243, 156, 18), 0);
    lv_obj_t * b_lbl = lv_label_create(bright_btn); lv_label_set_text_fmt(b_lbl, "Ljusstyrka: %d%%", (display_brightness * 100) / 255);
    lv_obj_center(b_lbl); lv_obj_add_event_cb(bright_btn, ui_brightness_event_handler, LV_EVENT_CLICKED, nullptr);
}

inline void add_manual_relay_to_settings(lv_obj_t * list, uint8_t relayPin, const char* relayName) {
    lv_list_add_text(list, "Manuell Reläkontakt");
    lv_obj_t * row = lv_obj_create(list); lv_obj_set_size(row, LV_PCT(100), 60);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * lbl = lv_label_create(row); lv_label_set_text_fmt(lbl, "R%d: %s", relayPin+1, relayName); lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_t * sw = lv_switch_create(row); lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_color(sw, lv_color_make(39, 174, 96), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, ui_manual_relay_event_handler, LV_EVENT_VALUE_CHANGED, (void*)(uint32_t)relayPin);
}

#endif // UI_MANAGER_H
