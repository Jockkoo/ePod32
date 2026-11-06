#include "ui_backend.h"

const char *TAG = "UI BACKEND";
static ui_handler_t ui_handler_loc;
ui_handler_t *ui_handler = &ui_handler_loc;

static const ui_state_t main_menu_options[] = { ALL_SONGS, FAVORITES, USER_SETTINGS };

static void print_current_ui_state() {
    switch (ui_handler->current_ui_state) {
    case MAIN_MENU:
        ESP_LOGI(TAG, "Currently in state: MAIN_MENU");
        break;
    case ALL_SONGS:
        ESP_LOGI(TAG, "Currently in state: ALL_SONGS");
        break;
    case FAVORITES:
        ESP_LOGI(TAG, "Currently in state: FAVORITES");
        break;
    case USER_SETTINGS:
        ESP_LOGI(TAG, "Currently in state: USER_SETTINGS");
        break;
    default:
        break;
    }
}

static uint32_t get_current_ui_state_item_cnt() {
    switch (ui_handler->current_ui_state) {
    case ALL_SONGS:
        return 21; // placeholder
        // Extract song cnt
    case FAVORITES:
        return 9; // placeholder
        // Extract song cnt
    case USER_SETTINGS:
        return 20; // placeholder
        // Extract setting cnt
    default:
        return ui_handler->navigator.item_cnt; // No change
    }
}

static void navigator_set_params() {
    ui_handler->navigator.cursor_pos = 0;
    ui_handler->navigator.item_cnt = get_current_ui_state_item_cnt();
}

static void change_ui_state_uppon_confirm() {
    switch (ui_handler->current_ui_state) {
    case MAIN_MENU:
        ui_handler->current_ui_state = main_menu_options[ui_handler->navigator.cursor_pos];
        navigator_set_params();
        break;
    case ALL_SONGS:
        // TODO
        break;
    case FAVORITES:
        // TODO
        break;
    case USER_SETTINGS:
        // TODO
        break;
    default:
        break;
    }
    ESP_LOGI(TAG, "This menu has: %d items", ui_handler->navigator.item_cnt);
}

// TODO: there are some issues while navigating
static void navigate(user_input_types_t event) {
    uint8_t item_cnt = ui_handler->navigator.item_cnt;
    switch (event) {
    case MOVE_UP:
        if (ui_handler->navigator.cursor_pos == 0) {
            ui_handler->navigator.cursor_pos = item_cnt - 1;
        } else {
            ui_handler->navigator.cursor_pos--;
        }
        break;
    case MOVE_DOWN:
        if (ui_handler->navigator.cursor_pos == item_cnt - 1) {
            ui_handler->navigator.cursor_pos = 0;
        } else {
            ui_handler->navigator.cursor_pos++;
        }
        break;
    case CONFIRM:
        change_ui_state_uppon_confirm();
        break;
    default:
        break;
    }
    xSemaphoreGive(ui_handler->ui_changed);
}

static void ui_backend_task(void *arg) {
    user_input_types_t user_input;
    while (1) {
        if (xQueueReceive(ui_handler->event_queue, &user_input, portMAX_DELAY)) {
            if (ui_handler->current_ui_state == WELCOME_SCREEN) {
                ui_handler->current_ui_state = MAIN_MENU;
                ESP_LOGI(TAG, "Currently in state: WELCOME_SCREEN, changing to MAIN_MENU...");
                xSemaphoreGive(ui_handler->ui_changed);
            } else {
                navigate(user_input);
            }
            print_current_ui_state();
        }
    }
}

void ui_backend_init() {
    ESP_LOGI(TAG, "Initializing UI backend");
    ui_handler->current_ui_state = WELCOME_SCREEN;
    // Since we are sure to go to main menu after welcome screen we can assign it here
    ui_handler->navigator.item_cnt = MAIN_MENU_ITEM_CNT;
    ui_handler->navigator.cursor_pos = 0;
    ui_handler->event_queue = xQueueCreate(10, sizeof(user_input_types_t));
    ui_handler->ui_changed = xSemaphoreCreateBinary();
    xTaskCreate(ui_backend_task, "ui backend task", 4096, NULL, 6, NULL);
    ESP_LOGI(TAG, "UI backend initialized");
}
