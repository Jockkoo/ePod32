#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"

typedef enum {
    BUTTON_EVENT_SINGLE_PRESS,
    BUTTON_EVENT_DOUBLE_PRESS,
    BUTTON_EVENT_TRIPLE_PRESS,
    BUTTON_EVENT_HOLD,
} button_event_t;

typedef enum {
    BUTTON_ACTIVE_LOW = 0,
    BUTTON_ACTIVE_HIGH = 1,
} button_active_level_t;

typedef struct {
    gpio_num_t io_num;
    button_active_level_t active_level;
    uint16_t debounce_time_ms;
    uint16_t hold_time_ms;
} button_io_conf_t;

typedef void (*button_callback_fn)(button_event_t event);

typedef struct {
    button_io_conf_t io_conf;
    button_callback_fn callback;
} button_t;

button_t *button_conf(button_io_conf_t *button_io_conf,
                      button_callback_fn callback_fn);

#endif