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

#define MAKE_LABEL_RETRO(label)                                                                                        \
    do {                                                                                                               \
        lv_obj_set_style_bg_opa(label, LV_OPA_100, 0);                                                                 \
        lv_obj_set_style_border_width(label, 3, 0);                                                                    \
        lv_obj_set_style_border_color(label, lv_color_black(), 0);                                                     \
        lv_obj_set_style_radius(label, 0, 0);                                                                          \
        lv_obj_set_style_pad_all(label, 6, 0);                                                                         \
    } while (0);

static const char *TAG = "UI FRONTEND";

const char *welcome_screen_text = "Welcome User, to start listening press any button";

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *label;
} ui_welcome_screen_t;

typedef struct {
    lv_obj_t *screen;
    lv_obj_t **labels;
    uint32_t current_selection;
} ui_main_menu_t;

typedef struct {
    lv_obj_t *screen;
    lv_obj_t **labels;
    uint8_t songs_per_page;
    uint32_t current_selection;
} ui_song_list_t;

typedef struct {
    epd_handle_t epd;
    lv_display_t *disp;
    lv_obj_t *scr;
    ui_welcome_screen_t *welcome_screen;
    ui_main_menu_t *main_menu;
    ui_song_list_t *song_list;
} ui_frontend_t;

static ui_frontend_t ui_frontend_handler_loc;
static ui_frontend_t *ui_frontend_handler = &ui_frontend_handler_loc;

// Welcome screen

static ui_welcome_screen_t *create_welcome_screen() {
    ui_welcome_screen_t *welcome_screen = malloc(sizeof(ui_welcome_screen_t));
    welcome_screen->screen = lv_obj_create(NULL);
    welcome_screen->label = lv_label_create(welcome_screen->screen);
    lv_obj_set_width(welcome_screen->label, 100);
    lv_obj_set_style_bg_color(ui_frontend_handler->scr, lv_color_white(), 0);
    lv_obj_set_style_text_color(ui_frontend_handler->scr, lv_color_black(), 0);
    lv_obj_align(welcome_screen->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_long_mode(welcome_screen->label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(welcome_screen->label, welcome_screen_text);
    return welcome_screen;
}

static void render_welcome_screen() {
    if (lv_screen_active() != ui_frontend_handler->welcome_screen->screen) {
        lv_screen_load(ui_frontend_handler->welcome_screen->screen);
    }
}

// Main menu screen

static ui_main_menu_t *create_main_menu() {
    ui_main_menu_t *main_menu = malloc(sizeof(ui_main_menu_t));
    main_menu->screen = lv_obj_create(NULL);
    main_menu->labels = malloc(MAIN_MENU_ITEM_CNT * sizeof(lv_obj_t *));
    main_menu->current_selection = 0;
    for (size_t i = 0; i < MAIN_MENU_ITEM_CNT; i++) {
        main_menu->labels[i] = lv_label_create(main_menu->screen);
        lv_obj_set_width(main_menu->labels[i], 100);
        lv_obj_set_style_bg_color(main_menu->screen, lv_color_white(), 0);
        lv_obj_set_style_text_color(main_menu->screen, lv_color_black(), 0);
        MAKE_LABEL_RETRO(main_menu->labels[i]);
    }
    lv_obj_align(main_menu->labels[0], LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_align_to(main_menu->labels[1], main_menu->labels[0], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_align(main_menu->labels[2], LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_text(main_menu->labels[0], "All songs");
    lv_label_set_text(main_menu->labels[1], "Favorites");
    lv_label_set_text(main_menu->labels[2], "User settings");
    return main_menu;
}

static void render_main_menu() {
    // current_pos is one step behind actual position, we update that at the end
    lv_obj_set_style_bg_color(ui_frontend_handler->main_menu->labels[ui_frontend_handler->main_menu->current_selection],
                              lv_color_white(), 0);
    lv_obj_set_style_text_color(
        ui_frontend_handler->main_menu->labels[ui_frontend_handler->main_menu->current_selection], lv_color_black(), 0);

    lv_obj_set_style_bg_color(ui_frontend_handler->main_menu->labels[ui_handler->current_page->cursor_pos],
                              lv_color_black(), 0);
    lv_obj_set_style_text_color(ui_frontend_handler->main_menu->labels[ui_handler->current_page->cursor_pos],
                                lv_color_white(), 0);

    ui_frontend_handler->main_menu->current_selection = ui_handler->current_page->cursor_pos;

    if (lv_screen_active() != ui_frontend_handler->main_menu->screen) {
        lv_screen_load(ui_frontend_handler->main_menu->screen);
    }
}

// Song lists (all songs and favorites)

static ui_song_list_t *create_song_list() {
    ui_song_list_t *song_list = malloc(sizeof(ui_song_list_t));
    song_list->screen = lv_obj_create(NULL);
    // uint32_t item_cnt = ui_handler->current_page->item_cnt;
    song_list->songs_per_page = SONGS_PER_VIEW;
    song_list->current_selection = 0;
    lv_obj_set_flex_flow(song_list->screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(song_list->screen, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_border_width(song_list->screen, 0, 0);
    lv_obj_set_style_pad_left(song_list->screen, 10, 0);
    lv_obj_set_style_pad_top(song_list->screen, 20, 0);
    lv_obj_set_style_pad_bottom(song_list->screen, 10, 0);
    lv_obj_set_style_bg_color(song_list->screen, lv_color_white(), 0);

    // We only need songs_per_page labels, later we only change the text!
    song_list->labels = malloc(song_list->songs_per_page * sizeof(lv_obj_t *));
    for (size_t i = 0; i < song_list->songs_per_page; i++) {
        song_list->labels[i] = lv_label_create(song_list->screen);
        lv_obj_set_width(song_list->labels[i], LV_PCT(100));
        lv_obj_set_style_bg_color(song_list->labels[i], lv_color_white(), 0);
        lv_obj_set_style_text_color(song_list->labels[i], lv_color_black(), 0);
        lv_obj_set_style_bg_opa(song_list->labels[i], LV_OPA_100, 0);
        lv_obj_set_style_border_width(song_list->labels[i], 1, 0);
        lv_obj_set_style_border_side(song_list->labels[i], LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(song_list->labels[i], lv_color_black(), 0);
    }
    return song_list;
}

#define FILE_LINE_LEN 128 // No song name is this long anyway

static void render_song_list() {
    uint16_t relative_cursor_pos = get_relative_cursor_pos(ui_handler->current_page);
    uint16_t counter = 0;

    if (ui_handler->current_page->view_changed) {
        FILE *fp = fopen("/spiffs/all_songs.txt", "r");
        uint32_t current_line = 0;
        uint32_t target_line = ui_handler->current_page->items_per_view * ui_handler->current_page->current_view;
        ESP_LOGI(TAG, "Target line after view switch: %d", target_line);
        char line[FILE_LINE_LEN];
        while (fgets(line, sizeof(line), fp)) {
            if (current_line >= target_line) {
                ESP_LOGI(TAG, "Counter: %d", counter);
                lv_label_set_text(ui_frontend_handler->song_list->labels[counter], line);
                counter++;
            }
            if (++current_line == target_line + ui_handler->current_page->items_per_view) {
                break;
            }
        }

        // for (--counter; counter < ui_frontend_handler->song_list->songs_per_page; counter++) {
        //     lv_obj_set_flag(ui_frontend_handler->song_list->labels[counter], LV_OBJ_FLAG_HIDDEN, true);
        // }
        // if (counter == ui_frontend_handler->song_list->songs_per_page - 1) {
        //     lv_obj_set_flag(ui_frontend_handler->song_list->labels[counter], LV_OBJ_FLAG_HIDDEN, false);
        // }
        fclose(fp);
    }

    ESP_LOGI(TAG, "Current selection before drawing: %d", ui_frontend_handler->song_list->current_selection);
    lv_obj_set_style_bg_color(ui_frontend_handler->song_list->labels[ui_frontend_handler->song_list->current_selection],
                              lv_color_white(), 0);
    lv_obj_set_style_text_color(
        ui_frontend_handler->song_list->labels[ui_frontend_handler->song_list->current_selection], lv_color_black(), 0);

    lv_obj_set_style_bg_color(ui_frontend_handler->song_list->labels[relative_cursor_pos], lv_color_black(), 0);
    lv_obj_set_style_text_color(ui_frontend_handler->song_list->labels[relative_cursor_pos], lv_color_white(), 0);

    ui_frontend_handler->song_list->current_selection = relative_cursor_pos;

    if (lv_screen_active() != ui_frontend_handler->song_list->screen) {
        lv_screen_load(ui_frontend_handler->song_list->screen);
    }

    ui_handler->current_page->view_changed = false;
}

static void ui_task(void *arg) {
    lv_timer_handler();
    epd_lvgl_refresh(ui_frontend_handler->disp);

    while (1) {
        if (xSemaphoreTake(ui_handler->ui_changed, portMAX_DELAY)) {
            switch (ui_handler->current_page->id) {
            case WELCOME_SCREEN:
                render_welcome_screen();
                break;
            case MAIN_MENU:
                render_main_menu();
                break;
            case ALL_SONGS:
                render_song_list();
                break;
            case FAVORITES:
                break;
            case SETTINGS:
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
    ui_frontend_handler->welcome_screen = create_welcome_screen();
    ui_frontend_handler->main_menu = create_main_menu();
    ui_frontend_handler->song_list = create_song_list();

    // FreeRTOS
    xTaskCreate(ui_task, "ui", 8192, NULL, 5, NULL);

    xSemaphoreGive(ui_handler->ui_changed);

    ESP_LOGI(TAG, "UI frontend initialized");
}