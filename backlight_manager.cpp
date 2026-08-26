#include "backlight_manager.h"
#include <Wire.h>
#include <ESP32_IO_Expander_Library.h> // Krävs för Rev 4 (TCA9554)

#define I2C_SDA_INTERNAL 8
#define I2C_SCL_INTERNAL 9
#define TCA9554_ADDRESS  0x20 // Standard I2C-adress för expandern

ESP32_IO_Expander *expander = nullptr;

void initBacklight() {
    // Initiera den interna I2C-bussen (Buss 0) i 400kHz för skärm/touch/expanders
    Wire.begin(I2C_SDA_INTERNAL, I2C_SCL_INTERNAL, 400000);

    Serial.println("[Backlight] Initierar TCA9554 expander för Rev 4...");
    
    // Skapa och konfigurera IO-expandern för Rev 4
    expander = new ESP32_IO_Expander_TCA9554(Wire, TCA9554_ADDRESS);
    
    if (expander != nullptr) {
        expander->begin();
        // Sätt pinne 2 (typisk för Waveshare backlight-aktivering via expander) till OUTPUT
        expander->pinMode(IO_EXPANDER_PIN_NUM_2, OUTPUT);
        expander->digitalWrite(IO_EXPANDER_PIN_NUM_2, HIGH); // Slå på bakgrundsbelysning
        Serial.println("[Backlight] Rev 4 Bakgrundsbelysning AKTIVERAD.");
    } else {
        Serial.println("[ERROR] Misslyckades att initiera IO-expander!");
    }
}

void setBacklightBrightness(uint8_t brightness) {
    // Grundläggande on/off via expandern för Rev 4 (kan expanderas till PWM om Waveshares hårdvara stöder det på den pinnen)
    if (expander != nullptr) {
        if (brightness > 0) {
            expander->digitalWrite(IO_EXPANDER_PIN_NUM_2, HIGH);
        } else {
            expander->digitalWrite(IO_EXPANDER_PIN_NUM_2, LOW);
        }
    }
}
