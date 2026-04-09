#include "gui.h"
#include "bloch.h"
#include "config.h" // For DISPLAY_WIDTH, DISPLAY_HEIGHT
#include "lgfx.h"   // For swipe_direction_t, lgfx_pop_swipe
#include "receiver.h"

extern BlochSphere bloch; // main.cpp

// GUI
lv_obj_t *screen;
lv_obj_t *tileview;
lv_obj_t *tile1;
lv_obj_t *tile2;
lv_obj_t *tile3;

static lv_obj_t *gate_buttons[8]; // Expanded to 8 buttons
static lv_obj_t *gate_labels[8];
static lv_obj_t *reset_button;
static lv_obj_t *reset_label;
static lv_obj_t *hint_label;
static lv_obj_t *state_label;

static lv_style_t style_gate_btn;
static lv_style_t style_gate_label;
static lv_style_t style_reset_btn;
static lv_style_t style_reset_label;
static lv_style_t style_info_label;
static lv_style_t style_state_label;

typedef struct
{
    const char *label;
    BlochSphere::permute_t permute;
} gate_def_t;

static const int kTotalButtons = 8;
static const gate_def_t kGates[] = {
    {"X", BlochSphere::PERMUTE_X},
    {"Y", BlochSphere::PERMUTE_Y},
    {"Z", BlochSphere::PERMUTE_Z},
    {"H", BlochSphere::PERMUTE_H},
    {"S", BlochSphere::PERMUTE_S},
    {"S*", BlochSphere::PERMUTE_NS},
    {"T", BlochSphere::PERMUTE_T},
    {"T*", BlochSphere::PERMUTE_NT},
};

static void gate_event_handler(lv_event_t *e);
static void reset_event_handler(lv_event_t *e);
static void update_state_label();

void gui_begin()
{
    // init hw drivers & lvgl
    lgfx_init();
    lvgl_init();

    // create (empty) screen
    screen = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(screen, 4, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_radius(screen, 0, 0);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A2A2A), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x264D4D), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

    lv_style_init(&style_gate_btn);
    lv_style_set_radius(&style_gate_btn, 10);
    lv_style_set_bg_color(&style_gate_btn, lv_color_hex(0x1F7A8C));
    lv_style_set_bg_grad_color(&style_gate_btn, lv_color_hex(0x58A4B0));
    lv_style_set_bg_grad_dir(&style_gate_btn, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_gate_btn, 2);
    lv_style_set_border_color(&style_gate_btn, lv_color_hex(0xBEE3DB));
    lv_style_set_pad_all(&style_gate_btn, 8);

    lv_style_init(&style_gate_label);
    lv_style_set_text_font(&style_gate_label, &lv_font_montserrat_26);
    lv_style_set_text_color(&style_gate_label, lv_color_hex(0xF2F7F7));

    lv_style_init(&style_reset_btn);
    lv_style_set_radius(&style_reset_btn, 10);
    lv_style_set_bg_color(&style_reset_btn, lv_color_hex(0xB23A48));
    lv_style_set_bg_grad_color(&style_reset_btn, lv_color_hex(0xC95D63));
    lv_style_set_bg_grad_dir(&style_reset_btn, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_reset_btn, 2);
    lv_style_set_border_color(&style_reset_btn, lv_color_hex(0xFFD6DA));
    lv_style_set_pad_all(&style_reset_btn, 8);

    lv_style_init(&style_reset_label);
    lv_style_set_text_font(&style_reset_label, &lv_font_montserrat_26);
    lv_style_set_text_color(&style_reset_label, lv_color_hex(0xFFF8F8));

    lv_style_init(&style_info_label);
    lv_style_set_text_font(&style_info_label, &lv_font_montserrat_14);
    lv_style_set_text_color(&style_info_label, lv_color_hex(0xD9F0F0));

    lv_style_init(&style_state_label);
    lv_style_set_text_font(&style_state_label, &lv_font_montserrat_14);
    lv_style_set_text_color(&style_state_label, lv_color_hex(0xFFF3CD));

    hint_label = lv_label_create(screen);
    lv_obj_add_style(hint_label, &style_info_label, 0);
    lv_label_set_text(hint_label, "Swipe < > for gates and reset");
    lv_obj_align(hint_label, LV_ALIGN_TOP_MID, 0, 2);

    state_label = lv_label_create(screen);
    lv_obj_add_style(state_label, &style_state_label, 0);
    lv_obj_align(state_label, LV_ALIGN_BOTTOM_MID, 0, -2);

    // --- NEW: Tileview Setup ---
    tileview = lv_tileview_create(screen);
    // Make tileview background transparent so the screen gradient shows
    lv_obj_set_style_bg_opa(tileview, 0, 0);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF); // Hide scrollbars

    // Add two horizontal tiles
    tile1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
    tile2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
    tile3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR);

    const int top_h = 22;
    const int bottom_h = 22;
    const int gap = 6;
    const int grid_w = DISPLAY_WIDTH - 8;
    const int grid_h = DISPLAY_HEIGHT - top_h - bottom_h - 10;
    const int btn_w = (grid_w - gap) / 2;
    const int btn_h = (grid_h - gap) / 2;
    const int left_x = 4;
    const int right_x = left_x + btn_w + gap;
    const int top_y = top_h + 2;
    const int bottom_y = top_y + btn_h + gap;

    for (int i = 0; i < kTotalButtons; ++i)
    {
        // Decide which tile the button goes on
        lv_obj_t *parent_tile = (i < 4) ? tile1 : tile2;

        gate_buttons[i] = lv_btn_create(parent_tile);
        lv_obj_add_style(gate_buttons[i], &style_gate_btn, 0);
        lv_obj_add_event_cb(gate_buttons[i], gate_event_handler, LV_EVENT_CLICKED, NULL);

        // Local index (0 to 3) for layout math on the current tile
        int local_i = i % 4;
        const int x = (local_i % 2 == 0) ? left_x : right_x;
        const int y = (local_i < 2) ? top_y : bottom_y;

        lv_obj_set_pos(gate_buttons[i], x, y);
        lv_obj_set_size(gate_buttons[i], btn_w, btn_h);

        gate_labels[i] = lv_label_create(gate_buttons[i]);
        lv_obj_add_style(gate_labels[i], &style_gate_label, 0);
        lv_label_set_text(gate_labels[i], kGates[i].label);
        lv_obj_center(gate_labels[i]);
    }

    reset_button = lv_btn_create(tile3);
    lv_obj_add_style(reset_button, &style_reset_btn, 0);
    lv_obj_add_event_cb(reset_button, reset_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(reset_button, grid_w - 8, grid_h / 2);
    lv_obj_align(reset_button, LV_ALIGN_CENTER, 0, 6);

    reset_label = lv_label_create(reset_button);
    lv_obj_add_style(reset_label, &style_reset_label, 0);
    lv_label_set_text(reset_label, "RESET");
    lv_obj_center(reset_label);

    update_state_label();

    // load screen
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);

    return;
};

void gui_loop()
{
    time_t now = millis();

    // Note: lgfx_pop_swipe() logic removed! LVGL handles swiping natively now.

    static time_t prv_state_tick = 0;
    if (now - prv_state_tick > 120)
    {
        update_state_label();
        prv_state_tick = now;
    }

    static time_t prv_tick = 0;
    if (now - prv_tick > (1000 / DISPLAY_MAXFPS))
    {
        lv_tick_inc(now - prv_tick);
        prv_tick = now;
        lv_timer_handler();
    };
};

static void gate_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    const lv_obj_t *target = lv_event_get_target(e);
    for (int i = 0; i < kTotalButtons; ++i)
    {
        if (target == gate_buttons[i])
        {
            receiver_apply_permute(kGates[i].permute);
            update_state_label();
            return;
        }
    }
}

static void reset_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    receiver_reset_bloch();
    update_state_label();
}

static void update_state_label()
{
    const Vector3d s = bloch.getState();
    char buf[64];
    snprintf(buf, sizeof(buf), "x:%0.3f  y:%0.3f  z:%0.3f", s[0], s[1], s[2]);
    lv_label_set_text(state_label, buf);
}