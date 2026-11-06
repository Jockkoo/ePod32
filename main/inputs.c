#include "inputs.h"

static const char *TAG = "INPUTS";

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

void confirm_cb(button_t *button, button_action_t action) {
    user_input_types_t user_input;
    if (action == BUTTON_SINGLE_PRESS) {
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        user_input = CONFIRM;
        xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);
    } else {
        ESP_LOGI(TAG, "Unknown action!");
    }
}