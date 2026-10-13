#include "button.h"
#include "esp_log.h"
#include <stddef.h>

#define LED_GPIO 23
#define BUTTON_GPIO 22
#define DEBOUNCE_DELAY_MS 200
#define MS_TO_US(ms) ((ms) * 1000ULL)

static const char *TAG = "MAIN";

void cb(button_event_t event) {
    switch (event) {
    case BUTTON_EVENT_SINGLE_PRESS:
        ESP_LOGI(TAG, "Single press!");
        break;
    case BUTTON_EVENT_DOUBLE_PRESS:
        ESP_LOGI(TAG, "Double press!");
        break;
    case BUTTON_EVENT_TRIPLE_PRESS:
        ESP_LOGI(TAG, "Triple press!");
        break;
    default:
        ESP_LOGI(TAG, "Unknown action!");
        break;
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting the prigram!");

    button_config_t button_conf = button_config_default;
    button_conf.gpio_num = BUTTON_GPIO;
    button_conf.debounce_time_ms = 200;
    button_config(&button_conf, cb);
}