#ifndef PAGE_SHELLY_H
#define PAGE_SHELLY_H

#include <lvgl.h>
#include "config.h"

// Externa funktioner från network_manager.h
extern void controlShellyProRelay(ShellyDevice &device, int channel, bool state);

// Globala pekare till knapparna så vi kan ändra deras färg live
lv_obj_t *btn_sh_pro1_ch0 = nullptr;
lv_obj_t *btn_sh_pro2_ch0 = nullptr;
lv_obj_t *btn_sh_pro2_ch1 = nullptr;

// Globala textetiketter på knapparna (visar ON/OFF)
lv_obj_t *lbl_sh_pro1_ch0 = nullptr;
lv_obj_t *lbl_sh_pro2_ch0 = nullptr;
lv_obj_t *lbl_sh_pro2_ch1 = nullptr;

// Container för att dölja sidan om Shelly inte används
lv_obj_t *cont_shelly_page = nullptr;

// Callback-funktion när någon trycker på en Shelly-knapp på skärmen
static void shelly_btn_click_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    int type = (int)lv_event_get_user_data(e); // 1 = Pro 1 Ch0, 2 = Pro 2 Ch0, 3 = Pro 2 Ch1

    if (type == 1) {
        bool current_state = shellyPro1.channel_states[0];
        controlShellyProRelay(shellyPro1, 0, !current_state);
    } 
    else if (type == 2) {
        bool current_state = shellyPro2.channel_states[0];
        controlShellyProRelay(shellyPro2, 0, !current_state);
    } 
    else if (type == 3) {
        bool current_state = shellyPro2.channel_states[1];
        controlShellyProRelay(shellyPro2, 1, !current_state);
    }
}

/**
 * Hjälpfunktion för att skapa ett snyggt text- och knappblock för ett Shelly-relä
 */
lv_obj_t* create_shelly_row(lv_obj_t *parent, const char* name, int y_pos, const char* ch_label, int click_id, lv_obj_t **out_btn, lv_obj_t **out_lbl) {
    // Skapa en mörk bakgrundsrad i Victron-stil
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, 440, 65);
    lv_obj_set_pos(row, 5, y_pos);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(row, lv_color_make(30, 35, 45), 0);
    lv_obj_set_style_border_color(row, lv_color_make(45, 50, 65), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_radius(row, 10, 0);

    // Enhetsnamn och kanal till vänster
    lv_obj_t *lbl_name = lv_label_create(row);
    lv_label_set_text_fmt(lbl_name, "%s (%s)", name, ch_label);
    lv_obj_align(lbl_name, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_text_font(lbl_name, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_name, lv_color_make(220, 225, 230), 0);

    // Knapp till höger
    lv_obj_t *btn = lv_btn_create(row);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_add_event_cb(btn, shelly_btn_click_cb, LV_EVENT_CLICKED, (void*)click_id);

    // Text inuti knappen (ON/OFF)
    lv_obj_t *lbl_btn = lv_label_create(btn);
    lv_label_set_text(lbl_btn, "---");
    lv_obj_center(lbl_btn);
    lv_obj_set_style_text_font(lbl_btn, &lv_font_montserrat_14, 0);

    *out_btn = btn;
    *out_lbl = lbl_btn;
    return row;
}

/**
 * Genererar den grafiska layouten för Shelly-sidan vid boot
 */
void create_shelly_page(lv_obj_t *parent) {
    cont_shelly_page = lv_obj_create(parent);
    lv_obj_set_size(cont_shelly_page, 460, 340);
    lv_obj_align(cont_shelly_page, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_clear_flag(cont_shelly_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opacity(cont_shelly_page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont_shelly_page, 0, 0);

    // Skapa rader för dina Shelly Pro-enheter (Placeras under varandra vertikalt)
    create_shelly_row(cont_shelly_page, shellyPro1.cfg.name.c_str(), 10, "Kanal 1", 1, &btn_sh_pro1_ch0, &lbl_sh_pro1_ch0);
    create_shelly_row(cont_shelly_page, shellyPro2.cfg.name.c_str(), 85, "Kanal 1", 2, &btn_sh_pro2_ch0, &lbl_sh_pro2_ch0);
    create_shelly_row(cont_shelly_page, shellyPro2.cfg.name.c_str(), 160, "Kanal 2", 3, &btn_sh_pro2_ch1, &lbl_sh_pro2_ch1);
}

/**
 * UPPDATERINGSFUNKTION: Körs live i loop() varje sekund.
 * Ändrar färg på knapparna (Grön = PÅ, Röd/Grå = AV) och döljer sidan om enheterna är avstängda.
 */
void update_shelly_page_live() {
    if (cont_shelly_page == nullptr) return;

    // Om ingen Shelly-enhet är aktiverad i inställningarna -> Dölj hela vyn
    if (!shellyPro1.cfg.enabled && !shellyPro2.cfg.enabled) {
        lv_obj_add_flag(cont_shelly_page, LV_OBJ_FLAG_HIDDEN);
        return;
    } else {
        lv_obj_clear_flag(cont_shelly_page, LV_OBJ_FLAG_HIDDEN);
    }

    // --- 1. Uppdatera Shelly Pro 1 (Kanal 0) ---
    if (btn_sh_pro1_ch0 != nullptr && lbl_sh_pro1_ch0 != nullptr) {
        if (shellyPro1.cfg.enabled) {
            lv_obj_clear_flag(lv_obj_get_parent(btn_sh_pro1_ch0), LV_OBJ_FLAG_HIDDEN);
            bool is_on = shellyPro1.channel_states[0];
            lv_label_set_text(lbl_sh_pro1_ch0, is_on ? "ON" : "OFF");
            lv_obj_set_style_bg_color(btn_sh_pro1_ch0, is_on ? lv_color_make(40, 180, 99) : lv_color_make(180, 50, 50), 0);
        } else {
            lv_obj_add_flag(lv_obj_get_parent(btn_sh_pro1_ch0), LV_OBJ_FLAG_HIDDEN);
        }
    }

    // --- 2. Uppdatera Shelly Pro 2 (Kanal 0) ---
    if (btn_sh_pro2_ch0 != nullptr && lbl_sh_pro2_ch0 != nullptr) {
        if (shellyPro2.cfg.enabled) {
            lv_obj_clear_flag(lv_obj_get_parent(btn_sh_pro2_ch0), LV_OBJ_FLAG_HIDDEN);
            bool is_on = shellyPro2.channel_states[0];
            lv_label_set_text(lbl_sh_pro2_ch0, is_on ? "ON" : "OFF");
            lv_obj_set_style_bg_color(btn_sh_pro2_ch0, is_on ? lv_color_make(40, 180, 99) : lv_color_make(180, 50, 50), 0);
        } else {
            lv_obj_add_flag(lv_obj_get_parent(btn_sh_pro2_ch0), LV_OBJ_FLAG_HIDDEN);
        }
    }

    // --- 3. Uppdatera Shelly Pro 2 (Kanal 1) ---
    if (btn_sh_pro2_ch1 != nullptr && lbl_sh_pro2_ch1 != nullptr) {
        if (shellyPro2.cfg.enabled) {
            bool is_on = shellyPro2.channel_states[1];
            lv_label_set_text(lbl_sh_pro2_ch1, is_on ? "ON" : "OFF");
            lv_obj_set_style_bg_color(btn_sh_pro2_ch1, is_on ? lv_color_make(40, 180, 99) : lv_color_make(180, 50, 50), 0);
        }
    }
}

#endif // PAGE_SHELLY_H
