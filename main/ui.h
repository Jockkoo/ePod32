#ifndef UI_H
#define UI_H

#include "button.h"
#include "epaper.h"
#include "epaper_lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>

void scroll_down_cb(button_t *button, button_action_t action);
void scroll_up_cb(button_t *button, button_action_t action);

void ui_init();

#endif