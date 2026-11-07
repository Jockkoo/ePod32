#include "ui_backend.h"

const char *TAG = "UI BACKEND";

static ui_handler_t ui_handler_loc;
ui_handler_t *ui_handler = &ui_handler_loc;

static page_t pages[PAGE_COUNT];
static const page_id_t welcome_screen_subpage[] = { MAIN_MENU };
static const page_id_t main_menu_subpages[] = { ALL_SONGS, FAVORITES, SETTINGS };

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

static void change_ui_state_uppon_confirm() {
    switch (ui_handler->current_page->id) {
    case MAIN_MENU:
        ui_handler->current_page =
            ui_handler
                ->menus[ui_handler->current_page->subpages[ui_handler->current_page->cursor_pos]];
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
    uint8_t item_cnt = ui_handler->current_page->item_cnt;
    switch (event) {
    case MOVE_UP:
        if (ui_handler->current_page->cursor_pos == 0) {
            ui_handler->current_page->cursor_pos = item_cnt - 1;
        } else {
            ui_handler->current_page->cursor_pos--;
        }
        break;
    case MOVE_DOWN:
        if (ui_handler->current_page->cursor_pos == item_cnt - 1) {
            ui_handler->current_page->cursor_pos = 0;
        } else {
            ui_handler->current_page->cursor_pos++;
        }
        break;
    case CONFIRM:
        change_ui_state_uppon_confirm();
        break;
    default:
        break;
    }
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
    ui_handler->menus[WELCOME_SCREEN]->id = WELCOME_SCREEN;
    ui_handler->menus[WELCOME_SCREEN]->cursor_pos = 0;
    ui_handler->menus[WELCOME_SCREEN]->item_cnt = 0;
    ui_handler->menus[WELCOME_SCREEN]->parent = NONE;
    ui_handler->menus[WELCOME_SCREEN]->subpages = welcome_screen_subpage;

    // Main menu
    ui_handler->menus[MAIN_MENU]->id = MAIN_MENU;
    ui_handler->menus[MAIN_MENU]->cursor_pos = 0;
    ui_handler->menus[MAIN_MENU]->item_cnt = MAIN_MENU_ITEM_CNT;
    ui_handler->menus[MAIN_MENU]->parent = WELCOME_SCREEN;
    ui_handler->menus[MAIN_MENU]->subpages = main_menu_subpages;

    // All songs
    ui_handler->menus[ALL_SONGS]->id = ALL_SONGS;
    ui_handler->menus[ALL_SONGS]->cursor_pos = 0;
    ui_handler->menus[ALL_SONGS]->item_cnt = 0;
    ui_handler->menus[ALL_SONGS]->parent = MAIN_MENU;
    ui_handler->menus[ALL_SONGS]->subpages = NULL;

    // Favorites
    ui_handler->menus[FAVORITES]->id = FAVORITES;
    ui_handler->menus[FAVORITES]->cursor_pos = 0;
    ui_handler->menus[FAVORITES]->item_cnt = 0;
    ui_handler->menus[FAVORITES]->parent = MAIN_MENU;
    ui_handler->menus[FAVORITES]->subpages = NULL;

    // User settings
    ui_handler->menus[SETTINGS]->id = SETTINGS;
    ui_handler->menus[SETTINGS]->cursor_pos = 0;
    ui_handler->menus[SETTINGS]->item_cnt = SETTINGS_ITEM_CNT;
    ui_handler->menus[SETTINGS]->parent = MAIN_MENU;
    ui_handler->menus[SETTINGS]->subpages = NULL;
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
