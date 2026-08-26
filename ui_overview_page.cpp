#include "ui_manager.h"

lv_obj_t *battery_arc = nullptr;
lv_obj_t *lbl_soc_value = nullptr;
lv_obj_t *lbl_power_value = nullptr;
lv_obj_t *lbl_specs_value = nullptr;
lv_obj_t *mppt_arc = nullptr;
lv_obj_t *lbl_mppt_pwr = nullptr;
lv_obj_t *ip22_arc = nullptr;
lv_obj_t *lbl_ip22_pwr = nullptr;
lv_obj_t *lbl_ruuvi_data = nullptr;
lv_obj_t *lbl_xiaomi_data = nullptr;

void create_overview_page(lv_obj_t *parent) {
    if (ui_style_version == 1) return; // Säkerhetsstopp
    
    lv_obj_set_style_bg_color(parent, lv_color_make(12, 12, 12), 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // Central mätare
    battery_arc = lv_arc_create(parent);
    lv_obj_set_size(battery_arc, 170, 170);
    lv_obj_align(battery_arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_range(battery_arc, 0, 100);
    lv_arc_set_bg_angles(battery_arc, 130, 50);
    lv_arc_set_value(battery_arc, 100);
    lv_obj_remove_style(battery_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(battery_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(battery_arc, lv_color_make(39, 174, 96), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(battery_arc, lv_color_make(35, 35, 35), LV_PART_MAIN);
    lv_obj_set_style_arc_width(battery_arc, 12, LV_PART_ANY);

    lbl_soc_value = lv_label_create(parent);
    lv_label_set_text(lbl_soc_value, "--%");
    lv_obj_set_style_text_font(lbl_soc_value, &lv_font_montserrat_32, 0);
    lv_obj_align_to(lbl_soc_value, battery_arc, LV_ALIGN_CENTER, 0, -20);

    lbl_power_value = lv_label_create(parent);
    lv_label_set_text(lbl_power_value, "0 W");
    lv_obj_set_style_text_font(lbl_power_value, &lv_font_montserrat_16, 0);
    lv_obj_align_to(lbl_power_value, lbl_soc_value, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    lbl_specs_value = lv_label_create(parent);
    lv_label_set_text(lbl_specs_value, "0.0V  |  0.0A");
    lv_obj_set_style_text_font(lbl_specs_value, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_specs_value, lv_color_make(150, 150, 150), 0);
    lv_obj_align_to(lbl_specs_value, battery_arc, LV_ALIGN_BOTTOM_MID, 0, -20);

    // MPPT (Solar)
    mppt_arc = lv_arc_create(parent);
    lv_obj_set_size(mppt_arc, 75, 75);
    lv_obj_align(mppt_arc, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_arc_set_range(mppt_arc, 0, 500);
    lv_arc_set_bg_angles(mppt_arc, 180, 360);
    lv_arc_set_value(mppt_arc, 0);
    lv_obj_remove_style(mppt_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(mppt_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(mppt_arc, lv_color_make(241, 196, 15), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(mppt_arc, lv_color_make(25, 25, 25), LV_PART_MAIN);
    lv_obj_set_style_arc_width(mppt_arc, 6, LV_PART_ANY);

    lv_obj_t *lbl_mppt_title = lv_label_create(parent);
    lv_label_set_text(lbl_mppt_title, "SOLAR");
    lv_obj_set_style_text_font(lbl_mppt_title, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_mppt_title, lv_color_make(120, 120, 120), 0);
    lv_obj_align_to(lbl_mppt_title, mppt_arc, LV_ALIGN_CENTER, 0, -10);

    lbl_mppt_pwr = lv_label_create(parent);
    lv_label_set_text(lbl_mppt_pwr, "0 W");
    lv_obj_set_style_text_font(lbl_mppt_pwr, &lv_font_montserrat_12, 0);
    lv_obj_align_to(lbl_mppt_pwr, mppt_arc, LV_ALIGN_CENTER, 0, 8);

    // IP22 (AC In)
    ip22_arc = lv_arc_create(parent);
    lv_obj_set_size(ip22_arc, 75, 75);
    lv_obj_align(ip22_arc, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_arc_set_range(ip22_arc, 0, 1000);
    lv_arc_set_bg_angles(ip22_arc, 180, 360);
    lv_arc_set_value(ip22_arc, 0);
    lv_obj_remove_style(ip22_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(ip22_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(ip22_arc, lv_color_make(41, 128, 185), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ip22_arc, lv_color_make(25, 25, 25), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ip22_arc, 6, LV_PART_ANY);

    lv_obj_t *lbl_ip22_title = lv_label_create(parent);
    lv_label_set_text(lbl_ip22_title, "AC IN");
    lv_obj_set_style_text_font(lbl_ip22_title, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_ip22_title, lv_color_make(120, 120, 120), 0);
    lv_obj_align_to(lbl_ip22_title, ip22_arc, LV_ALIGN_CENTER, 0, -10);

    lbl_ip22_pwr = lv_label_create(parent);
    lv_label_set_text(lbl_ip22_pwr, "0 W");
    lv_obj_set_style_text_font(lbl_ip22_pwr, &lv_font_montserrat_12, 0);
    lv_obj_align_to(lbl_ip22_pwr, ip22_arc, LV_ALIGN_CENTER, 0, 8);

    // Sensorer
    lbl_ruuvi_data = lv_label_create(parent);
    lv_label_set_text(lbl_ruuvi_data, "Ruuvi:\n--.-°C\n--%");
    lv_obj_set_style_text_font(lbl_ruuvi_data, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_ruuvi_data, LV_ALIGN_BOTTOM_LEFT, 20, -50);

    lbl_xiaomi_data = lv_label_create(parent);
    lv_label_set_text(lbl_xiaomi_data, "Xiaomi:\n--.-°C\n--%");
    lv_obj_set_style_text_font(lbl_xiaomi_data, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_xiaomi_data, LV_ALIGN_BOTTOM_RIGHT, -20, -50);
}

void ui_update_live_data() {
    if (ui_style_version == 1 || battery_arc == nullptr) return; // Hoppa över om v1 körs
    
    if (lvgl_mutex != NULL && xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(15)) == pdTRUE) {
        if (shunt.cfg.enabled) {
            lv_arc_set_value(battery_arc, (int)shunt.soc);
            lv_label_set_text_fmt(lbl_soc_value, "%d%%", (int)shunt.soc);
            lv_label_set_text_fmt(lbl_power_value, "%.0f W", shunt.power);
            lv_label_set_text_fmt(lbl_specs_value, "%.1fV  |  %.1fA", shunt.voltage, shunt.current);
            if (shunt.power >= 0) lv_obj_set_style_text_color(lbl_power_value, lv_color_make(39, 174, 96), 0);
            else lv_obj_set_style_text_color(lbl_power_value, lv_color_make(231, 76, 60), 0);
        }
        if (mppt.cfg.enabled) {
            lv_arc_set_value(mppt_arc, (int)mppt.power);
            lv_label_set_text_fmt(lbl_mppt_pwr, "%.0f W", mppt.power);
        }
        if (ip22.cfg.enabled) {
            lv_arc_set_value(ip22_arc, (int)ip22.power);
            lv_label_set_text_fmt(lbl_ip22_pwr, "%.0f W", ip22.power);
        }
        if (ruuvi.cfg.enabled) {
            lv_label_set_text_fmt(lbl_ruuvi_data, "%s:\n%.1f °C\n%.0f %%", ruuvi.cfg.name.c_str(), ruuvi.temperature, ruuvi.humidity);
        }
        if (mijia.cfg.enabled) {
            lv_label_set_text_fmt(lbl_xiaomi_data, "%s:\n%.1f °C\n%.0f %%", mijia.cfg.name.c_str(), mijia.temperature, mijia.humidity);
        }
        xSemaphoreGive(lvgl_mutex);
    }
}
