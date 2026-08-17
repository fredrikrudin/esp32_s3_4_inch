void build_gui_v2() {
    // ... [Befintlig kod för statusrad och skärmcontainer] ...

    // ---- 1. SKAPA EN GEMENSAM ELEGOO-SIDA ----
    page_elegoo_relays = lv_obj_create(container);
    lv_obj_set_size(page_elegoo_relays, 460, 425);
    lv_obj_set_style_bg_color(page_elegoo_relays, lv_color_hex(0x0B0C0E), 0);
    lv_obj_set_style_border_width(page_elegoo_relays, 0, 0);
    lv_obj_add_flag(page_elegoo_relays, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * lbl_title = lv_label_create(page_elegoo_relays);
    lv_label_set_text_fmt(lbl_title, "🎛️ ELEGOO %d-KANALS RELÄKORT", elegoo.elegoo_channels);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_16, 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);

    list_elegoo_relays = lv_obj_create(page_elegoo_relays);
    lv_obj_set_size(list_elegoo_relays, 450, 350);
    lv_obj_align(list_elegoo_relays, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(list_elegoo_relays, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(list_elegoo_relays, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_bg_color(list_elegoo_relays, lv_color_hex(0x0B0C0E), 0);
    lv_obj_set_style_border_width(list_elegoo_relays, 0, 0);

    // Rita endast ut det antal rader som är inställt (4 eller 8)
    for (int i = 0; i < elegoo.elegoo_channels; i++) {
        create_v2_relay_control_row(list_elegoo_relays, "Fysiskt Relä: Utgång " + String(i + 1), i, elegoo.relay_states[i], elegoo.schedules[i].schedule_active, elegoo.schedules[i].on_hour, elegoo.schedules[i].off_hour);
    }

    // ---- 2. DYNAMISK BYGGE AV HAMBURGERMENYN ----
    menu_sidebar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu_sidebar, 220, 445);
    lv_obj_align(menu_sidebar, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(menu_sidebar, lv_color_hex(0x111215), 0);
    lv_obj_set_flex_flow(menu_sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(menu_sidebar, LV_OBJ_FLAG_HIDDEN);

    // Lägg till menyval dynamiskt baserat på om hårdvaran är aktiverad eller inte
    add_menu_item("📊 Startsida", 0);
    add_menu_item("📋 Översiktssida", 1);
    if(shunt.enabled) add_menu_item("🔋 Enhet: SmartShunt", 2);
    if(mppt.enabled)  add_menu_item("☀️ Enhet: MPPT Solcell", 3);
    if(shelly.enabled) add_menu_item("🔌 Enhet: Shelly", 4);
    
    // NYTT: Lägg endast till reläsidan i menyn om kanaler är satt till 4 eller 8!
    if(elegoo.elegoo_channels > 0) {
        add_menu_item("🎛️ Elegoo Reläkort", 5); 
    }
    
    add_menu_item("🔍 Sök Enheter", 6);
    add_menu_item("🛠️ Systemstatus", 7);
}
