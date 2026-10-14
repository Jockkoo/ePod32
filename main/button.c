#include "button.h"

// TODO maybe?, see if everything can be done with a semaphore instead of a queue

#define BUTTON_DELETE_MSG 0xFFFFF // No reason for this value, just not a real GPIO pin

struct _button {
    button_config_t config;
    button_callback_fn callback;
    QueueHandle_t queue;
    // SemaphoreHandle_t semaphore;
};

static inline bool is_button_pressed(button_t *button) {
    button_active_level_t level =
        (gpio_get_level(button->config.gpio_num) == 1) ? (BUTTON_ACTIVE_HIGH) : (BUTTON_ACTIVE_LOW);
    bool pressed = (level == button->config.active_level);
    return pressed;
}

static void button_handle_event(button_t *button, uint8_t press_count) {
    switch (press_count) {
    case 1:
        button->callback(button, BUTTON_EVENT_SINGLE_PRESS);
        break;
    case 2:
        button->callback(button, BUTTON_EVENT_DOUBLE_PRESS);
        break;
    case 3:
        button->callback(button, BUTTON_EVENT_TRIPLE_PRESS);
        break;
    default:
        ESP_LOGE("ERROR", "Unknown event happened");
    }
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
    uint32_t dummy_io;
    TickType_t queue_wait_time = portMAX_DELAY;
    TickType_t press_tick = 0;
    TickType_t release_tick = 0;
    uint8_t press_count = 0;

    while (1) {
        if (xQueueReceive(button->queue, &dummy_io, queue_wait_time)) {
            // Deletion check
            if (dummy_io == BUTTON_DELETE_MSG) {
                // Placeholder for now
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(button->config.debounce_time_ms));
            xQueueReset(button->queue);

            if (is_button_pressed(button)) {
                queue_wait_time = portMAX_DELAY;
                press_tick = xTaskGetTickCount();
            } else {
                press_count++;
                release_tick = xTaskGetTickCount();
                TickType_t ticks_held = release_tick - press_tick;

                if (ticks_held >= pdMS_TO_TICKS(button->config.hold_time_ms)) {
                    button->callback(button, BUTTON_EVENT_HOLD);
                    press_count = 0;
                } else {
                    queue_wait_time = pdMS_TO_TICKS(button->config.multi_click_delay_ms);
                    continue;
                }
            }

        }
        // Timeout
        else {
            queue_wait_time = portMAX_DELAY;
            press_count = (press_count > 3) ? 3 : press_count;
            button_handle_event(button, press_count);
            press_count = 0;
        }
    }
    // Placeholder function for now
    ESP_LOGI("BUTTON", "Deleted at GPIO: %d", button->config.gpio_num);
    // xSemaphoreGive(button->semaphore);
    vTaskDelete(NULL);
}

button_t *button_config(button_config_t *conf, button_callback_fn callback) {
    button_t *button = calloc(1, sizeof(button_t));
    if (button == NULL) {
        return NULL;
    }
    button->config = *conf;
    button->callback = callback;
    button->queue = xQueueCreate(conf->queue_size, sizeof(uint32_t));
    // button->semaphore = xSemaphoreCreateBinary();

    gpio_int_type_t intr_type;
    gpio_pullup_t pullup_en;
    gpio_pulldown_t pulldown_en;

    if (conf->active_level == BUTTON_ACTIVE_LOW) {
        intr_type = GPIO_INTR_ANYEDGE;
        pullup_en = GPIO_PULLUP_ENABLE;
        pulldown_en = GPIO_PULLDOWN_DISABLE;
    } else {
        intr_type = GPIO_INTR_ANYEDGE;
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

    return button;
}

void delete_button(button_t *self) {
    if (self == NULL) {
        return;
    }
    uint32_t delete_msg = BUTTON_DELETE_MSG; // Just a dummy for now, might add future functionality

    gpio_intr_disable(self->config.gpio_num);
    gpio_isr_handler_remove(self->config.gpio_num);

    xQueueReset(self->queue);
    xQueueSend(self->queue, &delete_msg, 0);
    // xSemaphoreTake(button->semaphore, portMAX_DELAY);

    // vSemaphoreDelete(button->semaphore);
    vQueueDelete(self->queue);
    gpio_reset_pin(self->config.gpio_num);

    free(self);
}

gpio_num_t button_get_gpio(button_t *self) { return self->config.gpio_num; }