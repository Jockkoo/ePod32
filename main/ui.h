#ifndef UI_H
#define UI_H

#include "button.h"
#include "epaper.h"
#include "epaper_lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui_backend.h"
#include <stdlib.h>

extern ui_handler_t *ui_handler;
void ui_init();

#endif