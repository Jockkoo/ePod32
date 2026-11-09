#include "inputs.h"

static const char *TAG = "INPUTS";

// To lazy to make this smarter

#define INSERT_DEFAULT_CB(INPUT_TO_TRACK)                                                                              \
    do {                                                                                                               \
        user_input_types_t user_input;                                                                                 \
        if (action == BUTTON_SINGLE_PRESS) {                                                                           \
            ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));                                      \
            user_input = (INPUT_TO_TRACK);                                                                             \
            xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);                                           \
        } else {                                                                                                       \
            ESP_LOGI(TAG, "Unknown action!");                                                                          \
        }                                                                                                              \
    } while (0)

void scroll_down_cb(button_t *button, button_action_t action) {
    user_input_types_t user_input;
    if (action == BUTTON_SINGLE_PRESS) {
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        user_input = (MOVE_DOWN);
        xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);
    } else {
        ESP_LOGI(TAG, "Unknown action!");
    }
}

void scroll_up_cb(button_t *button, button_action_t action) {
    user_input_types_t user_input;
    if (action == BUTTON_SINGLE_PRESS) {
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        user_input = (MOVE_UP);
        xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);
    } else {
        ESP_LOGI(TAG, "Unknown action!");
    }
}

void confirm_and_back_cb(button_t *button, button_action_t action) {
    user_input_types_t user_input;
    switch (action) {
    case BUTTON_SINGLE_PRESS:
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        user_input = CONFIRM;
        xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);
        break;
    case BUTTON_DOUBLE_PRESS:
        ESP_LOGI(TAG, "Double press from GPIO: %d", button_get_gpio(button));
        user_input = BACK;
        xQueueSend(ui_handler->event_queue, &user_input, portMAX_DELAY);
        break;
    default:
        ESP_LOGI(TAG, "Unknown action!");
        break;
    }
}