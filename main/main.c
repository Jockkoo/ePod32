#include "button.h"
#include "esp_log.h"
#include <stddef.h>

#define BUTTON1_GPIO 22
#define BUTTON2_GPIO 21

static const char *TAG = "MAIN";
static volatile uint8_t single_press_count = 0;
static volatile uint8_t double_press_count = 0;

void cb(button_t *button, button_event_t event) {
    switch (event) {
    case BUTTON_EVENT_SINGLE_PRESS:
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        single_press_count++;
        ESP_LOGI(TAG, "Single press count: %d", single_press_count);
        break;
    case BUTTON_EVENT_DOUBLE_PRESS:
        ESP_LOGI(TAG, "Double press from GPIO: %d", button_get_gpio(button));
        double_press_count++;
        break;
    case BUTTON_EVENT_TRIPLE_PRESS:
        ESP_LOGI(TAG, "Triple press from GPIO: %d", button_get_gpio(button));
        break;
    case BUTTON_EVENT_HOLD:
        ESP_LOGI(TAG, "Held from GPIO: %d", button_get_gpio(button));
        break;
    default:
        ESP_LOGI(TAG, "Unknown action!");
        break;
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting the prigram!");

    button_config_t button_conf1 = button_config_default;
    button_conf1.gpio_num = BUTTON1_GPIO;
    button_t *button1 = button_config(&button_conf1, cb);
    button_config_t button_conf2 = button_config_default;
    button_conf2.gpio_num = BUTTON2_GPIO;
    button_t *button2 = button_config(&button_conf2, cb);
    while (1) {
        // Testing code
        if (single_press_count == 3) {
            single_press_count = 0;
            delete_button(button1);
            button1 = NULL;
        }
        if (double_press_count == 3) {
            double_press_count = 0;
            delete_button(button2);
            button2 = NULL;
        }
        //
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}