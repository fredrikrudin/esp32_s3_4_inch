#ifndef XIAOMI_MANAGER_H
#define XIAOMI_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Avkodar Xiaomis mätvärden för temperatur och luftfuktighet passivt
void handleXiaomiMijiaBLE(String addr, uint8_t* data, int len) {
    if (!mijia.enabled || !addr.equalsIgnoreCase(mijia.mac)) return;

    if (len >= 12) {
        // Temperatur lagras som 16-bitars integer (multiplicerat med 0.1)
        int16_t raw_temp = (data[1] << 8) | data[0];
        mijia.temperature = raw_temp * 0.1;

        // Luftfuktighet lagras på efterföljande byte
        mijia.humidity = data[2];
        
        if (len >= 14) {
            mijia.battery_level = data[3]; // Batteriprocent
        }
    }
}

#endif
