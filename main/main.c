#include "button.h"
#include "driver/i2s_std.h"
#include "epaper.h"
#include "epaper_lvgl.h"
#include "esp_log.h"
#include "ui.h"
#include <stdlib.h>

#define BUTTON_UP_GPIO 32
#define BUTTON_DOWN_GPIO 33

static const char *TAG = "MAIN";

void app_main(void) {
    // move button to ui?
    button_config_t btn_up_cfg = button_config_default;
    btn_up_cfg.gpio_num = BUTTON_UP_GPIO;
    button_t *btn_up = button_config(&btn_up_cfg, scroll_up_cb);

    button_config_t btn_down_cfg = button_config_default;
    btn_down_cfg.gpio_num = BUTTON_DOWN_GPIO;
    button_t *btn_down = button_config(&btn_down_cfg, scroll_down_cb);

    ui_init();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (placeholder_cnt1 == 3) {
            delete_button(btn_down);
        }
        if (placeholder_cnt2 == 4) {
            delete_button(btn_up);
        }
    }
}