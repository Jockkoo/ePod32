#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdbool.h>
#include <stdlib.h>

#define BUTTON_CONFIG_DEFAULT                                                                                          \
    {                                                                                                                  \
        .gpio_num = GPIO_NUM_0,                                                                                        \
        .active_level = BUTTON_ACTIVE_LOW,                                                                             \
        .debounce_time_ms = 50,                                                                                        \
        .multi_click_delay_ms = 300,                                                                                   \
        .hold_time_ms = 750,                                                                                           \
        .queue_size = 10,                                                                                              \
        .actions_to_consider = BUTTON_SINGLE_PRESS,                                                                    \
    }

typedef enum {
    BUTTON_SINGLE_PRESS = 0b0001,
    BUTTON_DOUBLE_PRESS = 0b0010,
    BUTTON_TRIPLE_PRESS = 0b0100,
    BUTTON_HOLD = 0b1000,
} button_action_t;

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
    uint8_t actions_to_consider;
} button_config_t;

typedef struct _button button_t;

typedef void (*button_callback_fn)(button_t *self, button_action_t action);

button_t *init_button(button_config_t *button_io_conf, button_callback_fn callback_fn);
button_t *init_default_button(uint8_t button_gpio, button_callback_fn cb);
void delete_button(button_t *self);

// Implement more methods later
gpio_num_t button_get_gpio(button_t *self);

#endif