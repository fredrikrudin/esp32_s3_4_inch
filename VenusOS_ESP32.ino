#include "config.h"
#include "ui_manager.h"
#include "ble_manager.h"
#include "network_manager.h"
#include "elegoo_relay_manager.h"
#include "storage_manager.h"

// ==========================================
// 1. ALLOKERING AV GLOBALA SYSTEMRESURSER
// ==========================================
SemaphoreHandle_t lvgl_mutex = NULL;
TwoWire InternalI2C = TwoWire(0);
TwoWire ExternalI2C = TwoWire(1);

// Instansiering av dina enhets- och sensorstrukturer
VictronDevice shunt, mppt, ip22;
RuuviTagDevice ruuvi;
XiaomiMijiaDevice mijia;
ShellyDevice shellyPro1, shellyPro2;
ElegooRelaySystem elegoo;

// Globala systeminställningar
String wifi_ssid = "";
String wifi_pass = "";
int update_interval = 1000;
int display_brightness = 255;
int ui_style_version = 2; // 1 = Klassisk kvadratisk v1, 2 = Nya cirkulära GUI-v2

// Allokering av globala LVGL-gränssnittsobjekt
lv_obj_t * main_keyboard = nullptr;
lv_obj_t * lbl_footer_clock = nullptr;
lv_obj_t * btn_hamburger = nullptr;
lv_obj_t * page_settings_container = nullptr; 
lv_obj_t * page_overview_container = nullptr; 

// Globala objekt för sökmodulen
lv_obj_t * discovery_popup = nullptr;
lv_obj_t * discovery_list = nullptr;
DeviceConfig * active_discovery_target = nullptr;

// ==========================================
// 2. ASYNKRON TASK: KLOCKEDRIVEN RELÄAUTOMATION (Core 0)
// ==========================================
void clockSchedulingTask(void *pvParameters) {
    // Konfigurera svensk lokal tid med automatisk sommar- och vintertid (DST)
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
    Serial.println("[Skedulering] Klocka, NTP och reläautomation startad på Core 0.");

    while(1) {
        struct tm timeinfo;
        
        // Försök hämta lokal tid från ESP32:s interna klocka
        if (!getLocalTime(&timeinfo)) {
            Serial.println("[Klocka] Väntar på tidssynkronisering mot NTP...");
            vTaskDelay(pdMS_TO_TICKS(5000)); 
            continue;
        }

        // Konvertera tm_wday (0=Sön, 1=Mån...) till vårt format (0=Mån, 6=Sön)
        int currentDayOfWeek = timeinfo.tm_wday - 1;
        if (currentDayOfWeek < 0) currentDayOfWeek = 6; 

        // Räkna om aktuell tid till minuter sedan midnatt för enkel utvärdering
        int nowInMinutes = (timeinfo.tm_hour * 60) + timeinfo.tm_min;

        // --- AUTOMATION: 4-KANALSKORT (Elegoo) ---
        if (elegoo.enabled_4ch) {
            for (int i = 0; i < 4; i++) {
                if (!elegoo.schedule4[i].isEnabled) continue;
                
                if (elegoo.schedule4[i].days[currentDayOfWeek]) {
                    int start = (elegoo.schedule4[i].startHour * 60) + elegoo.schedule4[i].startMinute;
                    int end = (elegoo.schedule4[i].endHour * 60) + elegoo.schedule4[i].endMinute;

                    // Hantera normala tidsfönster vs tidsfönster som passerar midnatt
                    bool shouldBeOn = (start < end) ? (nowInMinutes >= start && nowInMinutes < end) 
                                                    : (nowInMinutes >= start || nowInMinutes < end);
                    setExternalRelay(RELAY_4CH_PINS[i], shouldBeOn);
                }
            }
        }

        // --- AUTOMATION: 8-KANALSKORT (Elegoo) ---
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
        
        // Söv uppgiften i 60 sekunder för att helt avlasta processorkärnan
        vTaskDelay(pdMS_TO_TICKS(60000)); 
    }
}

// ==========================================
// 3. ASYNKRON TASK: LVGL RENDERINGS-MOTOR & DATASYNC (Core 1)
// ==========================================
void lvglRenderTask(void *pvParameters) {
  while(1) {
    // 1. Injicera mät- och sensordata trådsäkert till dina LVGL-skärmvyer
    ui_update_live_data();        // Uppdaterar mätbågar för Shunt/Solceller på översikten
    ui_update_environment_data(); // Uppdaterar mätbågar och fuktstaplar på miljösidan

    // 2. Låt LVGL bearbeta animationer, renderingar och pekskärms-touch
    if (lvgl_mutex != NULL) {
      if (xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        // lv_timer_handler(); // Exekvera LVGL-huvudloopen
        xSemaphoreGive(lvgl_mutex);
      }
    }
    
    // Garanterar en uppdateringshastighet på ~200Hz för maximalt flyt vid beröring
    vTaskDelay(pdMS_TO_TICKS(5)); 
  }
}

// ==========================================
// 4. SYSTEMINITIALISERING (Körs en gång vid boot)
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=============================================");
  Serial.println("  STARTAR VENUSOS GUI V2 AUTOMATIONSYSTEM    ");
  Serial.println("=============================================");

  // 1. Skapa Mutex-skydd för att förhindra trådkrockar och flimmer i grafiken
  lvgl_mutex = xSemaphoreCreateMutex();
  if (lvgl_mutex == NULL) {
    Serial.println("[Kritiskt fel] Kunde inte allokera LVGL Mutex!");
    while(1); // Stoppa hårdvaran vid kritiskt synkfel
  }

  // 2. Initiera hårdvarans I2C-bussar (Buss 0 för skärm/touch, Buss 1 för externa reläer)
  InternalI2C.begin(INTERNAL_SDA, INTERNAL_SCL, INTERNAL_I2C_FREQ);
  ExternalI2C.begin(EXTERNAL_SDA, EXTERNAL_SCL, EXTERNAL_I2C_FREQ);
  Serial.println("[Hårdvara] Dubbla I2C-bussar separerade och aktiverade.");

  // 3. Ladda alla inställningar, krypteringsnycklar och reläscheman permanent från Flash (NVS)
  loadAllSettings();

  // 4. Initiera gränssnittet trådsäkert
  if (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
    // ui_init(); // Ersätt med ditt anrop för att rita upp grundlayouten (header/footer/containers)
    
    // Bygg upp rätt startsida baserat på användarens sparade val (ui_style_version)
    if (ui_style_version == 1) {
        // create_classic_overview_page(page_overview_container); // Din gamla vy
    } else {
        create_overview_page(page_overview_container); // Nya cirkulära Victron GUI-v2
    }
    
    xSemaphoreGive(lvgl_mutex);
  }

  // 5. FREE-RTOS SKEDULERING: Registrera och starta bakgrundstrådarna på tilldelade CPU-kärnor
  // Grafikmotor och touchanvändning (Hög prioritet, körs dedikerat på Core 1)
  xTaskCreatePinnedToCore(lvglRenderTask, "LVGL_Render_Task", 8192, NULL, 3, NULL, 1); 
  
  // Tidssynkronisering och reläautomation (Normal prioritet, körs isolerat på Core 0)
  xTaskCreatePinnedToCore(clockSchedulingTask, "Clock_Schedule_Task", 4096, NULL, 1, NULL, 0); 

  // Starta kommunikationsmodulerna (BLE-skanning och Wi-Fi körs oberoende på Core 0)
  ble_manager_init(); 
  // network_manager_init(); 
  
  Serial.println("[System] Samtliga asynkrona FreeRTOS-trådar har startats.");
}

// ==========================================
// 5. ARDUINO STANDARD LOOP (Lämnas tom!)
// ==========================================
void loop() {
  // Eftersom allt styrs via den oberoende FreeRTOS-skeduleringen ovan, lämnas loop() tom.
  // Vi lägger en fast fördröjning här så att bakgrundssystemet (IDLE-task) hinner andas.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
