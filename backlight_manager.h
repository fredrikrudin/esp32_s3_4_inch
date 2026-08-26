#ifndef BACKLIGHT_MANAGER_H
#define BACKLIGHT_MANAGER_H

#include <Arduino.h>
#include <ESP32_IO_Expander.h> 
#include "config.h"

const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8; 
const int LEGACY_BL_PIN = 4;   

bool isHardwareRevisionV4 = false;
ESP32_IO_Expander *internalExpander = nullptr;

void setBacklightBrightness(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    display_brightness = brightness;

    if (isHardwareRevisionV4) {
        if (brightness == 0) {
            internalExpander->digitalWrite(1, LOW);  
            internalExpander->digitalWrite(4, LOW);  
        } else {
            internalExpander->digitalWrite(1, HIGH); 
            internalExpander->digitalWrite(4, brightness > 20 ? HIGH : LOW); 
        }
    } else {
        ledcWrite(PWM_CHANNEL, brightness);
    }
}

void initBacklight() {
    Serial.println("[Backlight] Initierar ljusstyrkesystem...");
    Wire.begin(8, 9); 
    
    internalExpander = new ESP32_IO_Expander_TCA9554(Wire, 0x20);
    
    if (internalExpander && internalExpander->begin()) {
        Serial.println("[Backlight] Hårdvarurevision v4 detekterad!");
        isHardwareRevisionV4 = true;
        internalExpander->pinMode(1, OUTPUT); 
        internalExpander->pinMode(4, OUTPUT); 
        internalExpander->digitalWrite(1, HIGH);
    } else {
        Serial.println("[Backlight] Hårdvarurevision v1-v3 detekterad.");
        isHardwareRevisionV4 = false;
        ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(LEGACY_BL_PIN, PWM_CHANNEL);
    }
    setBacklightBrightness(display_brightness);
}

#endif // BACKLIGHT_MANAGER_H
