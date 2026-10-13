#include "button.h"

// Make each button have it's own time
static volatile uint64_t current_time_us = 0;
static volatile uint64_t previous_time_us = 0;
static volatile uint64_t elapsed_time_us = 0;

typedef enum {
    BUTTON_NOT_PRESSED,
    BUTTON_PRESSED,
} button_pressed_state_t;

typedef struct {
    button_config_t config;
    button_callback_fn callback;
    uint16_t press_count;
    QueueHandle_t queue;
} button_t;

static inline bool is_button_pressed(button_t *button) {
    button_active_level_t level =
        (gpio_get_level(button->config.gpio_num) == 1) ? (BUTTON_ACTIVE_HIGH) : (BUTTON_ACTIVE_LOW);
    bool pressed = (level == button->config.active_level);
    return pressed;
}

static void IRAM_ATTR button_isr_handler(void *arg) {
    button_t *button = (button_t *)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(button->queue, &button->config.gpio_num, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void button_task(void *pvParameters) {
    button_t *button = (button_t *)pvParameters;

    while (1) {
        if (xQueueReceive(button->queue, &button->config.gpio_num, portMAX_DELAY)) {
            current_time_us = esp_timer_get_time();
            uint16_t time_difference_ms = (current_time_us - previous_time_us) / 1000;

            if (is_button_pressed(button) && time_difference_ms > button->config.debounce_time_ms) {
                xQueueReset(button->queue);
                // Registered the first click

                uint8_t click_count button->callback(BUTTON_EVENT_SINGLE_CLICK);
                TickType_t time_window = pdMS_TO_TICKS(button->config.multi_click_delay_ms);
                TickType_t start_tick = xTaskGetTickCount();

                while (1) {
                    TickType_t elapsed_ticks = xTaskGetTickCount() - start_tick;
                    TickType_t remaining_ticks = if (elapsed_ticks > time_window) { break; }
                }
                previous_time_us = current_time_us;
            }
        }
    }
}

void button_config(button_config_t *conf, button_callback_fn callback) {
    button_t *button = malloc(sizeof(button_t));
    if (button == NULL) {
        return;
    }
    button->config = *conf;
    button->callback = callback;
    button->press_count = 0;
    button->queue = xQueueCreate(conf->queue_size, sizeof(uint32_t));

    gpio_int_type_t intr_type;
    gpio_pullup_t pullup_en;
    gpio_pulldown_t pulldown_en;

    if (conf->active_level == BUTTON_ACTIVE_LOW) {
        intr_type = GPIO_INTR_NEGEDGE;
        pullup_en = GPIO_PULLUP_ENABLE;
        pulldown_en = GPIO_PULLDOWN_DISABLE;
    } else {
        intr_type = GPIO_INTR_POSEDGE;
        pullup_en = GPIO_PULLUP_DISABLE;
        pulldown_en = GPIO_PULLDOWN_ENABLE;
    }
    gpio_config_t io_conf = {
        .intr_type = intr_type,
        .pull_down_en = pulldown_en,
        .pull_up_en = pullup_en,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << conf->gpio_num),
    };
    gpio_config(&io_conf);

    esp_err_t err = gpio_install_isr_service(0);
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        gpio_isr_handler_add(button->config.gpio_num, button_isr_handler, (void *)button);
    } // Maybe do something if this fails
    xTaskCreate(button_task, "button_task", 2048, button, 10, NULL);
}