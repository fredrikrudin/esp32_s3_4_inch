#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX_Waveshare_4inch : public lgfx::LGFX_Device {
    lgfx::Panel_ST7701  _panel_instance;
    lgfx::Bus_RGB       _bus_instance;
    lgfx::Touch_GT911   _touch_instance;
public:
    LGFX_Waveshare_4inch() {
        {
            auto cfg = _bus_instance.config(); cfg.panel_width = 480; cfg.panel_height = 480;
            cfg.pin_d0 = 11; cfg.pin_d1 = 12; cfg.pin_d2 = 13; cfg.pin_d3 = 14;
            cfg.pin_d4 = 0;  cfg.pin_d5 = 1;  cfg.pin_d6 = 2;  cfg.pin_d7 = 3;
            cfg.pin_d8 = 4;  cfg.pin_d9 = 5;  cfg.pin_d10 = 6; cfg.pin_d11 = 7;
            cfg.pin_d12 = 8; cfg.pin_d13 = 9; cfg.pin_d14 = 10; cfg.pin_d15 = 15;
            cfg.pin_pclk = 16; cfg.pin_vsync = 17; cfg.pin_hsync = 18; cfg.pin_de = 19;
            cfg.freq_write = 12000000; _bus_instance.config(cfg); _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config(); cfg.pin_cs = 39; cfg.pin_rst = 40;
            cfg.panel_width = 480; cfg.panel_height = 480; _panel_instance.config(cfg);
        }
        {
            auto cfg = _touch_instance.config(); cfg.x_max = 479; cfg.y_max = 479;
            cfg.pin_int = 38; cfg.pin_rst = 37; cfg.pin_sda = 36; cfg.pin_scl = 35;
            cfg.freq = 400000; _touch_instance.config(cfg); _panel_instance.setTouch(&_touch_instance);
        }
        setPanel(&_panel_instance);
    }
};
#endif
