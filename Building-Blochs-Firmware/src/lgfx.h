#ifndef __LGFX_H
#define __LGFX_H

// We need to define these in this order
#include "config.h"

#ifndef WOKWI
#define LGFX_AUTODETECT
#endif

#include <LovyanGFX.hpp>
#include <lvgl.h>

#ifdef WOKWI
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9342 _panel_instance; // <-- CHANGED to ILI9342
    lgfx::Bus_SPI _bus_instance;

public:
    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            // CoreS3 exact SPI pins
            cfg.pin_sclk = 36;
            cfg.pin_mosi = 37;
            cfg.pin_miso = -1;
            cfg.pin_dc = 35;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 3; // CoreS3 CS pin
            cfg.pin_rst = -1;
            cfg.pin_busy = -1;
            cfg.panel_width = 320;  // <-- CHANGED to native landscape width
            cfg.panel_height = 240; // <-- CHANGED to native landscape height
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true; // <-- CoreS3 panels usually need this inverted
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};
#endif

extern LGFX _lgfx;

void lgfx_init();
void lvgl_init();

extern lv_indev_drv_t _lv_touch_drv; /*Descriptor of a input device driver*/

#endif // __SOOGH_LGFX_H
