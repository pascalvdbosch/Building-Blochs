#include "lgfx.h"

#include "config.h"

#include <LovyanGFX.hpp>
#include <lvgl.h>
#include <esp_heap_caps.h>

// Draw buffer(s)
#define LV_BUF_SIZE (DISPLAY_HEIGHT * DISPLAY_WIDTH / 10)
lv_disp_draw_buf_t _lv_draw_buf;
lv_color_t *_lv_color_buf1 = nullptr;
lv_color_t *_lv_color_buf2 = nullptr;

// Display driver
lv_disp_drv_t _lv_display_drv; /*Descriptor of a display driver*/
static void lv_disp_cb(lv_disp_drv_t *, const lv_area_t *, lv_color_t *);

lv_indev_drv_t _lv_touch_drv; /*Descriptor of a input device driver*/
static void lv_touchpad_cb(lv_indev_drv_t *, lv_indev_data_t *);

static bool _touch_active = false;
static uint16_t _touch_start_x = 0;
static uint16_t _touch_start_y = 0;
static uint16_t _touch_last_x = 0;
static uint16_t _touch_last_y = 0;
static swipe_direction_t _pending_swipe = SWIPE_NONE;

// M5Stack has Lovyan Auto-detect, apparently
LGFX _lgfx;

void lgfx_init()
{
    _lgfx.init();
#ifdef WOKWI
    _lgfx.setColorDepth(16);
    _lgfx.setRotation(6);
#else
    _lgfx.setColorDepth(24);
    _lgfx.setRotation(1);
#endif

    _lgfx.fillScreen(_lgfx.color888(255, 128, 64));
};

#if LV_USE_LOG
void serial_log_cb(const char *line)
{
    Serial.print(line);
};
#endif

void lvgl_init()
{
    _lv_color_buf1 = (lv_color_t *)heap_caps_malloc(LV_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!_lv_color_buf1)
    {
        // lv_color_t 			    _lv_color_buf1[LV_BUF_SIZE];
    };
    _lv_color_buf2 = nullptr;
#ifdef LFGX_DOUBLEBUF
    _lv_color_buf2 = (lv_color_t *)heap_caps_malloc(LV_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!_lv_color_buf2)
    {
        // lv_color_t 			    _lv_color_buf2[LV_BUF_SIZE];
    };
#endif

    lv_init();

#if LV_USE_LOG
    lv_log_register_print_cb(serial_log_cb);
#endif

    lv_disp_draw_buf_init(&_lv_draw_buf, _lv_color_buf1, _lv_color_buf2, LV_BUF_SIZE);

    lv_disp_drv_init(&_lv_display_drv);       /*Basic initialization*/
    _lv_display_drv.flush_cb = lv_disp_cb;    /*Set your driver function*/
    _lv_display_drv.draw_buf = &_lv_draw_buf; /*Assign the buffer to the display*/
    _lv_display_drv.hor_res = DISPLAY_WIDTH;  /*Set the horizontal resolution of the display*/
    _lv_display_drv.ver_res = DISPLAY_HEIGHT; /*Set the vertical resolution of the display*/
    lv_disp_drv_register(&_lv_display_drv);   /*Finally register the driver*/

    lv_indev_drv_init(&_lv_touch_drv);          /*Basic initialization*/
    _lv_touch_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
    _lv_touch_drv.read_cb = lv_touchpad_cb;     /*Set your driver function*/
    lv_indev_drv_register(&_lv_touch_drv);      /*Finally register the driver*/
};

bool lgfx_pop_swipe(swipe_direction_t *direction)
{
    if (_pending_swipe == SWIPE_NONE)
    {
        return false;
    }

    if (direction)
    {
        *direction = _pending_swipe;
    }
    _pending_swipe = SWIPE_NONE;
    return true;
}

static void lv_disp_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    _lgfx.startWrite();
    _lgfx.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t *)&color_p->full);
    _lgfx.endWrite();

    lv_disp_flush_ready(disp);
};

static void lv_touchpad_cb(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;

    if (!_lgfx.getTouch(&touchX, &touchY))
    {
        if (_touch_active)
        {
            const int dx = static_cast<int>(_touch_last_x) - static_cast<int>(_touch_start_x);
            const int dy = static_cast<int>(_touch_last_y) - static_cast<int>(_touch_start_y);

            if ((abs(dx) >= 25) && (abs(dx) > abs(dy)))
            {
                _pending_swipe = (dx > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
            }
            _touch_active = false;
        }

        data->state = LV_INDEV_STATE_RELEASED;
        return;
    };

    if (!_touch_active)
    {
        _touch_active = true;
        _touch_start_x = touchX;
        _touch_start_y = touchY;
    }
    _touch_last_x = touchX;
    _touch_last_y = touchY;

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touchX;
    data->point.y = touchY;

    // Serial.printf("Touch = (%d, %d)\n", touchX, touchY);
};
