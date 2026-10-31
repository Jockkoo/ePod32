#include "ui.h"

#define EPD_WIDTH 128
#define EPD_HEIGHT 296
#define EPD_SPI_HOST SPI3_HOST /* Same as using VSPI */
#define EPD_GPIO_SDA 23
#define EPD_GPIO_SCL 18
#define EPD_GPIO_CS 5
#define EPD_GPIO_RES 16
#define EPD_GPIO_DC 17
#define EPD_GPIO_BUSY 21
#define EPD_SPI_CLK_FREQ 5000000 // 5MHz

#define SONGS_PER_PAGE 5

// TODO, add SIGNLE PRESS ONLY BUTTON VARIANT

static const char *TAG = "UI";

struct global_params_t {
    epd_handle_t epd;
    lv_display_t *disp;
    lv_obj_t *scr;
    lv_obj_t **song_rows;
    QueueHandle_t queue;
    int8_t selected_song;
};

typedef enum {
    SCROLL_UP,
    SCROLL_DOWN,
} ui_actions_t;

static struct global_params_t g;

static const char *songs[SONGS_PER_PAGE] = {
    "Song 1", "Song 2", "Song 3", "Song 4", "Song 5",
};

static void create_song_list(lv_obj_t *scr, lv_obj_t **song_rows) {
    for (size_t i = 0; i < SONGS_PER_PAGE; i++) {
        song_rows[i] = lv_obj_create(scr);
        lv_obj_set_size(song_rows[i], EPD_WIDTH - 20, 32);
        lv_obj_set_pos(song_rows[i], 4, 8 + i * 32);
        lv_obj_set_style_border_width(song_rows[i], 0, 0);
        lv_obj_set_style_radius(song_rows[i], 0, 0);

        lv_obj_t *label = lv_label_create(song_rows[i]);

        lv_label_set_text(label, songs[i]);
    }
}

static void update_song_selection(lv_obj_t **song_rows) {
    for (size_t i = 0; i < SONGS_PER_PAGE; i++) {
        if (i == g.selected_song) {
            lv_obj_set_style_bg_color(song_rows[i], lv_color_black(), 0);
            lv_obj_set_style_text_color(song_rows[i], lv_color_white(), 0);
        } else {
            lv_obj_set_style_bg_color(song_rows[i], lv_color_white(), 0);
            lv_obj_set_style_text_color(song_rows[i], lv_color_black(), 0);
        }
    }
}

void scroll_down_cb(button_t *button, button_action_t action) {
    ui_actions_t ui_action;
    if (action == BUTTON_SINGLE_PRESS) {
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        ui_action = SCROLL_DOWN;
        xQueueSend(g.queue, &ui_action, portMAX_DELAY);
    } else {
        ESP_LOGI(TAG, "Unknown action!");
    }
}

void scroll_up_cb(button_t *button, button_action_t action) {
    ui_actions_t ui_action;
    if (action == BUTTON_SINGLE_PRESS) {
        ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
        ui_action = SCROLL_UP;
        xQueueSend(g.queue, &ui_action, portMAX_DELAY);
    } else {
        ESP_LOGI(TAG, "Unknown action!");
    }
}

static void ui_task(void *arg) {
    create_song_list(g.scr, g.song_rows);
    update_song_selection(g.song_rows);
    lv_timer_handler();
    epd_lvgl_refresh(g.disp);
    ui_actions_t ui_action;

    while (1) {
        if (xQueueReceive(g.queue, &ui_action, portMAX_DELAY)) {
            switch (ui_action) {
            case SCROLL_DOWN:
                g.selected_song++;
                ESP_LOGI(TAG, "Action from queue: scroll DOWN");
                break;
            case SCROLL_UP:
                g.selected_song--;
                ESP_LOGI(TAG, "Action from queue: scroll UP");
                break;
            default:
                break;
            }
            if (g.selected_song == SONGS_PER_PAGE) {
                g.selected_song = 0;
            } else if (g.selected_song < 0) {
                g.selected_song = SONGS_PER_PAGE - 1;
            }

            update_song_selection(g.song_rows);
            lv_timer_handler();
            epd_lvgl_refresh(g.disp);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void ui_init() {
    g.selected_song = 0;

    lv_init(); /* Initialize LVGL internal Systems
    We always want to initialize this first */

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
            .width = 0, /* Assigning 0 to w and h uses the panel default */
            .height = 0,
        },
    };

    esp_err_t err = epd_init(&cfg, &g.epd);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "EPD initialization failed: %s.", esp_err_to_name(err));
        vTaskDelete(NULL);
    }

    epd_lvgl_config_t lvgl_cfg = EPD_LVGL_CONFIG_DEFAULT();

    lvgl_cfg.epd = g.epd;
    lvgl_cfg.update_mode = EPD_UPDATE_PARTIAL;
    lvgl_cfg.use_partial_refresh = true;
    lvgl_cfg.partial_threshold = 100;
    lvgl_cfg.dither_mode = EPD_DITHER_NONE;

    g.disp = epd_lvgl_init(&lvgl_cfg);

    if (g.disp == NULL) {
        ESP_LOGE(TAG, "LVGL initialization failed.: %s");
        vTaskDelete(NULL);
    }

    g.scr = lv_screen_active();

    lv_obj_set_style_bg_color(g.scr, lv_color_white(), 0);
    lv_obj_set_style_text_color(g.scr, lv_color_black(), 0);

    g.song_rows = malloc(SONGS_PER_PAGE * sizeof(lv_obj_t *));

    g.queue = xQueueCreate(10, sizeof(ui_actions_t));
    xTaskCreate(ui_task, "ui", 8192, NULL, 5, NULL);
}