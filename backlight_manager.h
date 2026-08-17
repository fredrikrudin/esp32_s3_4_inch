#ifndef BACKLIGHT_MANAGER_H
#define BACKLIGHT_MANAGER_H
#include <Arduino.h>
#include "config.h"

void init_backlight() {
    ledcSetup(0, 5000, 8); 
    ledcAttachPin(45, 0);  
    ledcWrite(0, display_brightness);
}
void set_backlight_brightness(int brightness) {
    if (brightness < 15) brightness = 15; 
    display_brightness = brightness;
    ledcWrite(0, display_brightness);
}
#endif
