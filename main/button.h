#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdbool.h>
#include <stdlib.h>

#define button_config_default                                                                      \
    {                                                                                              \
        .gpio_num = GPIO_NUM_0,                                                                    \
        .active_level = BUTTON_ACTIVE_LOW,                                                         \
        .debounce_time_ms = 50,                                                                    \
        .multi_click_delay_ms = 500,                                                               \
        .hold_time_ms = 1000,                                                                      \
        .queue_size = 10,                                                                          \
    }

typedef enum {
    BUTTON_EVENT_SINGLE_CLICK,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_TRIPLE_CLICK,
    BUTTON_EVENT_HOLD,
} button_event_t;

typedef enum {
    BUTTON_ACTIVE_LOW = 0,
    BUTTON_ACTIVE_HIGH = 1,
} button_active_level_t;

typedef struct {
    gpio_num_t gpio_num;
    button_active_level_t active_level;
    uint16_t debounce_time_ms;
    uint16_t multi_click_delay_ms;
    uint16_t hold_time_ms;
    uint16_t queue_size;
} button_config_t;

typedef void (*button_callback_fn)(button_event_t event);

void button_config(button_config_t *button_io_conf, button_callback_fn callback_fn);

#endif