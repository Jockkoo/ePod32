#include "button.h"
#include "driver/i2s_std.h"
#include "epaper.h"
#include "epaper_lvgl.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "inputs.h"
#include "ui.h"
#include "ui_backend.h"
#include <stdlib.h>

static const char *TAG = "MAIN";

void app_main(void) {
    // move button to ui?
    button_t *btn_up = init_default_button(BUTTON_UP_GPIO, scroll_up_cb);
    button_t *btn_down = init_default_button(BUTTON_DOWN_GPIO, scroll_down_cb);
    button_t *btn_confirm = init_default_button(BUTTON_CONFIRM_GPIO, confirm_cb);

    // For string testing purposes
    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path = "/spiffs",
        .format_if_mount_failed = true,
        .max_files = 2,
        .partition_label = NULL,
    };
    esp_err_t err = esp_vfs_spiffs_register(&spiffs_conf);

    if (err != ESP_OK) {
        ESP_LOGE("SPIFFS", "Mount failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI("SPIFFS", "Mounted successfully!");

    ui_init();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}