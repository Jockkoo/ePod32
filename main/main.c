#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stddef.h>

#define LED_GPIO 23
#define BUTTON_GPIO 22
#define DEBOUNCE_DELAY_MS 200
#define MS_TO_US(ms) ((ms) * 1000ULL)

static const char *TAG = "MAIN";
static QueueHandle_t button_queue;
static volatile uint64_t current_time = 0;
static volatile uint64_t last_time = 0;

static void IRAM_ATTR button_isr_handler(void *arg) {
    current_time = esp_timer_get_time();
    if (current_time - last_time > MS_TO_US(DEBOUNCE_DELAY_MS)) {
        uint32_t gpio_num = (uint32_t)arg;
        xQueueSendFromISR(button_queue, &gpio_num, NULL);
        last_time = current_time;
    }
}

void button_task(void *pvParameters) {
    uint32_t io_num;
    while (1) {
        if (xQueueReceive(button_queue, &io_num, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Button Pressed!");
        }
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting the prigram!");
    button_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(button_task, "Button task", 2048, (void *)LED_GPIO, 10, NULL);
    gpio_config_t button_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
    };
    gpio_config(&button_conf);

    gpio_config_t led_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_GPIO),
    };
    gpio_config(&led_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void *)BUTTON_GPIO);

    ESP_LOGI(TAG, "App started!");
}