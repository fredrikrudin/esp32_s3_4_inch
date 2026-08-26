#include "ui_manager.h"

lv_obj_t *battery_arc = nullptr; lv_obj_t *lbl_soc_value = nullptr; lv_obj_t *lbl_power_value = nullptr; lv_obj_t *lbl_specs_value = nullptr;
lv_obj_t *mppt_arc = nullptr;    lv_obj_t *lbl_mppt_pwr = nullptr;   lv_obj_t *ip22_arc = nullptr;    lv_obj_t *lbl_ip22_pwr = nullptr;
lv_obj_t *lbl_inverter_pwr = nullptr; lv_obj_t *lbl_ruuvi_data = nullptr; lv_obj_t *lbl_xiaomi_data = nullptr;

lv_obj_t *particle_mppt = nullptr; lv_obj_t *particle_ip22 = nullptr; lv_obj_t *particle_inverter = nullptr;
lv_anim_t anim_mppt; lv_anim_t anim_ip22; lv_anim_t anim_inverter;

static void anim_x_cb(void * var, int32_t v) { lv_obj_set_x((lv_obj_t*)var, v); }
static void anim_y_cb(void * var, int32_t v) { lv_obj_set_y((lv_obj_t*)var, v); }

void create_overview_page(lv_obj_t *parent) {
    if (ui_style_version == 1) return;
    lv_obj_set_style_bg_color(parent, lv_color_make(12, 12, 12), 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // Bakgrundslinjer
    static lv_point_t p_mppt[] = { {55, 55}, {130, 140} };   lv_obj_t *l_mppt = lv_line_create(parent); lv_line_set_points(l_mppt, p_mppt, 2); lv_obj_set_style_line_color(l_mppt, lv_color_make(35, 35, 35), 0);
    static lv_point_t p_ip22[] = { {425, 55}, {350, 140} }; lv_obj_t *l_ip22 = lv_line_create(parent); lv_line_set_points(l_ip22, p_ip22, 2); lv_obj_set_style_line_color(l_ip22, lv_color_make(35, 35, 35), 0);
    static lv_point_t p_inv[] = { {240, 170}, {240, 240} };  lv_obj_t *l_inv = lv_line_create(parent);  lv_line_set_points(l_inv, p_inv, 2);   lv_obj_set_style_line_color(l_inv, lv_color_make(35, 35, 35), 0);

    // Central mätare
    battery_arc = lv_arc_create(parent); lv_obj_set_size(battery_arc, 170, 170); lv_obj_align(battery_arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_range(battery_arc, 0, 100); lv_arc_set_bg_angles(battery_arc, 130, 50); lv_arc_set_value(battery_arc, 100);
    lv_obj_remove_style(battery_arc, NULL, LV_PART_KNOB); lv_obj_clear_flag(battery_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(battery_arc, lv_color_make(39, 174, 96), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(battery_arc, lv_color_make(25, 25, 25), LV_PART_MAIN);
    lv_obj_set_style_arc_width(battery_arc, 12, LV_PART_ANY);

    lbl_soc_value = lv_label_create(parent); lv_label_set_text(lbl_soc_value, "--%"); lv_obj_set_style_text_font(lbl_soc_value, &lv_font_montserrat_32, 0); lv_obj_align_to(lbl_soc_value, battery_arc, LV_ALIGN_CENTER, 0, -20);
    lbl_power_value = lv_label_create(parent); lv_label_set_text(lbl_power_value, "0 W"); lv_obj_set_style_text_font(lbl_power_value, &lv_font_montserrat_16, 0); lv_obj_align_to(lbl_power_value, lbl_soc_value, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lbl_specs_value = lv_label_create(parent); lv_label_set_text(lbl_specs_value, "0.0V  |  0.0A"); lv_obj_set_style_text_font(lbl_specs_value, &lv_font_montserrat_12, 0); lv_obj_set_style_text_color(lbl_specs_value, lv_color_make(150, 150, 150), 0); lv_obj_align_to(lbl_specs_value, battery_arc, LV_ALIGN_BOTTOM_MID, 0, -20);

    // Omgivande mätare (MPPT + AC In)
    mppt_arc = lv_arc_create(parent); lv_obj_set_size(mppt_arc, 75, 75); lv_obj_align(mppt_arc, LV_ALIGN_TOP_LEFT, 20, 20); lv_arc_set_range(mppt_arc, 0, 500); lv_arc_set_bg_angles(mppt_arc, 180, 360);
    lv_obj_remove_style(mppt_arc, NULL, LV_PART_KNOB); lv_obj_set_style_arc_color(mppt_arc, lv_color_make(241, 196, 15), LV_PART_INDICATOR);
    lbl_mppt_pwr = lv_label_create(parent); lv_obj_set_style_text_font(lbl_mppt_pwr, &lv_font_montserrat_12, 0); lv_obj_align_to(lbl_mppt_pwr, mppt_arc, LV_ALIGN_CENTER, 0, 8);

    ip22_arc = lv_arc_create(parent); lv_obj_set_size(ip22_arc, 75, 75); lv_obj_align(ip22_arc, LV_ALIGN_TOP_RIGHT, -20, 20); lv_arc_set_range(ip22_arc, 0, 1000); lv_arc_set_bg_angles(ip22_arc, 180, 360);
    lv_obj_remove_style(ip22_arc, NULL, LV_PART_KNOB); lv_obj_set_style_arc_color(ip22_arc, lv_color_make(41, 128, 185), LV_PART_INDICATOR);
    lbl_ip22_pwr = lv_label_create(parent); lv_obj_set_style_text_font(lbl_ip22_pwr, &lv_font_montserrat_12, 0); lv_obj_align_to(lbl_ip22_pwr, ip22_arc, LV_ALIGN_CENTER, 0, 8);

    // Inverter & Klimat
    lbl_inverter_pwr = lv_label_create(parent); lv_label_set_text(lbl_inverter_pwr, "Inverter: OFF"); lv_obj_set_style_text_font(lbl_inverter_pwr, &lv_font_montserrat_14, 0); lv_obj_align(lbl_inverter_pwr, LV_ALIGN_BOTTOM_MID, 0, -55);
    lbl_ruuvi_data = lv_label_create(parent); lv_obj_align(lbl_ruuvi_data, LV_ALIGN_BOTTOM_LEFT, 20, -50);
    lbl_xiaomi_data = lv_label_create(parent); lv_obj_align(lbl_xiaomi_data, LV_ALIGN_BOTTOM_RIGHT, -20, -50);

    // Partiklar
    particle_mppt = lv_obj_create(parent); lv_obj_set_size(particle_mppt, 6, 6); lv_obj_set_style_radius(particle_mppt, LV_RADIUS_CIRCLE, 0); lv_obj_set_style_bg_color(particle_mppt, lv_color_make(241, 196, 15), 0); lv_obj_add_flag(particle_mppt, LV_OBJ_FLAG_HIDDEN);
    particle_ip22 = lv_obj_create(parent); lv_obj_set_size(particle_ip22, 6, 6); lv_obj_set_style_radius(particle_ip22, LV_RADIUS_CIRCLE, 0); lv_obj_set_style_bg_color(particle_ip22, lv_color_make(41, 128, 185), 0); lv_obj_add_flag(particle_ip22, LV_OBJ_FLAG_HIDDEN);
    particle_inverter = lv_obj_create(parent); lv_obj_set_size(particle_inverter, 6, 6); lv_obj_set_style_radius(particle_inverter, LV_RADIUS_CIRCLE, 0); lv_obj_set_style_bg_color(particle_inverter, lv_color_make(231, 76, 60), 0); lv_obj_add_flag(particle_inverter, LV_OBJ_FLAG_HIDDEN);

    lv_anim_init(&anim_mppt);   lv_anim_set_var(&anim_mppt, particle_mppt);   lv_anim_set_repeat_count(&anim_mppt, LV_ANIM_REPEAT_INFINITE);
    lv_anim_init(&anim_ip22);   lv_anim_set_var(&anim_ip22, particle_ip22);   lv_anim_set_repeat_count(&anim_ip22, LV_ANIM_REPEAT_INFINITE);
    lv_anim_init(&anim_inverter); lv_anim_set_var(&anim_inverter, particle_inverter); lv_anim_set_repeat_count(&anim_inverter, LV_ANIM_REPEAT_INFINITE);
}

void ui_update_live_data() {
    if (ui_style_version == 1 || battery_arc == nullptr) return;
    if (lvgl_mutex != NULL && xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(15)) == pdTRUE) {
        if (shunt.cfg.enabled) {
            lv_arc_set_value(battery_arc, (int)shunt.soc); lv_label_set_text_fmt(lbl_soc_value, "%d%%", (int)shunt.soc);
            lv_label_set_text_fmt(lbl_power_value, "%.0f W", shunt.power); lv_label_set_text_fmt(lbl_specs_value, "%.1fV  |  %.1fA", shunt.voltage, shunt.current);
            lv_obj_set_style_text_color(lbl_power_value, shunt.power >= 0 ? lv_color_make(39, 174, 96) : lv_color_make(231, 76, 60), 0);
        }
        if (mppt.cfg.enabled && mppt.power > 5.0) {
            lv_obj_clear_flag(particle_mppt, LV_OBJ_FLAG_HIDDEN); uint32_t spd = lv_map((int)mppt.power, 0, 400, 2000, 400);
            lv_anim_set_values(&anim_mppt, 55, 130); lv_anim_set_exec_cb(&anim_mppt, anim_x_cb); lv_anim_set_time(&anim_mppt, spd); lv_anim_start(&anim_mppt);
            lv_anim_set_values(&anim_mppt, 55, 140); lv_anim_set_exec_cb(&anim_mppt, anim_y_cb); lv_anim_start(&anim_mppt);
        } else if (particle_mppt) { lv_obj_add_flag(particle_mppt, LV_OBJ_FLAG_HIDDEN); lv_anim_del(particle_mppt, NULL); }
        
        if (inverter.cfg.enabled && inverter.state == 3 && inverter.ac_power > 10.0) {
            lv_obj_clear_flag(particle_inverter, LV_OBJ_FLAG_HIDDEN); uint32_t spd = lv_map((int)inverter.ac_power, 0, 1500, 2000, 300);
            lv_anim_set_values(&anim_inverter, 240, 240); lv_anim_set_exec_cb(&anim_inverter, anim_x_cb); lv_anim_set_time(&anim_inverter, spd); lv_anim_start(&anim_inverter);
            lv_anim_set_values(&anim_inverter, 170, 240); lv_anim_set_exec_cb(&anim_inverter, anim_y_cb); lv_anim_start(&anim_inverter);
        }
        xSemaphoreGive(lvgl_mutex);
    }
}
