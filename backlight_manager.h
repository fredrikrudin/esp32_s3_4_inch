#ifndef BACKLIGHT_MANAGER_H
#define BACKLIGHT_MANAGER_H

#include <Arduino.h>
#include <ESP32_IO_Expander.h> // Krävs för Waveshare v4:s interna expander
#include "config.h"

// Inställningar för hårdvaru-PWM (Används på v1, v2 och v3)
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8; // 0-255 ljusstyrka
const int LEGACY_BL_PIN = 4;   // Bakgrundsbelysningens GPIO på v1-v3

// Global indikator för om vi kör på ett v4-kort (detekteras i init)
bool isHardwareRevisionV4 = false;
ESP32_IO_Expander *internalExpander = nullptr;

/**
 * Initierar bakgrundsbelysningen baserat på kortets revision.
 * Funktionen känner automatiskt av om det är ett v4-kort med v4-expander.
 */
void initBacklight() {
    Serial.println("[Backlight] Initierar ljusstyrkesystem...");

    // 1. Försök att ansluta till Waveshares interna expander (TCA9554 på adress 0x20 eller 0x3C)
    // Den interna expandern ligger på de interna I2C-pinnarna (SDA: 8, SCL: 9)
    Wire.begin(8, 9); 
    
    // Skapa en instans för den inbyggda expandern (TCA9554)
    internalExpander = new ESP32_IO_Expander_TCA9554(Wire, 0x20);
    
    if (internalExpander && internalExpander->begin()) {
        Serial.println("[Backlight] Hårdvarurevision v4 detekterad (Intern expander hittad)!");
        isHardwareRevisionV4 = true;

        // På v4-kort styrs belysningsaktiveringen via EXIO1 (BL_EN) och PWM via EXIO4 (BLC)
        internalExpander->pinMode(1, OUTPUT); // EXIO1: BL_EN
        internalExpander->pinMode(4, OUTPUT); // EXIO4: BLC
        
        // Aktivera belysnings-enheten (Sätt BL_EN till HIGH)
        internalExpander->digitalWrite(1, HIGH);
    } else {
        // Om expandern inte hittas kör vi äldre hårdvarustyrning (v1-v3)
        Serial.println("[Backlight] Hårdvarurevision v1-v3 detekterad (Kör direkt GPIO-PWM).");
        isHardwareRevisionV4 = false;
        
        // Konfigurera ESP32-S3:s inbyggda LEDC/PWM-motor på GPIO 4
        ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(LEGACY_BL_PIN, PWM_CHANNEL);
    }

    // Sätt skärmens startljusstyrka från din globala config-variabel
    setBacklightBrightness(display_brightness);
}

/**
 * Justerar skärmens ljusstyrka i realtid (0 - 255).
 * Denna funktion anropas sömlöst av dina LVGL-skjutreglage (sliders) och gester.
 */
void setBacklightBrightness(int brightness) {
    // Begränsa värdet mellan 0 (av) och 255 (max ljusstyrka)
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;

    // Spara det nya värdet i den globala strukturen
    display_brightness = brightness;

    if (isHardwareRevisionV4) {
        // Revision v4: Styr belysningen genom att slå av/på eller emulera via expandern
        // Om ljusstyrkan är satt till 0, stäng av BL_EN helt för att spara ström på v4
        if (brightness == 0) {
            internalExpander->digitalWrite(1, LOW);  // BL_EN = LOW
            internalExpander->digitalWrite(4, LOW);  // BLC = LOW
        } else {
            internalExpander->digitalWrite(1, HIGH); // BL_EN = HIGH
            
            // v4-expandern stöder ren digital av/på, eller mjukvaru-PWM om Waveshares bibliotek tillåter det.
            // För enkel och stabil kompabilitet sätter vi pinnen HIGH om ljusstyrkan är över 50%.
            internalExpander->digitalWrite(4, brightness > 20 ? HIGH : LOW); 
        }
    } else {
        // Revision v1-v3: Skriv direkt till ESP32-S3 hårdvaru-PWM
        ledcWrite(PWM_CHANNEL, brightness);
    }
}

/**
 * Enkel funktion för att snabbt tona in/ut skärmen vid rörelse eller gester.
 */
void fadeBacklight(int targetBrightness, int durationMs) {
    int startBrightness = display_brightness;
    int steps = 20;
    int stepTime = durationMs / steps;
    float brightnessStep = (float)(targetBrightness - startBrightness) / steps;

    for (int i = 0; i <= steps; i++) {
        int nextBrightness = startBrightness + (brightnessStep * i);
        setBacklightBrightness(nextBrightness);
        delay(stepTime);
    }
}

#endif // BACKLIGHT_MANAGER_H
