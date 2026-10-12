#include "button.h"

button_t button_io_conf(button_io_conf_t *conf, button_callback_fn callback) {
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
        .pin_bit_mask = (1ULL << conf->io_num),
    };
    gpio_config(&io_conf);
}