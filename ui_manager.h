#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "config.h"
#include "storage_manager.h"

// Deklaration av det globala tangentbordet
lv_obj_t * main_keyboard = nullptr;

// En struktur för att skicka med rätt kontext till tangentbords-callbacks
struct UiEventContext {
    DeviceConfig *cfg;
    VictronDevice *victronDev; // Sätts till nullptr om det inte är en Victron-enhet
    bool isKeyField;           // True om fältet redigerar AES-nyckeln, annars False (namnfält)
};

// Event handler för när användaren klickar i eller sparar data i en textruta
static void ta_event_handler(lv_event_t * e) {
    lv_obj_t * ta = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    UiEventContext *ctx = (UiEventContext*)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        if (main_keyboard == nullptr) {
            // Skapa tangentbordet längst ner på skärmen
            main_keyboard = lv_keyboard_create(lv_scr_act());
            lv_obj_set_size(main_keyboard, LV_PCT(100), LV_PCT(45));
            lv_obj_align(main_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        // Koppla tangentbordet till den klickade textrutan
        lv_keyboard_set_textarea(main_keyboard, ta);
        
        // Om det är nyckelfältet kan vi ställa in tangentbordet i Hex/Siffer-läge för enklare inmatning
        if (ctx->isKeyField) {
            lv_keyboard_set_mode(main_keyboard, LV_KEYBOARD_MODE_NUMBER);
        } else {
            lv_keyboard_set_mode(main_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        }
    }
    
    if (code == LV_EVENT_READY) {
        // Användaren klickade på Enter/Bock-knappen på skärmen
        if (ctx->isKeyField && ctx->victronDev != nullptr) {
            // Spara krypteringsnyckeln
            ctx->victronDev->key = lv_textarea_get_text(ta);
            Serial.printf("[UI] Ny AES-nyckel sparad för %s: %s\n", ctx->victronDev->cfg.name.c_str(), ctx->victronDev->key.c_str());
        } else if (ctx->cfg != nullptr) {
            // Spara enhetsnamnet
            ctx->cfg->name = lv_textarea_get_text(ta);
            Serial.printf("[UI] Nytt namn sparats för %s\n", ctx->cfg->name.c_str());
        }
        
        // Spara ändringarna direkt till det interna NVS Flash-minnet
        saveAllSettings();
        
        // Stäng och ta bort tangentbordet från RAM
        if (main_keyboard != nullptr) {
            lv_obj_del(main_keyboard);
            main_keyboard = nullptr;
        }
    }
}

// Event handler för när användaren slår på/av en enhet via en switch
static void switch_event_handler(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    DeviceConfig *cfg = (DeviceConfig*)lv_event_get_user_data(e);
    
    cfg->enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    
    // Spara ändringen direkt till flash-minnet
    saveAllSettings();
}

/**
 * Genererar en enhetsrad med en switch, namngivningsbar textruta 
 * samt ett dolt/extra fält för AES-nyckel om enheten är från Victron.
 */
void add_device_to_settings_list(lv_obj_t * list, DeviceConfig &cfg, const char* labelText, VictronDevice *victronDev = nullptr) {
    // Lägg till en sektionsrubrik i listan
    lv_list_add_text(list, labelText);

    // Beräkna radhöjden dynamiskt: Victron behöver mer plats (85px) för att husera nyckelfältet
    int row_height = (victronDev != nullptr) ? 95 : 55;

    // Skapa en rad/behållare för kontrollerna
    lv_obj_t * row = lv_obj_create(list);
    lv_obj_set_size(row, LV_PCT(100), row_height);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(row, 5, 0);

    // 1. Skapa en Switch för på/av (Vänsterjusterad överst)
    lv_obj_t * sw = lv_switch_create(row);
    lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 5, 5);
    if (cfg.enabled) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_VALUE_CHANGED, &cfg);

    // 2. Skapa en Textarea för NAMNET (Högerjusterad överst)
    lv_obj_t * ta_name = lv_textarea_create(row);
    lv_obj_set_size(ta_name, 180, 40);
    lv_obj_align(ta_name, LV_ALIGN_TOP_RIGHT, -5, 0);
    lv_textarea_set_one_line(ta_name, true);
    lv_textarea_set_text(ta_name, cfg.name.c_str());
    
    // Bygg kontext för namn-fältet
    UiEventContext *nameCtx = new UiEventContext { &cfg, victronDev, false };
    lv_obj_add_event_cb(ta_name, ta_event_handler, LV_EVENT_ALL, nameCtx);

    // 3. Om det är en Victron-enhet -> Skapa raden och textfältet för AES-Krypteringsnyckeln
    if (victronDev != nullptr) {
        // Skapa en liten ledtext för nyckeln
        lv_obj_t * lbl_key = lv_label_create(row);
        lv_label_set_text(lbl_key, "Kryptonyckel (HEX):");
        lv_obj_align(lbl_key, LV_ALIGN_BOTTOM_LEFT, 5, -12);
        lv_obj_set_style_text_font(lbl_key, &lv_font_montserrat_12, 0); // Mindre text för etiketten

        // Skapa textfältet för nyckeln (Placeras under namnfältet till höger)
        lv_obj_t * ta_key = lv_textarea_create(row);
        lv_obj_set_size(ta_key, 240, 40);
        lv_obj_align(ta_key, LV_ALIGN_BOTTOM_RIGHT, -5, 0);
        lv_textarea_set_one_line(ta_key, true);
        lv_textarea_set_max_length(ta_key, 32); // Max 32 tecken för en AES-128 Hex-nyckel
        lv_textarea_set_text(ta_key, victronDev->key.c_str());
        
        // Bygg kontext för nyckel-fältet
        UiEventContext *keyCtx = new UiEventContext { &cfg, victronDev, true };
        lv_obj_add_event_cb(ta_key, ta_event_handler, LV_EVENT_ALL, keyCtx);
    }
}

/**
 * Skapar inställningssidan för enhetshantering.
 * Anropas från menyknapparna i ditt befintliga h-filssystem.
 */
void create_settings_page(lv_obj_t * parent) {
    // Skapa en rullningsbar lista centrerad på skärmen
    lv_obj_t * list = lv_list_create(parent);
    lv_obj_set_size(list, 450, 390);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);

    // Lägg till Victron-enheterna med tillhörande krypteringsinstanser skickade som sista argument
    add_device_to_settings_list(list, shunt.cfg, "Victron SmartShunt", &shunt);
    add_device_to_settings_list(list, mppt.cfg, "Victron MPPT Solceller", &mppt);
    add_device_to_settings_list(list, ip22.cfg, "Victron IP22 Laddare", &ip22);
    
    // Lägg till standardenheter (utan krypteringsfält)
    add_device_to_settings_list(list, ruuvi.cfg, "RuuviTag Sensor");
    add_device_to_settings_list(list, mijia.cfg, "Xiaomi Mijia Sensor");
    add_device_to_settings_list(list, shellyPro1.cfg, "Shelly Pro 1 (1-Kanal)");
    add_device_to_settings_list(list, shellyPro2.cfg, "Shelly Pro 2 (2-Kanal)");
}

void initDisplayAndUI() {
    // Din befintliga kod för initiering av Waveshare-skärmdrivrutiner (ST7701/GT911)
    // ...
}

#endif // UI_MANAGER_H
