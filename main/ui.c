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

#define MAIN_MENU_ITEM_CNT 3

/*
For now I will not destroy any labels, if I get memory
issues along the way I will change this approach
*/

#define MAKE_LABEL_RETRO(label)                                                                    \
    do {                                                                                           \
        lv_obj_set_style_bg_opa(label, LV_OPA_100, 0);                                             \
        lv_obj_set_style_border_width(label, 3, 0);                                                \
        lv_obj_set_style_border_color(label, lv_color_black(), 0);                                 \
        lv_obj_set_style_radius(label, 0, 0);                                                      \
        lv_obj_set_style_pad_all(label, 6, 0);                                                     \
    } while (0);

static const char *TAG = "UI FRONTEND";

const char *welcome_screen_text = "Welcome User, to start listening press any button";

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *label;
} ui_frontend_welcome_screen_handler_t;

// Usable for main menu, all songs and favorites
typedef struct {
    lv_obj_t *screen;
    lv_obj_t **labels;
    uint32_t current_selection;
} ui_frontend_list_handler_t;

typedef struct {
    epd_handle_t epd;
    lv_display_t *disp;
    lv_obj_t *scr;
    ui_frontend_welcome_screen_handler_t *welcome_screen_handler;
    ui_frontend_list_handler_t *main_menu_handler;
    ui_frontend_list_handler_t *song_list_handler;
} ui_frontend_hanlder_t;

static ui_frontend_hanlder_t ui_frontend_handler_loc;
static ui_frontend_hanlder_t *ui_frontend_handler = &ui_frontend_handler_loc;

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

static void render_welcome_screen() {
    if (lv_screen_active() != ui_frontend_handler->welcome_screen_handler->screen) {
        lv_screen_load(ui_frontend_handler->welcome_screen_handler->screen);
    }
}

// Main menu screen

static ui_frontend_list_handler_t *create_main_menu() {
    ui_frontend_list_handler_t *main_menu_handler = malloc(sizeof(ui_frontend_list_handler_t));
    main_menu_handler->screen = lv_obj_create(NULL);
    main_menu_handler->labels = malloc(MAIN_MENU_ITEM_CNT * sizeof(lv_obj_t *));
    main_menu_handler->current_selection = ui_handler->navigator.cursor_pos;
    for (size_t i = 0; i < MAIN_MENU_ITEM_CNT; i++) {
        main_menu_handler->labels[i] = lv_label_create(main_menu_handler->screen);
        lv_obj_set_width(main_menu_handler->labels[i], 100);
        lv_obj_set_style_bg_color(main_menu_handler->screen, lv_color_white(), 0);
        lv_obj_set_style_text_color(main_menu_handler->screen, lv_color_black(), 0);
        MAKE_LABEL_RETRO(main_menu_handler->labels[i]);
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

static void render_main_menu() {
    // current_pos is one step behind actual position, we update that at the end
    lv_obj_set_style_bg_color(
        ui_frontend_handler->main_menu_handler
            ->labels[ui_frontend_handler->main_menu_handler->current_selection],
        lv_color_white(), 0);
    lv_obj_set_style_text_color(
        ui_frontend_handler->main_menu_handler
            ->labels[ui_frontend_handler->main_menu_handler->current_selection],
        lv_color_black(), 0);

    lv_obj_set_style_bg_color(
        ui_frontend_handler->main_menu_handler->labels[ui_handler->navigator.cursor_pos],
        lv_color_black(), 0);
    lv_obj_set_style_text_color(
        ui_frontend_handler->main_menu_handler->labels[ui_handler->navigator.cursor_pos],
        lv_color_white(), 0);

    ui_frontend_handler->main_menu_handler->current_selection = ui_handler->navigator.cursor_pos;

    if (lv_screen_active() != ui_frontend_handler->main_menu_handler->screen) {
        lv_screen_load(ui_frontend_handler->main_menu_handler->screen);
    }
}

// Song lists (all songs and favorites)

// TODO
// static ui_frontend_list_handler_t *create_song_list() {}

static void ui_task(void *arg) {
    lv_timer_handler();
    epd_lvgl_refresh(ui_frontend_handler->disp);

    while (1) {
        if (xSemaphoreTake(ui_handler->ui_changed, portMAX_DELAY)) {
            switch (ui_handler->current_ui_state) {
            case WELCOME_SCREEN:
                render_welcome_screen();
                break;
            case MAIN_MENU:
                render_main_menu();
                break;
            case ALL_SONGS:
                break;
            case FAVORITES:
                break;
            case USER_SETTINGS:
                break;
            default:
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

    ESP_LOGI(TAG, "UI frontend initialized");
}