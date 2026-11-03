#ifndef UI_BACKEND_H
#define UI_BACKEND_H

#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Button functions here for now
void scroll_down_cb(button_t *button, button_action_t action);
void scroll_up_cb(button_t *button, button_action_t action);

typedef enum {
    MOVE_UP = 0,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    CONFIRM,
    INPUTS_CNT,
} user_input_types_t;

typedef enum {
    WELCOME_SCREEN,
    MAIN_MENU,
    SONG_LIST,
    USER_SETTINGS,
} ui_state_t;

typedef struct {
    uint8_t cursor_pos;
    uint8_t item_cnt;
} main_menu_handler_t;

typedef struct {
    QueueHandle_t event_queue;
    SemaphoreHandle_t ui_changed;
    ui_state_t current_ui_state;
    ui_state_t next_ui_state;
    main_menu_handler_t main_menu_handler;
} ui_handler_t;

void ui_backend_init();

#endif