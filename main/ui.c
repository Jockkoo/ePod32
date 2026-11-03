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

/*
For now I will not destroy any labels, if I get memory
issues along the way I will change this approach
*/

static const char *TAG = "UI FRONTEND";

const char *welcome_screen_text = "Welcome User, to start listening press any button";

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *label;
} ui_frontend_welcome_screen_handler_t;

#define MAIN_MENU_ITEM_CNT 3

typedef struct {
    lv_obj_t *screen;
    lv_obj_t **labels;
} ui_frontend_main_menu_handler_t;

typedef struct {
    epd_handle_t epd;
    lv_display_t *disp;
    lv_obj_t *scr;
    ui_frontend_welcome_screen_handler_t *welcome_screen_handler;
    ui_frontend_main_menu_handler_t *main_menu_handler;
} ui_frontend_hanlder_t;

static ui_frontend_hanlder_t ui_frontend_handler_loc;
static ui_frontend_hanlder_t *ui_frontend_handler = &ui_frontend_handler_loc;

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

// static void update_song_selection(lv_obj_t **song_rows) {
//     for (size_t i = 0; i < SONGS_PER_PAGE; i++) {
//         if (i == g.selected_song) {
//             lv_obj_set_style_bg_color(song_rows[i], lv_color_black(), 0);
//             lv_obj_set_style_text_color(song_rows[i], lv_color_white(), 0);
//         } else {
//             lv_obj_set_style_bg_color(song_rows[i], lv_color_white(), 0);
//             lv_obj_set_style_text_color(song_rows[i], lv_color_black(), 0);
//         }
//     }
// }

// Useful rendering functions

static void make_label_retro(lv_obj_t *label) {
    /* Implement shadow later
    ---------------------
    */
    lv_obj_set_style_bg_opa(label, LV_OPA_100, 0);
    lv_obj_set_style_border_width(label, 3, 0);
    lv_obj_set_style_border_color(label, lv_color_black(), 0);
    lv_obj_set_style_radius(label, 0, 0);
    lv_obj_set_style_pad_all(label, 6, 0);
}

// Welcome screen

static ui_frontend_welcome_screen_handler_t *create_welcome_screen() {
    ui_frontend_welcome_screen_handler_t *welcome_screen_handler =
        malloc(sizeof(ui_frontend_welcome_screen_handler_t));
    welcome_screen_handler->screen = lv_obj_create(NULL);
    welcome_screen_handler->label = lv_label_create(welcome_screen_handler->screen);
    lv_obj_set_width(welcome_screen_handler->label, 100);
    lv_obj_set_style_bg_color(ui_frontend_handler->scr, lv_color_white(), 0);
    lv_obj_set_style_text_color(ui_frontend_handler->scr, lv_color_black(), 0);
    lv_obj_align(welcome_screen_handler->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_long_mode(welcome_screen_handler->label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(welcome_screen_handler->label, welcome_screen_text);
    return welcome_screen_handler;
}

// Main menu screen

static ui_frontend_main_menu_handler_t *create_main_menu() {
    ui_frontend_main_menu_handler_t *main_menu_handler =
        malloc(sizeof(ui_frontend_main_menu_handler_t));
    main_menu_handler->screen = lv_obj_create(NULL);
    main_menu_handler->labels = malloc(MAIN_MENU_ITEM_CNT * sizeof(lv_obj_t *));
    for (size_t i = 0; i < MAIN_MENU_ITEM_CNT; i++) {
        main_menu_handler->labels[i] = lv_label_create(main_menu_handler->screen);
        lv_obj_set_width(main_menu_handler->labels[i], 100);
        lv_obj_set_style_bg_color(main_menu_handler->screen, lv_color_white(), 0);
        lv_obj_set_style_text_color(main_menu_handler->screen, lv_color_black(), 0);
        make_label_retro(main_menu_handler->labels[i]);
    }
    lv_obj_align(main_menu_handler->labels[0], LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_align_to(main_menu_handler->labels[1], main_menu_handler->labels[0],
                    LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_align(main_menu_handler->labels[2], LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_text(main_menu_handler->labels[0], "All songs");
    lv_label_set_text(main_menu_handler->labels[1], "Favorites");
    lv_label_set_text(main_menu_handler->labels[2], "User settings");
    return main_menu_handler;
}

static void update_main_menu() {
    for (size_t i = 0; i < MAIN_MENU_ITEM_CNT; i++) {
        if (i == ui_handler->main_menu_handler.cursor_pos) {
            lv_obj_set_style_bg_color(ui_frontend_handler->main_menu_handler->labels[i],
                                      lv_color_black(), 0);
            lv_obj_set_style_text_color(ui_frontend_handler->main_menu_handler->labels[i],
                                        lv_color_white(), 0);
        } else {
            lv_obj_set_style_bg_color(ui_frontend_handler->main_menu_handler->labels[i],
                                      lv_color_white(), 0);
            lv_obj_set_style_text_color(ui_frontend_handler->main_menu_handler->labels[i],
                                        lv_color_black(), 0);
        }
    }
}

static void ui_task(void *arg) {
    lv_timer_handler();
    epd_lvgl_refresh(ui_frontend_handler->disp);

    while (1) {
        if (xSemaphoreTake(ui_handler->ui_changed, portMAX_DELAY)) {
            switch (ui_handler->current_ui_state) {
            case WELCOME_SCREEN:
                if (ui_handler->next_ui_state == MAIN_MENU) {
                    ESP_LOGI(TAG, "Changing screen");
                    update_main_menu();
                    lv_screen_load(ui_frontend_handler->main_menu_handler->screen);
                }
                break;
            case MAIN_MENU:
                update_main_menu();
                break;
            case SONG_LIST:
                break;
            case USER_SETTINGS:
                break;
            }
            lv_timer_handler();
            epd_lvgl_refresh(ui_frontend_handler->disp);
        }
    }
}

void ui_init() {
    ui_backend_init();
    ESP_LOGI(TAG, "Initializing UI frontend");
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
            .width = 0, /* Assigning 0 to w and h uses the panel default */
            .height = 0,
        },
    };

    esp_err_t err = epd_init(&cfg, &ui_frontend_handler->epd);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "EPD initialization failed: %s.", esp_err_to_name(err));
        vTaskDelete(NULL);
    }

    epd_lvgl_config_t lvgl_cfg = EPD_LVGL_CONFIG_DEFAULT();

    lvgl_cfg.epd = ui_frontend_handler->epd;
    lvgl_cfg.update_mode = EPD_UPDATE_PARTIAL;
    lvgl_cfg.use_partial_refresh = true;
    lvgl_cfg.partial_threshold = 100;
    lvgl_cfg.dither_mode = EPD_DITHER_NONE;

    ui_frontend_handler->disp = epd_lvgl_init(&lvgl_cfg);

    if (ui_frontend_handler->disp == NULL) {
        ESP_LOGE(TAG, "LVGL initialization failed.: %s");
        vTaskDelete(NULL);
    }

    ui_frontend_handler->scr = lv_screen_active();

    lv_obj_set_style_bg_color(ui_frontend_handler->scr, lv_color_white(), 0);
    lv_obj_set_style_text_color(ui_frontend_handler->scr, lv_color_black(), 0);

    // Assigning all handles
    ui_frontend_handler->welcome_screen_handler = create_welcome_screen();
    ui_frontend_handler->main_menu_handler = create_main_menu();

    // FreeRTOS
    xTaskCreate(ui_task, "ui", 8192, NULL, 5, NULL);

    lv_screen_load(ui_frontend_handler->welcome_screen_handler->screen);
    // Give semaphore straight away to indicate the initial UI change
    ESP_LOGI(TAG, "UI frontend initialized");
}