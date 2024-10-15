#include "gui.h"

#include "bloch.h"
#include "lgfx.h"

extern BlochSphere bloch; // main.cpp

// GUI
lv_obj_t *screen;
lv_obj_t *btnX, *btnY, *btnZ, *btnH;
lv_obj_t *lblX, *lblY, *lblZ, *lblH;
static void btnX_event_handler(lv_event_t * e);
static void btnY_event_handler(lv_event_t * e);
static void btnZ_event_handler(lv_event_t * e);
static void btnH_event_handler(lv_event_t * e);

void gui_begin()
{
	// init hw drivers & lvgl
	lgfx_init();
    lvgl_init();

	// create (empty) screen
    screen = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_radius(screen, 0, 0);

	// Add buttons
    const int H = DISPLAY_HEIGHT/2 - 5;
    const int W = DISPLAY_WIDTH/2 - 5;
    lv_obj_t *btnX = lv_btn_create(screen);
        lv_obj_add_event_cb(btnX, btnX_event_handler, LV_EVENT_ALL, NULL);
        lv_obj_set_size(btnX, W, H);
        lv_obj_align(btnX, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_t *btnY = lv_btn_create(screen);
        lv_obj_add_event_cb(btnY, btnY_event_handler, LV_EVENT_ALL, NULL);
        lv_obj_set_size(btnY, W, H);
        lv_obj_align(btnY, LV_ALIGN_TOP_RIGHT, -2, 2);
    lv_obj_t *btnZ = lv_btn_create(screen);
        lv_obj_add_event_cb(btnZ, btnZ_event_handler, LV_EVENT_ALL, NULL);
        lv_obj_set_size(btnZ, W, H);
        lv_obj_align(btnZ, LV_ALIGN_BOTTOM_LEFT, 2, -2);
    lv_obj_t *btnH = lv_btn_create(screen);
        lv_obj_add_event_cb(btnH, btnH_event_handler, LV_EVENT_ALL, NULL);
        lv_obj_set_size(btnH, W, H);
        lv_obj_align(btnH, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
        
    lv_obj_t *lblX = lv_label_create(btnX);
        lv_label_set_text(lblX, "X");
        lv_obj_center(lblX);
    lv_obj_t *lblY = lv_label_create(btnY);
        lv_label_set_text(lblY, "Y");
        lv_obj_center(lblY);
    lv_obj_t *lblZ = lv_label_create(btnZ);
        lv_label_set_text(lblZ, "Z");
        lv_obj_center(lblZ);
    lv_obj_t *lblH = lv_label_create(btnH);
        lv_label_set_text(lblH, "H");
        lv_obj_center(lblH);

	// load screen
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);

	return;
};

void gui_loop()
{
	time_t now = millis();
    static time_t prv_tick = 0;
	if(now - prv_tick > (1000/DISPLAY_MAXFPS))
	{
    	lv_tick_inc(now - prv_tick);
    	prv_tick = now;
	    lv_timer_handler();
	};
};

static void btnX_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        bloch.rotate(Vector3d(1, 0, 0), STEPS_PER_ROTATION * 0.5);
        return;
    };
};
static void btnY_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        bloch.rotate(Vector3d(0, 1, 0), STEPS_PER_ROTATION * 0.5);
        return;
    };
};
static void btnZ_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        bloch.rotate(Vector3d(0, 0, 1), STEPS_PER_ROTATION * 0.5);
        return;
    };
};
static void btnH_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        bloch.rotate(Vector3d(0.707, 0, 0.707), STEPS_PER_ROTATION * 0.5);
        return;
    };
};

