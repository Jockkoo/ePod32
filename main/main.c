#include "button.h"
#include "driver/i2s_std.h"
#include "epaper.h"
#include "epaper_lvgl.h"
#include "esp_log.h"
#include "lvgl.h"
#include <math.h>
#include <stddef.h>

// EPD - electrophoretic display

static const char *TAG = "MAIN";

#define EPD_WIDTH 296
#define EPD_HEIGHT 128
#define EPD_SPI_HOST SPI3_HOST
#define EPD_GPIO_SDA 23
#define EPD_GPIO_SCL 18
#define EPD_GPIO_CS 5
#define EPD_GPIO_RES 16
#define EPD_GPIO_DC 17
#define EPD_GPIO_BUSY 21
#define EPD_SPI_CLK_FREQ 5000000 // 5MHz

static uint8_t cnt = 0;

static void btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

void app_main(void) {
    lv_init();

    epd_config_t cfg = {
        .pins = {
            .busy = EPD_GPIO_BUSY,
            .rst = EPD_GPIO_RES,
            .dc = EPD_GPIO_DC,
            .cs = EPD_GPIO_CS,
            .sck = EPD_GPIO_SCL,
            .mosi = EPD_GPIO_SDA,
        },
        .spi = {
            .host = SPI3_HOST,
            .speed_hz = EPD_SPI_CLK_FREQ,
        },
        .panel = {
            .type = EPD_PANEL_SSD16XX_290,
            .width = 0,
            .height = 0,
        },
    };

    epd_handle_t epd;

    epd_init(&cfg, &epd);
    epd_panel_info_t panel_info;
    epd_get_info(epd, &panel_info);

    ESP_LOGI(TAG, "Panel: %d x %d, color mode: %d", panel_info.width, panel_info.height,
             panel_info.color_mode);

    epd_lvgl_config_t lvgl_cfg = EPD_LVGL_CONFIG_DEFAULT();
    lvgl_cfg.epd = epd;
    lvgl_cfg.update_mode = EPD_UPDATE_PARTIAL;
    lvgl_cfg.use_partial_refresh = true;
    lvgl_cfg.partial_threshold = 2000;                 // Force full refresh every N partial updates
    lvgl_cfg.dither_mode = EPD_DITHER_FLOYD_STEINBERG; // Enable grayscale dithering

    lv_display_t *disp = epd_lvgl_init(&lvgl_cfg);
    // lv_display_set_rotation(disp, LV_DISP_ROT_90);
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(scr, lv_color_hex(0xffffff), 0);

    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(label, "%d", cnt);
    epd_lvgl_refresh(disp);

    // lv_obj_t *btn = lv_button_create(scr);
    // lv_obj_set_pos(btn, 10, 10);
    // lv_obj_set_size(btn, 120, 50);

    // lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
    // lv_obj_t *label = lv_label_create(btn);
    // lv_label_set_text(label, "Button");
    // lv_obj_center(label);

    // lv_timer_handler();

    while (1) {
        lv_timer_handler();
        epd_lvgl_refresh(disp);
        lv_label_set_text_fmt(label, "%d", cnt);
        cnt++;
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}