#include "ui_backend.h"

const char *TAG = "UI BACKEND";

static ui_handler_t ui_handler_loc;
ui_handler_t *ui_handler = &ui_handler_loc;

static page_t pages[PAGE_COUNT];
static page_id_t welcome_screen_subpage[] = { MAIN_MENU };
static page_id_t main_menu_subpages[] = { ALL_SONGS, FAVORITES, SETTINGS };

#define default_menu_config(ID)                                                                                        \
    do {                                                                                                               \
        ui_handler->menus[(ID)]->id = (ID);                                                                            \
        ui_handler->menus[(ID)]->cursor_pos = 0;                                                                       \
        ui_handler->menus[(ID)]->item_cnt = 0;                                                                         \
        ui_handler->menus[(ID)]->items_per_view = 0;                                                                   \
        ui_handler->menus[(ID)]->current_view = 0;                                                                     \
        ui_handler->menus[(ID)]->parent = NONE;                                                                        \
        ui_handler->menus[(ID)]->subpages = NULL;                                                                      \
        ui_handler->menus[(ID)]->view_changed = true;                                                                  \
    } while (0)

// Functions bellow are placeholders -----------------------------------

static void print_current_ui_state() {
    switch (ui_handler->current_page->id) {
    case MAIN_MENU:
        ESP_LOGI(TAG, "Currently in state: MAIN_MENU");
        break;
    case ALL_SONGS:
        ESP_LOGI(TAG, "Currently in state: ALL_SONGS");
        break;
    case FAVORITES:
        ESP_LOGI(TAG, "Currently in state: FAVORITES");
        break;
    case SETTINGS:
        ESP_LOGI(TAG, "Currently in state: USER_SETTINGS");
        break;
    default:
        break;
    }
}

static uint32_t get_all_songs_count() {
    const char *path = "/spiffs/all_songs.txt";
    FILE *f_all_songs = fopen(path, "r");
    if (f_all_songs == NULL) {
        ESP_LOGE(TAG, "Could not open file: %s", path);
        return 0;
    }
    int c;
    uint32_t lines = 1;
    while ((c = fgetc(f_all_songs)) != EOF) {
        if (c == '\n') {
            lines++;
        }
    }
    fclose(f_all_songs);
    return lines;
}

static uint32_t get_favorites_count() {
    const char *path = "/spiffs/favorites.txt";
    FILE *f_all_songs = fopen(path, "r");
    if (f_all_songs == NULL) {
        ESP_LOGE(TAG, "Could not open file: %s", path);
        return 0;
    }
    int c;
    uint32_t lines = 1;
    while ((c = fgetc(f_all_songs)) != EOF) {
        if (c == '\n') {
            lines++;
        }
    }
    fclose(f_all_songs);
    return lines;
}

// ------------------------------------------------ end of placeholder functions

static void change_ui_state_uppon_confirm() {
    switch (ui_handler->current_page->id) {
    case MAIN_MENU:
        // Set current page to be next page
        ui_handler->current_page =
            ui_handler->menus[ui_handler->current_page->subpages[ui_handler->current_page->cursor_pos]];
        break;
    case ALL_SONGS:
        // TODO
        break;
    case FAVORITES:
        // TODO
        break;
    case SETTINGS:
        // TODO
        break;
    default:
        break;
    }
    ESP_LOGI(TAG, "This menu has: %d items", ui_handler->current_page->item_cnt);
}

// TODO: there are some issues while navigating
static void navigate(user_input_types_t event) {
    page_t *current_page = ui_handler->current_page;
    uint32_t item_cnt = current_page->item_cnt;
    uint32_t items_per_view = current_page->items_per_view;
    uint16_t last_view = current_page->current_view;
    ESP_LOGI(TAG, "Cursor before logic: %d", current_page->cursor_pos);
    ESP_LOGI(TAG, "View before logic: %d", current_page->current_view);
    switch (event) {
    case MOVE_UP:
        if (current_page->cursor_pos == 0) {
            current_page->cursor_pos = item_cnt - 1;
        } else {
            current_page->cursor_pos--;
        }
        current_page->current_view = current_page->cursor_pos / items_per_view;
        if (current_page->current_view != last_view) {
            current_page->view_changed = true;
        }
        break;
    case MOVE_DOWN:
        if (current_page->cursor_pos == item_cnt - 1) {
            current_page->cursor_pos = 0;
        } else {
            current_page->cursor_pos++;
        }
        current_page->current_view = current_page->cursor_pos / items_per_view;
        if (current_page->current_view != last_view) {
            current_page->view_changed = true;
        }
        break;
    case CONFIRM:
        change_ui_state_uppon_confirm();
        break;
    case BACK:
        ui_handler->current_page = ui_handler->menus[ui_handler->current_page->parent];
        break;
    default:
        break;
    }
    ESP_LOGI(TAG, "Cursor after logic: %d", current_page->cursor_pos);
    ESP_LOGI(TAG, "View after logic: %d", current_page->current_view);
    xSemaphoreGive(ui_handler->ui_changed);
}

static void ui_backend_task(void *arg) {
    user_input_types_t user_input;
    while (1) {
        if (xQueueReceive(ui_handler->event_queue, &user_input, portMAX_DELAY)) {
            if (ui_handler->current_page == ui_handler->menus[WELCOME_SCREEN]) {
                ui_handler->current_page = ui_handler->menus[MAIN_MENU];
                ESP_LOGI(TAG, "Currently in state: WELCOME_SCREEN, changing to MAIN_MENU...");
                xSemaphoreGive(ui_handler->ui_changed);
            } else {
                navigate(user_input);
            }
            print_current_ui_state();
        }
    }
}

static void init_all_menus() {
    for (size_t i = 0; i < PAGE_COUNT; i++) {
        ui_handler->menus[i] = &pages[i];
    }
    // Welcome screen
    default_menu_config(WELCOME_SCREEN);
    ui_handler->menus[WELCOME_SCREEN]->subpages = welcome_screen_subpage;

    // Main menu
    default_menu_config(MAIN_MENU);
    ui_handler->menus[MAIN_MENU]->item_cnt = MAIN_MENU_ITEM_CNT;
    ui_handler->menus[MAIN_MENU]->items_per_view = MAIN_MENU_ITEM_CNT;
    ui_handler->menus[MAIN_MENU]->parent = WELCOME_SCREEN;
    ui_handler->menus[MAIN_MENU]->subpages = main_menu_subpages;

    // All songs
    default_menu_config(ALL_SONGS);
    ui_handler->menus[ALL_SONGS]->item_cnt = get_all_songs_count();
    ui_handler->menus[ALL_SONGS]->items_per_view = SONGS_PER_VIEW;
    ui_handler->menus[ALL_SONGS]->parent = MAIN_MENU;

    // Favorites
    default_menu_config(FAVORITES);
    ui_handler->menus[FAVORITES]->item_cnt = get_favorites_count();
    ui_handler->menus[FAVORITES]->items_per_view = SONGS_PER_VIEW;
    ui_handler->menus[FAVORITES]->parent = MAIN_MENU;

    // User settings
    default_menu_config(SETTINGS);
    ui_handler->menus[SETTINGS]->item_cnt = SETTINGS_ITEM_CNT;
    ui_handler->menus[SETTINGS]->parent = MAIN_MENU;
}

void ui_backend_init() {
    ESP_LOGI(TAG, "Initializing UI backend");

    // Since we are sure to go to main menu after welcome screen we can assign it here
    init_all_menus();
    ui_handler->current_page = ui_handler->menus[WELCOME_SCREEN];
    ui_handler->event_queue = xQueueCreate(10, sizeof(user_input_types_t));
    ui_handler->ui_changed = xSemaphoreCreateBinary();
    xTaskCreate(ui_backend_task, "ui backend task", 4096, NULL, 6, NULL);
    ESP_LOGI(TAG, "UI backend initialized");
}
