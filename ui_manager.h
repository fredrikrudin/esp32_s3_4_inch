#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "storage_manager.h"

// Extern funktion som finns i discovery_manager.h
extern void triggerManualDiscovery(const char* typeFilter); 

lv_obj_t * main_keyboard = nullptr;
lv_obj_t * discovery_popup = nullptr;
lv_obj_t * discovery_list = nullptr;
DeviceConfig *active_discovery_target = nullptr; // Håller reda på vilken rad som ska ha adressen

struct UiEventContext {
    DeviceConfig *cfg;
    VictronDevice *victronDev; 
    bool isKeyField;           
};

struct ScanButtonContext {
    DeviceConfig *cfg;
    const char* typeFilter;
};

// Callback när en enhet väljs i söklistan
static void discovery_select_event_handler(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    const char * selected_addr = lv_list_get_btn_text(discovery_list, btn);
    
    if (active_discovery_target != nullptr && selected_addr != nullptr) {
        String addr_str = String(selected_addr);
        int start_idx = addr_str.indexOf('[');
        int end_idx = addr_str.indexOf(']');
        
        // Extrahera adressen inuti klamrarna "Enhetsnamn [AA:BB:CC...]"
        if (start_idx != -1 && end_idx != -1) {
            active_discovery_target->mac_or_ip = addr_str.substring(start_idx + 1, end_idx);
        } else {
            active_discovery_target->mac_or_ip = addr_str;
        }
        
        Serial.printf("[UI] Tilldelade adress %s till %s\n", 
                      active_discovery_target->mac_or_ip.c_str(), active_discovery_target->name.c_str());
        
        saveAllSettings(); // Spara direkt till NVS Flash
    }
    
    // Stäng popup-rutan
    if (discovery_popup != nullptr) {
        lv_obj_del(discovery_popup);
        discovery_popup = nullptr;
        discovery_list = nullptr;
        active_discovery_target = nullptr;
    }
}

// Stäng popup manuellt via krysset
static void close_popup_event_handler(lv_event_t * e) {
    if (discovery_popup != nullptr) {
        lv_obj_del(discovery_popup);
        discovery_popup = nullptr;
        discovery_list = nullptr;
        active_discovery_target = nullptr;
    }
}

/**
 * Funktion som anropas utifrån (från skannertråden) för att mata in träffar live på skärmen
 */
void ui_add_discovered_device(const char* name, const char* addr) {
    if (discovery_list == nullptr) return;
    
    String button_text = String(name) + " [" + String(addr) + "]";
    lv_obj_t * btn = lv_list_add_btn(discovery_list, LV_SYMBOL_PLUS, button_text.c_str());
    lv_obj_add_event_cb(btn, discovery_select_event_handler, LV_EVENT_CLICKED, nullptr);
}

// Öppnar popupfönstret och triggar sökningen
static void scan_trigger_callback(lv_event_t * e) {
    ScanButtonContext *scanCtx = (ScanButtonContext*)lv_event_get_user_data(e);
    active_discovery_target = scanCtx->cfg;
    
    // Skapa en modal Popup-ruta mitt på skärmen
    discovery_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(discovery_popup, 380, 360);
    lv_obj_align(discovery_popup, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(discovery_popup, LV_OBJ_FLAG_FLOATING);
    
    // Rubrik
    lv_obj_t * title = lv_label_create(discovery_popup);
    lv_label_set_text_fmt(title, "Söker %s enheter...", scanCtx->typeFilter);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 10);
    
    // Stäng-kryss
    lv_obj_t * close_btn = lv_btn_create(discovery_popup);
    lv_obj_set_size(close_btn, 35, 35);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_t * close_lbl = lv_label_create(close_btn);
    lv_label_set_text(close_lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(close_lbl);
    lv_obj_add_event_cb(close_btn, close_popup_event_handler, LV_EVENT_CLICKED, nullptr);
    
    // Rullningsbar söklista inuti popupen
    discovery_list = lv_list_create(discovery_popup);
    lv_obj_set_size(discovery_list, 340, 260);
    lv_obj_align(discovery_list, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    // Starta hårdvaruskanningen asynkront
    triggerManualDiscovery(scanCtx->typeFilter);
}

static void ta_event_handler(lv_event_t * e) {
    lv_obj_t * ta = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    UiEventContext *ctx = (UiEventContext*)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        if (main_keyboard == nullptr) {
            main_keyboard = lv_keyboard_create(lv_scr_act());
            lv_obj_set_size(main_keyboard, LV_PCT(100), LV_PCT(45));
            lv_obj_align(main_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        lv_keyboard_set_textarea(main_keyboard, ta);
        if (ctx->isKeyField) {
            lv_keyboard_set_mode(main_keyboard, LV_KEYBOARD_MODE_NUMBER);
        } else {
            lv_keyboard_set_mode(main_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        }
    }
    
    if (code == LV_EVENT_READY) {
        if (ctx->isKeyField && ctx->victronDev != nullptr) {
            ctx->victronDev->key = lv_textarea_get_text(ta);
        } else if (ctx->cfg != nullptr) {
            ctx->cfg->name = lv_textarea_get_text(ta);
        }
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

void add_device_to_settings_list(lv_obj_t * list, DeviceConfig &cfg, const char* labelText, const char* typeFilter, VictronDevice *victronDev = nullptr) {
    lv_list_add_text(list, labelText);

    // Utökad radhöjd för att rymma den nya "Sök enhet"-knappen och texten
    int row_height = (victronDev != nullptr) ? 140 : 100;

    lv_obj_t * row = lv_obj_create(list);
    lv_obj_set_size(row, LV_PCT(100), row_height);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(row, 5, 0);

    // 1. Switch för på/av
    lv_obj_t * sw = lv_switch_create(row);
    lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 5, 5);
    if (cfg.enabled) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_VALUE_CHANGED, &cfg);

    // 2. Textarea för enhetsnamn
    lv_obj_t * ta_name = lv_textarea_create(row);
    lv_obj_set_size(ta_name, 150, 40);
    lv_obj_align(ta_name, LV_ALIGN_TOP_RIGHT, -5, 0);
    lv_textarea_set_one_line(ta_name, true);
    lv_textarea_set_text(ta_name, cfg.name.c_str());
    UiEventContext *nameCtx = new UiEventContext { &cfg, victronDev, false };
    lv_obj_add_event_cb(ta_name, ta_event_handler, LV_EVENT_ALL, nameCtx);

    // 3. SÖKKNAPP (Placeras till vänster på rad två)
    lv_obj_t * scan_btn = lv_btn_create(row);
    lv_obj_set_size(scan_btn, 110, 35);
    lv_obj_align(scan_btn, LV_ALIGN_TOP_LEFT, 5, 45);
    lv_obj_set_style_bg_color(scan_btn, lv_color_make(0, 120, 215), 0); // Snygg blå färg
    lv_obj_t * scan_lbl = lv_label_create(scan_btn);
    lv_label_set_text(scan_lbl, "Sök enhet");
    lv_obj_center(scan_lbl);
    
    ScanButtonContext *scanCtx = new ScanButtonContext { &cfg, typeFilter };
    lv_obj_add_event_cb(scan_btn, scan_trigger_callback, LV_EVENT_CLICKED, scanCtx);

    // 4. Statusindikator: Visar den nuvarande tilldelade adressen
    lv_obj_t * lbl_addr = lv_label_create(row);
    String short_addr = cfg.mac_or_ip;
    if(short_addr.length() > 16) short_addr = short_addr.substring(0,14) + "..";
    lv_label_set_text_fmt(lbl_addr, "Adr: %s", short_addr.c_str());
    lv_obj_align(lbl_addr, LV_ALIGN_TOP_LEFT, 125, 55);
    lv_obj_set_style_text_font(lbl_addr, &lv_font_montserrat_12, 0);

    // 5. Victron-specifikt nyckelfält längst ner
    if (victronDev != nullptr) {
        lv_obj_t * lbl_key = lv_label_create(row);
        lv_label_set_text(lbl_key, "Kryptonyckel:");
        lv_obj_align(lbl_key, LV_ALIGN_BOTTOM_LEFT, 5, -12);
        lv_obj_set_style_text_font(lbl_key, &lv_font_montserrat_12, 0);

        lv_obj_t * ta_key = lv_textarea_create(row);
        lv_obj_set_size(ta_key, 220, 40);
        lv_obj_align(ta_key, LV_ALIGN_BOTTOM_RIGHT, -5, 0);
        lv_textarea_set_one_line(ta_key, true);
        lv_textarea_set_max_length(ta_key, 32);
        lv_textarea_set_text(ta_key, victronDev->key.c_str());
        UiEventContext *keyCtx = new UiEventContext { &cfg, victronDev, true };
        lv_obj_add_event_cb(ta_key, ta_event_handler, LV_EVENT_ALL, keyCtx);
    }
}

void create_settings_page(lv_obj_t * parent) {
    lv_obj_t * list = lv_list_create(parent);
    lv_obj_set_size(list, 450, 400);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);

    // Lägg till enheter med tillhörande filtersträngar för skannern
    add_device_to_settings_list(list, shunt.cfg, "Victron SmartShunt", "VICTRON", &shunt);
    add_device_to_settings_list(list, mppt.cfg, "Victron MPPT Solceller", "VICTRON", &mppt);
    add_device_to_settings_list(list, ip22.cfg, "Victron IP22 Laddare", "VICTRON", &ip22);
    
    add_device_to_settings_list(list, ruuvi.cfg, "RuuviTag Sensor", "RUUVI");
    add_device_to_settings_list(list, mijia.cfg, "Xiaomi Mijia Sensor", "XIAOMI");
    add_device_to_settings_list(list, shellyPro1.cfg, "Shelly Pro 1 (1-Kanal)", "SHELLY");
    add_device_to_settings_list(list, shellyPro2.cfg, "Shelly Pro 2 (2-Kanal)", "SHELLY");
}

#endif // UI_MANAGER_H
