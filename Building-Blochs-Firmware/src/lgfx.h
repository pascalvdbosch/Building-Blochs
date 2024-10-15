#ifndef __LGFX_H
#define __LGFX_H

// We need to define these in this order
#include "config.h"

#define LGFX_USE_V1
#define LGFX_AUTODETECT

#include <LovyanGFX.hpp>
#include <lvgl.h>

extern LGFX _lgfx;

void lgfx_init();
void lvgl_init();

extern lv_indev_drv_t 		_lv_touch_drv;           /*Descriptor of a input device driver*/

#endif // __SOOGH_LGFX_H
