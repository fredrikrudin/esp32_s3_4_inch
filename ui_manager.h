#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "network_manager.h"
#include "backlight_manager.h"

void keyboard_textarea_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e); lv_obj_t * ta = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) { lv_keyboard_set_textarea(main_keyboard, ta); lv_obj_clear_flag(main_keyboard, LV_OBJ_FLAG_HIDDEN); }
}
static void keyboard_ready_event_cb(lv_event_t * e) { lv_obj_add_flag(main_keyboard, LV_OBJ_FLAG_HIDDEN); }
static void hide_brightness_cb(lv_timer_t * timer) { lv_obj_add_flag(cap_brightness, LV_OBJ_FLAG_HIDDEN); if (brightness_timer) { lv_timer_del(brightness_timer); brightness_timer = NULL; } }
static void brightness_slider_cb(lv_event_t * e) { set_backlight_brightness(lv_slider_get_value(lv_event_get_target(e))); if (brightness_timer) lv_timer_reset(brightness_timer); }

static void global_gesture_cb(lv_event_t * e) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_TOP || dir == LV_DIR_BOTTOM) {
        lv_obj_clear_flag(cap_brightness, LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(cap_brightness);
        lv_slider_set_value(slider_brightness, display_brightness, LV_ANIM_OFF);
        if (brightness_timer) lv_timer_reset(brightness_timer); else brightness_timer = lv_timer_create(hide_brightness_cb, 3000, NULL);
    }
}

void create_v2_relay_control_row(lv_obj_t * parent, String label_text, long id, bool is_on, bool sch_act, int on_h, int off_h) {
    lv_obj_t * cap = lv_obj_create(parent); lv_obj_set_style_radius(cap, 18, 0);
    lv_obj_set_style_bg_color(cap, lv_color_hex(0x181A1F), 0); lv_obj_set_style_border_color(cap, lv_color_hex(0x282C34), 0);

    lv_obj_t * lbl = lv_label_create(cap); lv_label_set_text(lbl, label_text.c_str()); lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 5, 2);
    lv_obj_t * sw = lv_switch_create(cap); lv_obj_set_size(sw, 45, 20); lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -5, 0);
    if (is_on) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x2196F3), LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_t * ta_on = lv_textarea_create(cap); lv_obj_set_size(ta_on, 40, 28); lv_obj_align(ta_on, LV_ALIGN_BOTTOM_RIGHT, -120, -2);
    lv_textarea_set_text(ta_on, String(on_h).c_str()); lv_obj_add_event_cb(ta_on, keyboard_textarea_event_cb, LV_EVENT_ALL, NULL);
}

void build_gui_v2() {
    lv_obj_t * status_bar = lv_obj_create(lv_scr_act()); lv_obj_set_size(status_bar, 480, 35);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);

    lbl_clock = lv_label_create(status_bar); lv_obj_align(lbl_clock, LV_ALIGN_LEFT_MID, 55, 0);
    lbl_status_temp = lv_label_create(status_bar); lv_obj_align(lbl_status_temp, LV_ALIGN_RIGHT_MID, -10, 0);

    lv_obj_t * container = lv_obj_create(lv_scr_act()); lv_obj_set_size(container, 480, 445); lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x0B0C0E), 0);

    page_shelly = lv_obj_create(container); lv_obj_set_size(page_shelly, 460, 425);
    create_v2_relay_control_row(page_shelly, "Shelly Smart Relas", 99, shelly.current_status, shelly.schedule_active, shelly.on_hour, shelly.off_hour);

    lv_obj_t * gesture_layer = lv_obj_create(lv_scr_act()); lv_obj_set_size(gesture_layer, 480, 480);
    lv_obj_set_style_bg_opa(gesture_layer, LV_OPA_TRANSP, 0); lv_obj_set_style_border_width(gesture_layer, 0, 0);
    lv_obj_add_flag(gesture_layer, LV_OBJ_FLAG_CLICKABLE); lv_obj_add_event_cb(gesture_layer, global_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_move_background(gesture_layer);

    cap_brightness = lv_obj_create(lv_scr_act()); lv_obj_set_size(cap_brightness, 280, 80); lv_obj_align(cap_brightness, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(cap_brightness, 20, 0); lv_obj_set_style_bg_color(cap_brightness, lv_color_hex(0x111215), 0);
    lv_obj_set_style_border_color(cap_brightness, lv_color_hex(0x2196F3), 0); lv_obj_add_flag(cap_brightness, LV_OBJ_FLAG_HIDDEN);

    slider_brightness = lv_slider_create(cap_brightness); lv_obj_set_size(slider_brightness, 220, 12); lv_obj_align(slider_brightness, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_slider_set_range(slider_brightness, 15, 255); lv_obj_add_event_cb(slider_brightness, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    main_keyboard = lv_keyboard_create(lv_scr_act()); lv_obj_set_size(main_keyboard, 480, 180);
    lv_keyboard_set_mode(main_keyboard, LV_KEYBOARD_MODE_NUMBER); lv_obj_add_flag(main_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(main_keyboard, keyboard_ready_event_cb, LV_EVENT_READY, NULL);
}
#endif
