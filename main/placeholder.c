// #define BUTTON1_GPIO 22
// #define BUTTON2_GPIO 21

// static const char *TAG = "MAIN";
// static volatile uint8_t single_press_count = 0;
// static volatile uint8_t double_press_count = 0;

// void cb(button_t *button, button_event_t event) {
//     switch (event) {
//     case BUTTON_EVENT_SINGLE_PRESS:
//         ESP_LOGI(TAG, "Single press from GPIO: %d", button_get_gpio(button));
//         single_press_count++;
//         ESP_LOGI(TAG, "Single press count: %d", single_press_count);
//         break;
//     case BUTTON_EVENT_DOUBLE_PRESS:
//         ESP_LOGI(TAG, "Double press from GPIO: %d", button_get_gpio(button));
//         double_press_count++;
//         break;
//     case BUTTON_EVENT_TRIPLE_PRESS:
//         ESP_LOGI(TAG, "Triple press from GPIO: %d", button_get_gpio(button));
//         break;
//     case BUTTON_EVENT_HOLD:
//         ESP_LOGI(TAG, "Held from GPIO: %d", button_get_gpio(button));
//         break;
//     default:
//         ESP_LOGI(TAG, "Unknown action!");
//         break;
//     }
// }

// void app_main(void) {
//     ESP_LOGI(TAG, "Starting the prigram!");

//     button_config_t button_conf1 = BUTTON_CONFIG_DEFAULT;
//     button_conf1.gpio_num = BUTTON1_GPIO;
//     button_t *button1 = button_config(&button_conf1, cb);
//     button_config_t button_conf2 = BUTTON_CONFIG_DEFAULT;
//     button_conf2.gpio_num = BUTTON2_GPIO;
//     button_t *button2 = button_config(&button_conf2, cb);
//     while (1) {
//         // Testing code
//         if (single_press_count == 3) {
//             single_press_count = 0;
//             delete_button(button1);
//             button1 = NULL;
//         }
//         if (double_press_count == 3) {
//             double_press_count = 0;
//             delete_button(button2);
//             button2 = NULL;
//         }
//         //
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

//
// #define DIN_PIN 22
// #define LCK_PIN 25
// #define BCK_PIN 26
// #define SAMPLE_RATE 48000

// void app_main(void) {
//     i2s_chan_handle_t tx_handle;
//     i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
//     i2s_new_channel(&chan_cfg, &tx_handle, NULL);

//     const i2s_std_config_t std_cfg = {
//         .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
//         .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
//         I2S_SLOT_MODE_STEREO), .gpio_cfg = {
//             .mclk = I2S_GPIO_UNUSED,
//             .bclk = BCK_PIN,
//             .ws = LCK_PIN,
//             .dout = DIN_PIN,
//             .din = I2S_GPIO_UNUSED,
//             .invert_flags = {
//                 .bclk_inv = false,
//                 .mclk_inv = false,
//                 .ws_inv = false,
//             },
//         },
//     };

//     i2s_channel_init_std_mode(tx_handle, &std_cfg);
//     i2s_channel_enable(tx_handle);
// }