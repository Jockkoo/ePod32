#ifndef INPUTS_H
#define INPUTS_H

#include "button.h"
#include "freertos/FreeRTOS.h"
#include "ui_backend.h"

#define BUTTON_UP_GPIO 33
#define BUTTON_DOWN_GPIO 32
#define BUTTON_CONFIRM_AND_BACK_GPIO 13

extern ui_handler_t *ui_handler;

void scroll_down_cb(button_t *button, button_action_t action);
void scroll_up_cb(button_t *button, button_action_t action);
void confirm_and_back_cb(button_t *button, button_action_t action);

#endif