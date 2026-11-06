#ifndef UI_BACKEND_H
#define UI_BACKEND_H

#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAIN_MENU_ITEM_CNT 3

typedef enum {
    MOVE_UP = 0,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    CONFIRM,
    BACK,
    INPUTS_CNT,
} user_input_types_t;

typedef enum {
    WELCOME_SCREEN,
    MAIN_MENU,
    // ↓
    ALL_SONGS,
    FAVORITES,
    USER_SETTINGS,
} ui_state_t;

typedef struct {
    uint8_t cursor_pos;
    uint32_t item_cnt;
} navigator_t;

typedef struct {
    QueueHandle_t event_queue;
    SemaphoreHandle_t ui_changed;
    ui_state_t current_ui_state;
    // ui_state_t next_ui_state;
    navigator_t navigator;
} ui_handler_t;

void ui_backend_init();

#endif