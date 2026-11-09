#ifndef UI_BACKEND_H
#define UI_BACKEND_H

#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdio.h"
#include <string.h>

#define MAIN_MENU_ITEM_CNT 3
#define SETTINGS_ITEM_CNT 5 // Add a better macro later
#define SONGS_PER_VIEW 5

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
    uint16_t items_per_view;
    uint16_t current_view;
    uint32_t cursor_pos;
    uint32_t item_cnt;
    bool view_changed;
} page_t;

typedef struct {
    QueueHandle_t event_queue;
    SemaphoreHandle_t ui_changed;
    page_t *menus[PAGE_COUNT];
    page_t *current_page;
} ui_handler_t;

void ui_backend_init();

#define get_relative_cursor_pos(current_page) ((current_page->cursor_pos) % (current_page->items_per_view))

#define get_view_cnt(current_page) ((current_page->item_cnt) / (current_page->items_per_view) + 1)

#endif