#include "ui_manager.h"

// Referera till animation-callbacks allokerade i första filen
extern void anim_x_cb(void * var, int32_t v);
extern void anim_y_cb(void * var, int32_t v);

void ui_update_live_data() {
    if (ui_style_version == 1 || battery_arc == nullptr) return;
    
    if (lvgl_mutex != NULL && xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(15)) == pdTRUE) {
        
        // 1. BATTERIUPPDATERING (SmartShunt)
        if (shunt.cfg.enabled) {
            lv_arc_set_value(battery_arc, (int)shunt.soc);
            lv_label_set_text_fmt(lbl_soc_value, "%d%%", (int)shunt.soc);
            lv_label_set_text_fmt(lbl_power_value, "%.0f W", shunt.power);
            lv_label_set_text_fmt(lbl_specs_value, "%.1fV  |  %.1fA", shunt.voltage, shunt.current);
            lv_obj_set_style_text_color(lbl_power_value, (shunt.power >= 0) ? lv_color_make(39, 174, 96) : lv_color_make(231, 76, 60), 0);
        }

        // 2. REGULERING SOLFLÖDE (MPPT)
        if (mppt.cfg.enabled) {
            lv_arc_set_value(mppt_arc, (int)mppt.power);
            lv_label_set_text_fmt(lbl_mppt_pwr, "%.0f W", mppt.power);

            if (mppt.power > 5.0) {
                lv_obj_clear_flag(particle_mppt, LV_OBJ_FLAG_HIDDEN);
                uint32_t speed = lv_map((int)mppt.power, 0, 400, 2000, 400);
                lv_anim_set_values(&anim_mppt, 55, 130); lv_anim_set_exec_cb(&anim_mppt, anim_x_cb); lv_anim_set_time(&anim_mppt, speed); lv_anim_start(&anim_mppt);
                lv_anim_set_values(&anim_mppt, 55, 140); lv_anim_set_exec_cb(&anim_mppt, anim_y_cb); lv_anim_start(&anim_mppt);
            } else {
                lv_obj_add_flag(particle_mppt, LV_OBJ_FLAG_HIDDEN);
                lv_anim_del(particle_mppt, NULL);
            }
        }

        // 3. REGULERING LANDSTRÖMSFLÖDE (IP22)
        if (ip22.cfg.enabled) {
            lv_arc_set_value(ip22_arc, (int)ip22.power);
            lv_label_set_text_fmt(lbl_ip22_pwr, "%.0f W", ip22.power);

            if (ip22.power > 5.0) {
                lv_obj_clear_flag(particle_ip22, LV_OBJ_FLAG_HIDDEN);
                uint32_t speed = lv_map((int)ip22.power, 0, 800, 2000, 400);
                lv_anim_set_values(&anim_ip22, 425, 350); lv_anim_set_exec_cb(&anim_ip22, anim_x_cb); lv_anim_set_time(&anim_ip22, speed); lv_anim_start(&anim_ip22);
                lv_anim_set_values(&anim_ip22, 55, 140);  lv_anim_set_exec_cb(&anim_ip22, anim_y_cb); lv_anim_start(&anim_ip22);
            } else {
                lv_obj_add_flag(particle_ip22, LV_OBJ_FLAG_HIDDEN);
                lv_anim_del(particle_ip22, NULL);
            }
        }

        // 4. REGULERING INVERTERFLÖDE (Phoenix Inverter)
        if (inverter.cfg.enabled && lbl_inverter_pwr != nullptr) {
            if (inverter.state == 3) {
                lv_label_set_text_fmt(lbl_inverter_pwr, "Inverter: %.0f W", inverter.ac_power);
                if (inverter.ac_power > 10.0) {
                    lv_obj_clear_flag(particle_inverter, LV_OBJ_FLAG_HIDDEN);
                    uint32_t speed = lv_map((int)inverter.ac_power, 0, 1500, 2000, 300);
                    lv_anim_set_values(&anim_inverter, 240, 240); lv_anim_set_exec_cb(&anim_inverter, anim_x_cb); lv_anim_set_time(&anim_inverter, speed); lv_anim_start(&anim_inverter);
                    lv_anim_set_values(&anim_inverter, 170, 240); lv_anim_set_exec_cb(&anim_inverter, anim_y_cb); lv_anim_start(&anim_inverter);
                }
            } else {
                lv_label_set_text(lbl_inverter_pwr, inverter.state == 4 ? "Inverter: ECO" : "Inverter: OFF");
                lv_obj_add_flag(particle_inverter, LV_OBJ_FLAG_HIDDEN);
                lv_anim_del(particle_inverter, NULL);
            }
        }

        // 5. SYNCHRONISERING AV RUUVI & XIAOMI SENSORER
        if (ruuvi.cfg.enabled) lv_label_set_text_fmt(lbl_ruuvi_data, "%s:\n%.1f °C\n%.0f %%", ruuvi.cfg.name.c_str(), ruuvi.temperature, ruuvi.humidity);
        if (mijia.cfg.enabled) lv_label_set_text_fmt(lbl_xiaomi_data, "%s:\n%.1f °C\n%.0f %%", mijia.cfg.name.c_str(), mijia.temperature, mijia.humidity);
        
        xSemaphoreGive(lvgl_mutex);
    }
}
