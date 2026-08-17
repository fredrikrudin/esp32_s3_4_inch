#ifndef SYSTEM_DIAGNOSTICS_H
#define SYSTEM_DIAGNOSTICS_H

#include <Arduino.h>
#include "config.h"

// Deklarera den inbyggda temperaturfunktionen i ESP32-S3
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

// Returnerar drifttid (Uptime) i läsbart format: DD:HH:MM:SS
String getSystemUptime() {
    unsigned long sec = millis() / 1000;
    unsigned long min = sec / 60;
    unsigned long hr = min / 60;
    unsigned long day = hr / 24;
    
    char buf[32];
    sprintf(buf, "%ldd %02ld:%02ld:%02ld", day, hr % 24, min % 60, sec % 60);
    return String(buf);
}

// Läser av ESP32-S3:ans interna chip-temperatur
float getESP32Temperature() {
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
        return (temprature_sens_read() - 32) / 1.8;
    #else
        return 0.0;
    #endif
}

#endif
