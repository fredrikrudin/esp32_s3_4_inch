#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "storage_manager.h"

extern void triggerManualDiscovery(const char* typeFilter); 
extern void create_overview_page(lv_obj_t *parent); 
extern void create_classic_overview_page(lv_obj_t *parent);
extern void create_environment_page(lv_obj_t *parent);
extern void ui_update_live_data();
extern void ui_update_environment_data();

extern lv_obj_t *main_keyboard;
extern lv_obj_t *lbl_footer_clock;
extern lv_obj_t *btn_hamburger;
extern lv_obj_t *page_settings_container;
extern lv_obj_t *page_overview_container;
extern lv_obj_t *discovery_popup;
extern lv_obj_t *discovery_list;
extern DeviceConfig *active_discovery_target;

struct UiEventContext { DeviceConfig *cfg; VictronDevice *victronDev; bool isKeyField; };
struct ScanButtonContext { DeviceConfig *cfg; const char* typeFilter; };

static void ui_brightness_event_handler(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if (display_brightness < 64)       display_brightness = 128;
        else if (display_brightness < 128)  display_brightness = 192;
        else if (display_brightness < 192)  display_brightness = 255;
        else                                display_brightness = 64;
        extern void setDisplayBrightness(int brightness);
        setDisplayBrightness(display_brightness);
        lv_label_set_text_fmt(lv_obj_get_child(lv_event_get_target(e), 0), "Ljusstyrka: %d%%", (display_brightness * 100) / 255);
        saveAllSettings();
    }
}

static void ui_manual_relay_event_handler(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        uint32_t relayPin = (uint32_t)lv_event_get_user_data(e);
        bool isChecked = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
        extern void setExternalRelay(uint8_t relayPin, bool state);
        setExternalRelay((uint8_t)relayPin, isChecked);
    }
}

static void ui_style_toggle_event_handler(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if (ui_style_version == 2) { ui_style_version = 1; lv_label_set_text(lv_obj_get_child(lv_event_get_target(e), 0), "Gränssnitt: Klassisk v1"); }
        else { ui_style_version = 2; lv_label_set_text(lv_obj_get_child(lv_event_get_target(e), 0), "Gränssnitt: Cirkulär v2"); }
        saveScheduleToNVS(); 
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

// Lägg till dessa anrop i din slutgiltiga inställningslista
inline void add_ui_controls_to_settings(lv_obj_t * list) {
    lv_list_add_text(list, "System & Reläkontroller");
    
    // Gränssnittsväljare
    lv_obj_t * row_style = lv_obj_create(list); lv_obj_set_size(row_style, LV_PCT(100), 60);
    lv_obj_t * btn_style = lv_btn_create(row_style); lv_obj_set_size(btn_style, 200, 35); lv_obj_center(btn_style);
    lv_obj_t * lbl_style = lv_label_create(btn_style);
    lv_label_set_text(lbl_style, ui_style_version == 2 ? "Gränssnitt: Cirkulär v2" : "Gränssnitt: Klassisk v1");
    lv_obj_center(lbl_style);
    lv_obj_add_event_cb(btn_style, ui_style_toggle_event_handler, LV_EVENT_CLICKED, nullptr);

    // Ljusstyrka
    lv_obj_t * row_bright = lv_obj_create(list); lv_obj_set_size(row_bright, LV_PCT(100), 60);
    lv_obj_t * btn_bright = lv_btn_create(row_bright); lv_obj_set_size(btn_bright, 200, 35); lv_obj_center(btn_bright);
    lv_obj_t * lbl_bright = lv_label_create(btn_bright);
    lv_label_set_text_fmt(lbl_bright, "Ljusstyrka: %d%%", (display_brightness * 100) / 255);
    lv_obj_center(lbl_bright);
    lv_obj_add_event_cb(btn_bright, ui_brightness_event_handler, LV_EVENT_CLICKED, nullptr);
}

inline void add_manual_relay_to_settings(lv_obj_t * list, uint8_t relayPin, const char* relayName) {
    lv_obj_t * row = lv_obj_create(list); lv_obj_set_size(row, LV_PCT(100), 60);
    lv_obj_t * lbl = lv_label_create(row); lv_label_set_text_fmt(lbl, "R%d: %s", relayPin + 1, relayName);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_t * sw = lv_switch_create(row); lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_add_event_cb(sw, ui_manual_relay_event_handler, LV_EVENT_VALUE_CHANGED, (void*)(uint32_t)relayPin);
}

#endif // UI_MANAGER_H
