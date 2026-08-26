#include "ui_manager.h"
#include <lvgl.h>
#include <Arduino.h>

// Definiera globala trådsäkra variabler och Mutex
SemaphoreHandle_t lvgl_mutex = NULL;
TaskHandle_t uiTaskHandle = NULL;

// Deklarera externa LVGL-initieringsfunktioner (skapas av användaren eller LVGL-generator)
extern void ui_init(void); 

// FreeRTOS Task som körs exklusivt på Core 1 i ~200Hz (5ms fördröjning = 200Hz)
void ui_task(void *pvParameters) {
    Serial.println("[UI] Startar LVGL grafik-task på Core 1...");
    
    while (1) {
        // Ta mutexen innan LVGL-uppdatering körs
        if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            lv_timer_handler(); // Hantera grafik och touch-event
            xSemaphoreGive(lvgl_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // ~200Hz uppdateringsfrekvens
    }
}

void initUI() {
    // 1. Skapa Mutex för trådsäker dataöverföring mellan kärnorna
    lvgl_mutex = xSemaphoreCreateMutex();
    if (lvgl_mutex == NULL) {
        Serial.println("[ERROR] Kunde inte skapa LVGL-mutex!");
        return;
    }

    // 2. Initiera LVGL-biblioteket
    lv_init();

    // 3. Initiera display- och touch-drivrutiner (använder inställningar från display_config.h)
    // OBS: Denna del anpassas efter exakt drivrutin (t.ex. ST7701S / GT911 på Waveshare)
    Serial.println("[UI] Skärm- och touchdrivrutiner initierade.");

    // 4. Starta Fredriks VenusOS GUI v2 vyer
    ui_init(); 

    // 5. Lås fast grafik-tasken på Core 1 (Kärnisolerad skedulering)
    xTaskCreatePinnedToCore(
        ui_task,          // Funktion
        "LVGL_Task",      // Namn
        8192,             // Stackstorlek (8KB inställt för OPI PSRAM-stabilitet)
        NULL,             // Parametrar
        3,                // Prioritet
        &uiTaskHandle,    // Task handle
        1                 // Kärna 1
    );
}
