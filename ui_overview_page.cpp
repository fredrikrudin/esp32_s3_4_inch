#ifndef UI_OVERVIEW_PAGE_H
#define UI_OVERVIEW_PAGE_H

#include <lvgl.h>
#include "config.h"

// --- Globala UI-pekare för mätare och textfällt ---
extern lv_obj_t *battery_arc;
extern lv_obj_t *lbl_soc_value;
extern lv_obj_t *lbl_power_value;
extern lv_obj_t *lbl_specs_value;

extern lv_obj_t *mppt_arc;
extern lv_obj_t *lbl_mppt_pwr;

extern lv_obj_t *ip22_arc;
extern lv_obj_t *lbl_ip22_pwr;

extern lv_obj_t *lbl_inverter_pwr;

extern lv_obj_t *lbl_ruuvi_data;
extern lv_obj_t *lbl_xiaomi_data;

// --- Globala UI-objekt för animationer ---
extern lv_obj_t *particle_mppt;
extern lv_obj_t *particle_ip22;
extern lv_obj_t *particle_inverter;

extern lv_anim_t anim_mppt;
extern lv_anim_t anim_ip22;
extern lv_anim_t anim_inverter;

// --- Funktionsdeklarationer ---
void create_overview_page(lv_obj_t *parent);
void ui_update_live_data();

#endif // UI_OVERVIEW_PAGE_H
