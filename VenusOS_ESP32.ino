#include "config.h"
#include "ui_manager.h"
#include "ble_manager.h"
#include "network_manager.h"
#include "elegoo_relay_manager.h"
#include "storage_manager.h"

// Allokera globala variabler och I2C-bussar
SemaphoreHandle_t lvgl_mutex = NULL;
TwoWire InternalI2C = TwoWire(0);
TwoWire ExternalI2C = TwoWire(1);

// Allokera dina systemstrukturer
VictronDevice shunt, mppt, ip22;
RuuviTagDevice ruuvi;
XiaomiMijiaDevice mijia;
ShellyDevice shellyPro1, shellyPro2;
ElegooRelaySystem elegoo;

String wifi_ssid = "";
String wifi_pass = "";
int update_interval = 1000;
int display_brightness = 255;

// Allokera globala LVGL-pekare
lv_obj_t * main_keyboard = nullptr;
lv_obj_t * lbl_footer_clock = nullptr;
lv_obj_t * btn_hamburger = nullptr;
lv_obj_t * page_settings_container = nullptr; 
lv_obj_t * page_overview_container = nullptr; 

// --- ASYNKRON TASK 1: TID- OCH DATUMAUTOMATION (Core 0) ---
void clockSchedulingTask(void *pvParameters) {
    // Konfigurera svensk lokaltid med automatisk sommar/vintertid
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
    Serial.println("[Skedulering] Klocka och tidsstyrning startad på Core 0.");

    while(1) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("[Klocka] Synkroniserar mot NTP-server...");
            vTaskDelay(pdMS_TO_TICKS(5000)); 
            continue;
        }

        // Konvertera tm_wday (0=Sön, 1=Mån...) till vårt format (0=Mån, 6=Sön)
        int currentDayOfWeek = timeinfo.tm_wday - 1;
        if (currentDayOfWeek < 0) currentDayOfWeek = 6; 

        int nowInMinutes = (timeinfo.tm_hour * 60) + timeinfo.tm_min;

        // 1. Skedulera 4-kanalssystemet
        if (elegoo.enabled_4ch) {
            for (int i = 0; i < 4; i++) {
                if (!elegoo.schedule4[i].isEnabled) continue;
                if (elegoo.schedule4[i].days[currentDayOfWeek]) {
                    int start = (elegoo.schedule4[i].startHour * 60) + elegoo.schedule4[i].startMinute;
                    int end = (elegoo.schedule4[i].endHour * 60) + elegoo.schedule4[i].endMinute;

                    bool shouldBeOn = (start < end) ? (nowInMinutes >= start && nowInMinutes < end) 
                                                    : (nowInMinutes >= start || nowInMinutes < end);
                    setExternalRelay(RELAY_4CH_PINS[i], shouldBeOn);
                }
            }
        }

        // 2. Skedulera 8-kanalssystemet
        if (elegoo.enabled_8ch) {
            for (int i = 0; i < 8; i++) {
                if (!elegoo.schedule8[i].isEnabled) continue;
                if (elegoo.schedule8[i].days[currentDayOfWeek]) {
                    int start = (elegoo.schedule8[i].startHour * 60) + elegoo.schedule8[i].startMinute;
                    int end = (elegoo.schedule8[i].endHour * 60) + elegoo.schedule8[i].endMinute;

                    bool shouldBeOn = (start < end) ? (nowInMinutes >= start && nowInMinutes < end) 
                                                    : (nowInMinutes >= start || nowInMinutes < end);
                    setExternalRelay(RELAY_8CH_PINS[i], shouldBeOn);
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(60000)); // Utvärdera en gång i minuten
    }
}

// --- ASYNKRON TASK 2: LVGL RENDERINGS-MOTOR (Core 1) ---
void lvglRenderTask(void *pvParameters) {
  while(1) {
    if (lvgl_mutex != NULL) {
      if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        // lv_timer_handler(); // Ersätt med din faktiska LVGL-handler anrop om den ligger i cpp
        xSemaphoreGive(lvgl_mutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5)); // Mjuk grafik och touch (200Hz)
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Startar VenusOS Moduler...");

  // Skapa skyddsmutex
  lvgl_mutex = xSemaphoreCreateMutex();
  if (lvgl_mutex == NULL) while(1);

  // Starta I2C-bussar oberoende av varandra
  InternalI2C.begin(INTERNAL_SDA, INTERNAL_SCL, INTERNAL_I2C_FREQ);
  ExternalI2C.begin(EXTERNAL_SDA, EXTERNAL_SCL, EXTERNAL_I2C_FREQ);

  // Ladda data från NVS-blixtminnet
  loadScheduleFromNVS();

  // Skapa asynkrona mjukvarutasks i FreeRTOS-skeduleraren
  xTaskCreatePinnedToCore(lvglRenderTask, "LVGL_Render", 8192, NULL, 3, NULL, 1); // Core 1 till UI
  xTaskCreatePinnedToCore(clockSchedulingTask, "Clock_Schedule", 4096, NULL, 1, NULL, 0); // Core 0 till Klocka

  ble_manager_init();
  // network_manager_init();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
