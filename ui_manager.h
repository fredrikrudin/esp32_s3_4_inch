#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "storage_manager.h"

// Externa funktionsdeklarationer från andra moduler
extern void triggerManualDiscovery(const char* typeFilter); 
extern void create_overview_page(lv_obj_t *parent);
extern void create_settings_page(lv_obj_t *parent);

// Globala gränssnittsobjekt för navigering och inmatning
lv_obj_t *main_keyboard = nullptr;
lv_obj_t *lbl_footer_clock = nullptr;
lv_obj_t *btn_hamburger = nullptr;
lv_obj_t *page_settings_container = nullptr;
lv_obj_t *page_overview_container = nullptr;

// Globala objekt för sökmodulen (Discovery Popup)
lv_obj_t *discovery_popup = nullptr;
lv_obj_t *discovery_list = nullptr;
DeviceConfig *active_discovery_target = nullptr;

// Kontextstruktur för tangentbordsinmatning (Namn vs AES-nyckel)
struct UiEventContext { 
    DeviceConfig *cfg; 
    VictronDevice *victronDev; 
    bool isKeyField; 
};

// Kontextstruktur för sökknappar
struct ScanButtonContext { 
    DeviceConfig *cfg; 
    const char* typeFilter; 
};

// Callback när användaren klickar i en textruta eller trycker på "Klar/Bock"
static void ta_event_handler(lv_event_t * e) {
    lv_obj_t * ta = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    UiEventContext *ctx = (UiEventContext*)lv_event_get_user_data(e);
    
    if (code == LV_EVENT_CLICKED) {
        if (main_keyboard == nullptr) {
            main_keyboard = lv_keyboard_create(lv_scr_act());
            lv_obj_set_size(main_keyboard, LV_PCT(100), LV_PCT(40)); // Täcker max 40% så bottenmenyn syns
            lv_obj_align(main_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        lv_keyboard_set_textarea(main_keyboard, ta);
        lv_keyboard_set_mode(main_keyboard, ctx->isKeyField ? LV_KEYBOARD_MODE_NUMBER : LV_KEYBOARD_MODE_TEXT_LOWER);
    }
    
    if (code == LV_EVENT_READY) {
        if (ctx->isKeyField && ctx->victronDev != nullptr) {
            ctx->victronDev->key = lv_textarea_get_text(ta);
        } else if (ctx->cfg != nullptr) {
            ctx->cfg->name = lv_textarea_get_text(ta);
        }
        saveAllSettings(); // Spara omedelbart till interna NVS-flashet
        if (main_keyboard != nullptr) { 
            lv_obj_del(main_keyboard); 
            main_keyboard = nullptr; 
        }
    }
}

// Callback när en enhet aktiveras eller inaktiveras via en switch
static void switch_event_handler(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    DeviceConfig *cfg = (DeviceConfig*)lv_event_get_user_data(e);
    cfg->enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    saveAllSettings();
}

// Callback när en hittad enhet väljs i söklistan
static void discovery_select_event_handler(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    const char * selected_addr = lv_list_get_btn_text(discovery_list, btn);
    
    if (active_discovery_target != nullptr && selected_addr != nullptr) {
        String addr_str = String(selected_addr);
        int start_idx = addr_str.indexOf('['); 
        int end_idx = addr_str.indexOf(']');
        
        if (start_idx != -1 && end_idx != -1) {
            active_discovery_target->mac_or_ip = addr_str.substring(start_idx + 1, end_idx);
        } else {
            active_discovery_target->mac_or_ip = addr_str;
        }
        saveAllSettings();
    }
    if (discovery_popup != nullptr) { 
        lv_obj_del(discovery_popup); 
        discovery_popup = nullptr; 
        discovery_list = nullptr; 
        active_discovery_target = nullptr; 
    }
}

// Callback för att stänga sökfönstret manuellt via krysset
static void close_popup_event_handler(lv_event_t * e) {
    if (discovery_popup != nullptr) { 
        lv_obj_del(discovery_popup); 
        discovery_popup = nullptr; 
        discovery_list = nullptr; 
        active_discovery_target = nullptr; 
    }
}

// Lägger till en funnen enhet i listraderna live under pågående skanning
void ui_add_discovered_device(const char* name, const char* addr) {
    if (discovery_list == nullptr) return;
    String button_text = String(name) + " [" + String(addr) + "]";
    lv_obj_t * btn = lv_list_add_btn(discovery_list, LV_SYMBOL_PLUS, button_text.c_str());
    lv_obj_add_event_cb(btn, discovery_select_event_handler, LV_EVENT_CLICKED, nullptr);
}

// Triggari hårdvaruskanningen och ritar upp popup-fönstret
static void scan_trigger_callback(lv_event_t * e) {
    ScanButtonContext *scanCtx = (ScanButtonContext*)lv_event_get_user_data(e);
    active_discovery_target = scanCtx->cfg;
    
    discovery_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(discovery_popup, 380, 320);
    lv_obj_align(discovery_popup, LV_ALIGN_CENTER, 0, -20);
    
    lv_obj_t * title = lv_label_create(discovery_popup);
    lv_label_set_text_fmt(title, "Söker %s...", scanCtx->typeFilter);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 10);
    
    lv_obj_t * close_btn = lv_btn_create(discovery_popup);
    lv_obj_set_size(close_btn, 35, 35);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_t * close_lbl = lv_label_create(close_btn);
    lv_label_set_text(close_lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(close_lbl);
    lv_obj_add_event_cb(close_btn, close_popup_event_handler, LV_EVENT_CLICKED, nullptr);
    
    discovery_list = lv_list_create(discovery_popup);
    lv_obj_set_size(discovery_list, 340, 220);
    lv_obj_align(discovery_list, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    triggerManualDiscovery(scanCtx->typeFilter);
}

// Växlar mjukt mellan Mätarskärmen och Inställningsmenyn
static void hamburger_click_event_handler(lv_event_t * e) {
    if (lv_obj_has_flag(page_settings_container, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(page_overview_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_settings_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_t * label = lv_obj_get_child(btn_hamburger, 0);
        lv_label_set_text(label, LV_SYMBOL_HOME);
    } else {
        lv_obj_add_flag(page_settings_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_overview_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_t * label = lv_obj_get_child(btn_hamburger, 0);
        lv_label_set_text(label, LV_SYMBOL_LIST);
    }
}
// Skapar en enskild rad i inställningslistan med switch, textfält och sökknapp
void add_device_to_settings_list(lv_obj_t * list, DeviceConfig &cfg, const char* labelText, const char* typeFilter, VictronDevice *victronDev = nullptr) {
    lv_list_add_text(list, labelText);
    int row_height = (victronDev != nullptr) ? 140 : 100;
    
    lv_obj_t * row = lv_obj_create(list);
    lv_obj_set_size(row, LV_PCT(100), row_height);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(row, 5, 0);

    lv_obj_t * sw = lv_switch_create(row);
    lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 5, 5);
    if (cfg.enabled) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_VALUE_CHANGED, &cfg);

    lv_obj_t * ta_name = lv_textarea_create(row);
    lv_obj_set_size(ta_name, 150, 40);
    lv_obj_align(ta_name, LV_ALIGN_TOP_RIGHT, -5, 0);
    lv_textarea_set_one_line(ta_name, true);
    lv_textarea_set_text(ta_name, cfg.name.c_str());
    
    // Fixat RAM-läckage (Memory Leaks) via fasta statiska arrayer
    static UiEventContext nameContexts[10];
    static int nameCtxCount = 0;
    if (nameCtxCount < 10) {
        nameContexts[nameCtxCount] = { &cfg, victronDev, false };
        lv_obj_add_event_cb(ta_name, ta_event_handler, LV_EVENT_ALL, &nameContexts[nameCtxCount]);
        nameCtxCount++;
    }

    lv_obj_t * scan_btn = lv_btn_create(row);
    lv_obj_set_size(scan_btn, 110, 35);
    lv_obj_align(scan_btn, LV_ALIGN_TOP_LEFT, 5, 45);
    lv_obj_set_style_bg_color(scan_btn, lv_color_make(0, 120, 215), 0);
    lv_obj_t * scan_lbl = lv_label_create(scan_btn);
    lv_label_set_text(scan_lbl, "Sök enhet");
    lv_obj_center(scan_lbl);
    
    static ScanButtonContext scanContexts[10];
    static int scanCtxCount = 0;
    if (scanCtxCount < 10) {
        scanContexts[scanCtxCount] = { &cfg, typeFilter };
        lv_obj_add_event_cb(scan_btn, scan_trigger_callback, LV_EVENT_CLICKED, &scanContexts[scanCtxCount]);
        scanCtxCount++;
    }

    lv_obj_t * lbl_addr = lv_label_create(row);
    String short_addr = cfg.mac_or_ip;
    if(short_addr.length() > 16) short_addr = short_addr.substring(0,14) + "..";
    lv_label_set_text_fmt(lbl_addr, "Adr: %s", short_addr.c_str());
    lv_obj_align(lbl_addr, LV_ALIGN_TOP_LEFT, 125, 55);
    lv_obj_set_style_text_font(lbl_addr, &lv_font_montserrat_12, 0);

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
        
        static UiEventContext keyContexts[5];
        static int keyCtxCount = 0;
        if (keyCtxCount < 5) {
            keyContexts[keyCtxCount] = { &cfg, victronDev, true };
            lv_obj_add_event_cb(ta_key, ta_event_handler, LV_EVENT_ALL, &keyContexts[keyCtxCount]);
            keyCtxCount++;
        }
    }
}

// Genererar inställningssidan och dess listvy
void create_settings_page(lv_obj_t * parent) {
    lv_obj_t * list = lv_list_create(parent);
    lv_obj_set_size(list, 450, 340); 
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 10);

    add_device_to_settings_list(list, shunt.cfg, "Victron SmartShunt", "VICTRON", &shunt);
    add_device_to_settings_list(list, mppt.cfg, "Victron MPPT Solceller", "VICTRON", &mppt);
    add_device_to_settings_list(list, ip22.cfg, "Victron IP22 Laddare", "VICTRON", &ip22);
    add_device_to_settings_list(list, ruuvi.cfg, "RuuviTag Sensor", "RUUVI");
    add_device_to_settings_list(list, mijia.cfg, "Xiaomi Mijia Sensor", "XIAOMI");
    add_device_to_settings_list(list, shellyPro1.cfg, "Shelly Pro 1 (1-Kanal)", "SHELLY");
    add_device_to_settings_list(list, shellyPro2.cfg, "Shelly Pro 2 (2-Kanal)", "SHELLY");
}

// Skapar den fasta bottenmenyn med klocka
void create_footer_navigation() {
    lv_obj_t *footer = lv_obj_create(lv_scr_act());
    lv_obj_set_size(footer, 480, 50);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(footer, lv_color_make(10, 12, 16), 0);
    lv_obj_set_style_border_color(footer, lv_color_make(35, 40, 50), 0);
    lv_obj_set_style_border_width(footer, 1, 0);
    lv_obj_set_style_border_side(footer, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_radius(footer, 0, 0);
    lv_obj_set_style_pad_all(footer, 0, 0);

    lbl_footer_clock = lv_label_create(footer);
    lv_label_set_text(lbl_footer_clock, "12:00");
    lv_obj_align(lbl_footer_clock, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_text_font(lbl_footer_clock, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl_footer_clock, lv_color_make(240, 240, 240), 0);

    btn_hamburger = lv_btn_create(footer);
    lv_obj_set_size(btn_hamburger, 45, 45);
    lv_obj_align(btn_hamburger, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_opacity(btn_hamburger, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_hamburger, 0, 0);

    lv_obj_t *btn_label = lv_label_create(btn_hamburger);
    lv_label_set_text(btn_label, LV_SYMBOL_LIST);
    lv_obj_center(btn_label);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(btn_label, lv_color_make(0, 162, 232), 0);

    lv_obj_add_event_cb(btn_hamburger, hamburger_click_event_handler, LV_EVENT_CLICKED, nullptr);
}

// Systeminitiering av fönsterlagren (Containers)
void initDisplayAndUI() {
    page_settings_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_settings_container, 480, 430);
    lv_obj_align(page_settings_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(page_settings_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(page_settings_container, lv_color_make(15, 18, 24), 0);
    lv_obj_set_style_border_width(page_settings_container, 0, 0);
    create_settings_page(page_settings_container);

    page_overview_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_overview_container, 480, 430);
    lv_obj_align(page_overview_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(page_overview_container, lv_color_make(15, 18, 24), 0);
    lv_obj_set_style_border_width(page_overview_container, 0, 0);
    create_overview_page(page_overview_container);

    create_footer_navigation();
}

#endif // UI_MANAGER_H
