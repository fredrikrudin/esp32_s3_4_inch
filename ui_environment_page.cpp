#include "ui_manager.h"

// --- Globala UI-pekare för live-uppdatering av miljösidan ---
lv_obj_t *page_environment_container = nullptr;

lv_obj_t *ruuvi_temp_arc = nullptr;
lv_obj_t *lbl_ruuvi_temp = nullptr;
lv_obj_t *bar_ruuvi_hum = nullptr;
lv_obj_t *lbl_ruuvi_hum = nullptr;

lv_obj_t *xiaomi_temp_arc = nullptr;
lv_obj_t *lbl_xiaomi_temp = nullptr;
lv_obj_t *bar_xiaomi_hum = nullptr;
lv_obj_t *lbl_xiaomi_hum = nullptr;

// Hjälpfunktion för att färgsätta temperaturmätarna dynamiskt baserat på grader
static lv_color_t get_temp_color(float temp) {
    if (temp < 5.0)   return lv_color_make(41, 128, 185);  // Kallt / Frostskydd (Blå)
    if (temp < 15.0)  return lv_color_make(52, 152, 219); // Svalt (Ljusblå)
    if (temp < 25.0)  return lv_color_make(39, 174, 96);  // Behagligt / Salong (Grön)
    if (temp < 32.0)  return lv_color_make(230, 126, 34); // Varmt (Orange)
    return lv_color_make(231, 76, 60);                     // Kritiskt varmt (Röd)
}

// --- Funktion som skapar Miljösidan ---
void create_environment_page(lv_obj_t *parent) {
    // Sätt samma GUI-v2 signaturbakgrund (Helt mörkt tema)
    lv_obj_set_style_bg_color(parent, lv_color_make(12, 12, 12), 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // Gemensam rubrik i toppen av sidan
    lv_obj_t *lbl_title = lv_label_create(parent);
    lv_label_set_text(lbl_title, "⚡ KLIMAT & SENSORSTATUS");
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_title, lv_color_make(243, 156, 18), 0); // Victron Orange
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 15);

    // =================================================================
    // SENSOR 1: RUUVI TAG (VÄNSTER SIDA)
    // =================================================================
    lv_obj_t *ruuvi_box = lv_obj_create(parent);
    lv_obj_set_size(ruuvi_box, 180, 240);
    lv_obj_align(ruuvi_box, LV_ALIGN_LEFT_MID, 15, 10);
    lv_obj_set_style_bg_color(ruuvi_box, lv_color_make(20, 20, 20), 0); // Diskret mörk kort-bakgrund
    lv_obj_set_style_border_color(ruuvi_box, lv_color_make(40, 40, 40), 0);
    lv_obj_set_style_border_width(ruuvi_box, 1, 0);
    lv_obj_clear_flag(ruuvi_box, LV_OBJ_FLAG_SCROLLABLE);

    // Sensornamn / Plats
    lv_obj_t *lbl_ruuvi_title = lv_label_create(ruuvi_box);
    lv_label_set_text(lbl_ruuvi_title, ruuvi.cfg.name.length() > 0 ? ruuvi.cfg.name.c_str() : "RuuviTag");
    lv_obj_set_style_text_font(lbl_ruuvi_title, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_ruuvi_title, LV_ALIGN_TOP_MID, 0, 5);

    // Cirkulär Temperaturmätare (-20°C till +60°C)
    ruuvi_temp_arc = lv_arc_create(ruuvi_box);
    lv_obj_set_size(ruuvi_temp_arc, 100, 100);
    lv_obj_align(ruuvi_temp_arc, LV_ALIGN_TOP_MID, 0, 35);
    lv_arc_set_range(ruuvi_temp_arc, -20, 60);
    lv_arc_set_bg_angles(ruuvi_temp_arc, 130, 50);
    lv_arc_set_value(ruuvi_temp_arc, 20); // Startvärde
    lv_obj_remove_style(ruuvi_temp_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(ruuvi_temp_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(ruuvi_temp_arc, lv_color_make(35, 35, 35), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ruuvi_temp_arc, 8, LV_PART_ANY);

    // Temperaturvärde i mitten av bågen
    lbl_ruuvi_temp = lv_label_create(ruuvi_box);
    lv_label_set_text(lbl_ruuvi_temp, "--.-°C");
    lv_obj_set_style_text_font(lbl_ruuvi_temp, &lv_font_montserrat_16, 0);
    lv_obj_align_to(lbl_ruuvi_temp, ruuvi_temp_arc, LV_ALIGN_CENTER, 0, 0);

    // Luftfuktighetsmätare (Horisontell stapel i botten av kortet)
    lv_obj_t *lbl_ruuvi_hum_title = lv_label_create(ruuvi_box);
    lv_label_set_text(lbl_ruuvi_hum_title, "Luftfuktighet:");
    lv_obj_set_style_text_font(lbl_ruuvi_hum_title, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_ruuvi_hum_title, lv_color_make(130, 130, 130), 0);
    lv_obj_align(lbl_ruuvi_hum_title, LV_ALIGN_BOTTOM_LEFT, 5, -35);

    bar_ruuvi_hum = lv_bar_create(ruuvi_box);
    lv_obj_set_size(bar_ruuvi_hum, 110, 8);
    lv_obj_align(bar_ruuvi_hum, LV_ALIGN_BOTTOM_LEFT, 5, -20);
    lv_bar_set_range(bar_ruuvi_hum, 0, 100);
    lv_obj_set_style_bg_color(bar_ruuvi_hum, lv_color_make(35, 35, 35), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar_ruuvi_hum, lv_color_make(41, 128, 185), LV_PART_INDICATOR); // Blå fuktindikator

    lbl_ruuvi_hum = lv_label_create(ruuvi_box);
    lv_label_set_text(lbl_ruuvi_hum, "--%");
    lv_obj_set_style_text_font(lbl_ruuvi_hum, &lv_font_montserrat_10, 0);
    lv_obj_align(lbl_ruuvi_hum, LV_ALIGN_BOTTOM_RIGHT, -5, -22);


    // =================================================================
    // SENSOR 2: XIAOMI MIJIA (HÖGER SIDA)
    // =================================================================
    lv_obj_t *xiaomi_box = lv_obj_create(parent);
    lv_obj_set_size(xiaomi_box, 180, 240);
    lv_obj_align(xiaomi_box, LV_ALIGN_RIGHT_MID, -15, 10);
    lv_obj_set_style_bg_color(xiaomi_box, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_color(xiaomi_box, lv_color_make(40, 40, 40), 0);
    lv_obj_set_style_border_width(xiaomi_box, 1, 0);
    lv_obj_clear_flag(xiaomi_box, LV_OBJ_FLAG_SCROLLABLE);

    // Sensornamn / Plats
    lv_obj_t *lbl_xiaomi_title = lv_label_create(xiaomi_box);
    lv_label_set_text(lbl_xiaomi_title, mijia.cfg.name.length() > 0 ? mijia.cfg.name.c_str() : "Xiaomi Mijia");
    lv_obj_set_style_text_font(lbl_xiaomi_title, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_xiaomi_title, LV_ALIGN_TOP_MID, 0, 5);

    // Cirkulär Temperaturmätare
    xiaomi_temp_arc = lv_arc_create(xiaomi_box);
    lv_obj_set_size(xiaomi_temp_arc, 100, 100);
    lv_obj_align(xiaomi_temp_arc, LV_ALIGN_TOP_MID, 0, 35);
    lv_arc_set_range(xiaomi_temp_arc, -20, 60);
    lv_arc_set_bg_angles(xiaomi_temp_arc, 130, 50);
    lv_arc_set_value(xiaomi_temp_arc, 20);
    lv_obj_remove_style(xiaomi_temp_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(xiaomi_temp_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(xiaomi_temp_arc, lv_color_make(35, 35, 35), LV_PART_MAIN);
    lv_obj_set_style_arc_width(xiaomi_temp_arc, 8, LV_PART_ANY);

    // Temperaturvärde
    lbl_xiaomi_temp = lv_label_create(xiaomi_box);
    lv_label_set_text(lbl_xiaomi_temp, "--.-°C");
    lv_obj_set_style_text_font(lbl_xiaomi_temp, &lv_font_montserrat_16, 0);
    lv_obj_align_to(lbl_xiaomi_temp, xiaomi_temp_arc, LV_ALIGN_CENTER, 0, 0);

    // Luftfuktighetsmätare 
    lv_obj_t *lbl_xiaomi_hum_title = lv_label_create(xiaomi_box);
    lv_label_set_text(lbl_xiaomi_hum_title, "Luftfuktighet:");
    lv_obj_set_style_text_font(lbl_xiaomi_hum_title, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_xiaomi_hum_title, lv_color_make(130, 130, 130), 0);
    lv_obj_align(lbl_xiaomi_hum_title, LV_ALIGN_BOTTOM_LEFT, 5, -35);

    bar_xiaomi_hum = lv_bar_create(xiaomi_box);
    lv_obj_set_size(bar_xiaomi_hum, 110, 8);
    lv_obj_align(bar_xiaomi_hum, LV_ALIGN_BOTTOM_LEFT, 5, -20);
    lv_bar_set_range(bar_xiaomi_hum, 0, 100);
    lv_obj_set_style_bg_color(bar_xiaomi_hum, lv_color_make(35, 35, 35), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar_xiaomi_hum, lv_color_make(41, 128, 185), LV_PART_INDICATOR);

    lbl_xiaomi_hum = lv_label_create(xiaomi_box);
    lv_label_set_text(lbl_xiaomi_hum, "--%");
    lv_obj_set_style_text_font(lbl_xiaomi_hum, &lv_font_montserrat_10, 0);
    lv_obj_align(lbl_xiaomi_hum, LV_ALIGN_BOTTOM_RIGHT, -5, -22);
}

// --- Trådsäker Datamottagning för Miljösidan (Körs i grafiktråden under Mutex) ---
void ui_update_environment_data() {
    // Om vi inte har initierat objekten (sidan är inte synlig/skapad), hoppa ur direkt
    if (ruuvi_temp_arc == nullptr || xiaomi_temp_arc == nullptr) return;

    if (lvgl_mutex != NULL && xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(15)) == pdTRUE) {
        
        // 1. Uppdatera RuuviTag-mätare live
        if (ruuvi.cfg.enabled) {
            lv_arc_set_value(ruuvi_temp_arc, (int)ruuvi.temperature);
            lv_label_set_text_fmt(lbl_ruuvi_temp, "%.1f°C", ruuvi.temperature);
            lv_bar_set_value(bar_ruuvi_hum, (int)ruuvi.humidity, LV_ANIM_ON);
            lv_label_set_text_fmt(lbl_ruuvi_hum, "%.0f%%", ruuvi.humidity);

            // Sätt dynamisk färg baserat på temperatur
            lv_color_t color = get_temp_color(ruuvi.temperature);
            lv_obj_set_style_arc_color(ruuvi_temp_arc, color, LV_PART_INDICATOR);
            lv_obj_set_style_text_color(lbl_ruuvi_temp, color, 0);
        }

        // 2. Uppdatera Xiaomi Mijia-mätare live
        if (mijia.cfg.enabled) {
            lv_arc_set_value(xiaomi_temp_arc, (int)mijia.temperature);
            lv_label_set_text_fmt(lbl_xiaomi_temp, "%.1f°C", mijia.temperature);
            lv_bar_set_value(bar_xiaomi_hum, (int)mijia.humidity, LV_ANIM_ON);
            lv_label_set_text_fmt(lbl_xiaomi_hum, "%.0f%%", mijia.humidity);

            // Sätt dynamisk färg baserat på temperatur
            lv_color_t color = get_temp_color(mijia.temperature);
            lv_obj_set_style_arc_color(xiaomi_temp_arc, color, LV_PART_INDICATOR);
            lv_obj_set_style_text_color(lbl_xiaomi_temp, color, 0);
        }

        xSemaphoreGive(lvgl_mutex);
    }
}
