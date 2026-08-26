#include "config.h"
#include "ui_manager.h"
#include "ble_manager.h"
#include "network_manager.h"
#include "elegoo_relay_manager.h"
#include "storage_manager.h"

// Instansiera globala variabler
SemaphoreHandle_t lvgl_mutex = NULL;
TwoWire InternalI2C = TwoWire(0);
TwoWire ExternalI2C = TwoWire(1);

VictronDevice shunt, mppt, ip22;
RuuviTagDevice ruuvi;
XiaomiMijiaDevice mijia;
ShellyDevice shellyPro1, shellyPro2;
ElegooRelaySystem elegoo;

String wifi_ssid = "";
String wifi_pass = "";
int update_interval = 1000;
int display_brightness = 255;

// Globala LVGL-objektallokeringar
lv_obj_t * main_keyboard = nullptr;
lv_obj_t * lbl_footer_clock = nullptr;
lv_obj_t * btn_hamburger = nullptr;
lv_obj_t * page_settings_container = nullptr; 
lv_obj_t * page_overview_container = nullptr; 

// --- ASYNKRON TASK: KLOCKEDRIVEN RELÄAUTOMATION (Core 0) ---
void clockSchedulingTask(void *pvParameters) {
    // Ställ in svensk tidszon med automatisk sommar/vintertid
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
    Serial.println("[Skedulering] Klocka och tidsstyrning startad.");

    while(1) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("[Klocka] Väntar på synkronisering mot NTP...");
            vTaskDelay(pdMS_TO_TICKS(5000)); 
            continue;
        }

        int currentDayOfWeek = timeinfo.tm_wday - 1;
        if (currentDayOfWeek < 0) currentDayOfWeek = 6; // Söndag = 6

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
        
        vTaskDelay(pdMS_TO_TICKS(60000)); // Körs en gång per minut
    }
}

// --- ASYNKRON TASK: LVGL RENDERING (Core 1) ---
void lvglRenderTask(void *pvParameters) {
  while(1) {
    if (lvgl_mutex != NULL) {
      if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        // lv_timer_handler(); // Ersätt med din faktiska LVGL-handler loop
        xSemaphoreGive(lvgl_mutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5)); // Mjuk touch och grafik i ~200Hz
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  lvgl_mutex = xSemaphoreCreateMutex();
  if (lvgl_mutex == NULL) while(1);

  // Initiera hårdvarubussarna separat
  InternalI2C.begin(INTERNAL_SDA, INTERNAL_SCL, INTERNAL_I2C_FREQ);
  ExternalI2C.begin(EXTERNAL_SDA, EXTERNAL_SCL, EXTERNAL_I2C_FREQ);

  // Ladda inställningar och reläscheman permanent från NVS
  loadScheduleFromNVS();

  xTaskCreatePinnedToCore(lvglRenderTask, "LVGL_Render", 8192, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(clockSchedulingTask, "Clock_Schedule", 4096, NULL, 1, NULL, 0);

  ble_manager_init();
  // network_manager_init();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
