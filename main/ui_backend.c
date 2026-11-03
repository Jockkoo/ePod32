#include "ui_backend.h"

const char *TAG = "UI BACKEND";

static ui_handler_t ui_handler_loc;
ui_handler_t *ui_handler = &ui_handler_loc;

// Button here for now

void scroll_down_cb(button_t *button, button_action_t action) {
    user_input_types_t user_input;
    if (action == BUTTON_SINGLE_PRESS) {
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        user_input = MOVE_DOWN;
        xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);
    } else {
        ESP_LOGI(TAG, "Unknown action!");
    }
}

void scroll_up_cb(button_t *button, button_action_t action) {
    user_input_types_t user_input;
    if (action == BUTTON_SINGLE_PRESS) {
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        user_input = MOVE_UP;
        xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);
    } else {
        ESP_LOGI(TAG, "Unknown action!");
    }
}

// TODO: there are some issues while navigating
static void handle_main_menu_events(user_input_types_t event) {
    uint8_t *cursor_pos = &ui_handler->main_menu_handler.cursor_pos;
    uint8_t item_cnt = ui_handler->main_menu_handler.item_cnt;
    switch (event) {
    case MOVE_UP:
        if (*cursor_pos == 0) {
            (*cursor_pos) = item_cnt - 1;
        } else {
            (*cursor_pos)--;
        }
        break;
    case MOVE_DOWN:
        if (*cursor_pos == item_cnt - 1) {
            (*cursor_pos) = 0;
        } else {
            (*cursor_pos)++;
        }
        break;
    case CONFIRM:
        // Nothing for now
        break;
    default:
        break;
    }
}

static void update_task(void *arg) {
    user_input_types_t user_input;
    while (1) {
        if (xQueueReceive(ui_handler->event_queue, &user_input, portMAX_DELAY)) {
            ui_handler->current_ui_state = ui_handler->next_ui_state;
            switch (ui_handler->current_ui_state) {
            case WELCOME_SCREEN:
                // For now no safety checks, we assume the event must be a button press
                ui_handler->next_ui_state = MAIN_MENU;
                xSemaphoreGive(ui_handler->ui_changed);
                break;
            case MAIN_MENU:
                // TODO: Massive changes needed here
                handle_main_menu_events(user_input);
                xSemaphoreGive(ui_handler->ui_changed);
                break;
            case SONG_LIST:
                // TODO
                break;
            case USER_SETTINGS:
                // TODO, try using flash memory for this
                break;
            default:
                break;
            }
        }
    }
}

void ui_backend_init() {
    ESP_LOGI(TAG, "Initializing UI backend");
    ui_handler->next_ui_state = WELCOME_SCREEN;
    ui_handler->current_ui_state = WELCOME_SCREEN;
    ui_handler->main_menu_handler.item_cnt = 3; // Hardcoded for now
    ui_handler->main_menu_handler.cursor_pos = 2;
    ui_handler->event_queue = xQueueCreate(10, sizeof(user_input_types_t));
    ui_handler->ui_changed = xSemaphoreCreateBinary();
    xTaskCreate(update_task, "ui backend main task", 4096, NULL, 6, NULL);
    ESP_LOGI(TAG, "UI backend initialized");
}
