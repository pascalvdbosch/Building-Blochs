#ifndef __GUI_H
#define __GUI_H

void gui_begin();
void gui_loop();

void gui_set_ble_connected(bool connected);
void gui_set_ble_task(const char* task);
void gui_set_screen_color(int r, int g, int b);
void gui_reset_screen_color();
#endif // __GUI_H
