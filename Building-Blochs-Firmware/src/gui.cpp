#include "gui.h"

#include "bloch.h"
#include "lgfx.h"

#include <stdio.h>

extern BlochSphere bloch; // main.cpp

// GUI
lv_obj_t *screen;
static lv_obj_t *gate_buttons[4];
static lv_obj_t *gate_labels[4];
static lv_obj_t *page_label;
static lv_obj_t *hint_label;
static lv_obj_t *state_label;

static lv_style_t style_gate_btn;
static lv_style_t style_gate_label;
static lv_style_t style_info_label;
static lv_style_t style_state_label;

static int current_page = 0;

typedef struct
{
    const char *label;
    BlochSphere::permute_t permute;
} gate_def_t;

static const int kButtonsPerPage = 4;
static const int kPages = 2;
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
static void rebuild_gate_page();
static void update_state_label();
static void apply_swipe_page_change(swipe_direction_t direction);

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

    lv_style_init(&style_info_label);
    lv_style_set_text_font(&style_info_label, &lv_font_montserrat_14);
    lv_style_set_text_color(&style_info_label, lv_color_hex(0xD9F0F0));

    lv_style_init(&style_state_label);
    lv_style_set_text_font(&style_state_label, &lv_font_montserrat_14);
    lv_style_set_text_color(&style_state_label, lv_color_hex(0xFFF3CD));

    page_label = lv_label_create(screen);
    lv_obj_add_style(page_label, &style_info_label, 0);
    lv_obj_align(page_label, LV_ALIGN_TOP_MID, 0, 2);

    hint_label = lv_label_create(screen);
    lv_obj_add_style(hint_label, &style_info_label, 0);
    lv_label_set_text(hint_label, "Swipe < > for more gates");
    lv_obj_align(hint_label, LV_ALIGN_TOP_RIGHT, -2, 2);

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

    for (int i = 0; i < kButtonsPerPage; ++i)
    {
        gate_buttons[i] = lv_btn_create(screen);
        lv_obj_add_style(gate_buttons[i], &style_gate_btn, 0);
        lv_obj_add_event_cb(gate_buttons[i], gate_event_handler, LV_EVENT_CLICKED, NULL);

        const int x = (i % 2 == 0) ? left_x : right_x;
        const int y = (i < 2) ? top_y : bottom_y;
        lv_obj_set_pos(gate_buttons[i], x, y);
        lv_obj_set_size(gate_buttons[i], btn_w, btn_h);

        gate_labels[i] = lv_label_create(gate_buttons[i]);
        lv_obj_add_style(gate_labels[i], &style_gate_label, 0);
        lv_obj_center(gate_labels[i]);
    }

    state_label = lv_label_create(screen);
    lv_obj_add_style(state_label, &style_state_label, 0);
    lv_obj_align(state_label, LV_ALIGN_BOTTOM_MID, 0, -2);

    rebuild_gate_page();
    update_state_label();

    // load screen
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);

    return;
};

void gui_loop()
{
    time_t now = millis();

    swipe_direction_t swipe;
    if (lgfx_pop_swipe(&swipe))
    {
        apply_swipe_page_change(swipe);
    }

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
    for (int i = 0; i < kButtonsPerPage; ++i)
    {
        if (target == gate_buttons[i])
        {
            const int gate_idx = (current_page * kButtonsPerPage) + i;
            bloch.permute(kGates[gate_idx].permute);
            update_state_label();
            return;
        }
    }
}

static void rebuild_gate_page()
{
    char page_buf[16];
    snprintf(page_buf, sizeof(page_buf), "Page %d/%d", current_page + 1, kPages);
    lv_label_set_text(page_label, page_buf);

    for (int i = 0; i < kButtonsPerPage; ++i)
    {
        const int gate_idx = (current_page * kButtonsPerPage) + i;
        lv_label_set_text(gate_labels[i], kGates[gate_idx].label);
    }
}

static void apply_swipe_page_change(swipe_direction_t direction)
{
    if (direction == SWIPE_LEFT && current_page < (kPages - 1))
    {
        current_page++;
        rebuild_gate_page();
    }
    else if (direction == SWIPE_RIGHT && current_page > 0)
    {
        current_page--;
        rebuild_gate_page();
    }
}

static void update_state_label()
{
    const Vector3d s = bloch.getState();
    char buf[64];
    snprintf(buf, sizeof(buf), "x:%0.3f  y:%0.3f  z:%0.3f", s[0], s[1], s[2]);
    lv_label_set_text(state_label, buf);
}
