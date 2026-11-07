#ifndef UI_BACKEND_H
#define UI_BACKEND_H

#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAIN_MENU_ITEM_CNT 3
#define SETTINGS_ITEM_CNT 5 // Add a better macro later

typedef enum {
    MOVE_UP = 0,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    CONFIRM,
    BACK,
    INPUTS_COUNT,
} user_input_types_t;

typedef enum {
    NONE = -1,
    WELCOME_SCREEN,
    MAIN_MENU,
    // ↓
    ALL_SONGS,
    FAVORITES,
    SETTINGS,
    PAGE_COUNT,
} page_id_t;

typedef struct {
    page_id_t id;
    page_id_t parent;
    page_id_t *subpages;
    uint32_t cursor_pos;
    uint32_t item_cnt;
} page_t;

typedef struct {
    QueueHandle_t event_queue;
    SemaphoreHandle_t ui_changed;
    page_t *menus[PAGE_COUNT];
    page_t *current_page;
} ui_handler_t;

void ui_backend_init();

#endif