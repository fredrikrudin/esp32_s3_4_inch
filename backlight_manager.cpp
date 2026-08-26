#include "backlight_manager.h"
#include <Wire.h>
#include <ESP32_IO_Expander_Library.h> // Officiella biblioteket för TCA9554

#define I2C_SDA_INTERNAL 8
#define I2C_SCL_INTERNAL 9
#define TCA9554_ADDRESS  0x20 
#define BACKLIGHT_PIN    2    // Waveshare hårdvaruspecifik pinne på expandern

// Korrekt klassinstansiering enligt Espressifs bibliotek
ESP32_IO_Expander *expander = nullptr;

void initBacklight() {
    Wire.begin(I2C_SDA_INTERNAL, I2C_SCL_INTERNAL, 400000);
    Serial.println("[Backlight] Initierar TCA9554 expander för Rev 4...");
    
    // Initiera TCA9554-modulen korrekt
    expander = new ESP32_IO_Expander_TCA9554(&Wire, TCA9554_ADDRESS);
    
    if (expander != nullptr) {
        expander->begin();
        // Använd vanliga pin-nummer (0-7) istället för trasiga makron
        expander->pinMode(BACKLIGHT_PIN, OUTPUT);
        expander->digitalWrite(BACKLIGHT_PIN, HIGH); // Slå på ljuset!
        Serial.println("[Backlight] Rev 4 Bakgrundsbelysning AKTIVERAD.");
    } else {
        Serial.println("[ERROR] Misslyckades att initiera IO-expander!");
    }
}

void setBacklightBrightness(uint8_t brightness) {
    if (expander != nullptr) {
        if (brightness > 0) {
            expander->digitalWrite(BACKLIGHT_PIN, HIGH);
        } else {
            expander->digitalWrite(BACKLIGHT_PIN, LOW);
        }
    }
}
