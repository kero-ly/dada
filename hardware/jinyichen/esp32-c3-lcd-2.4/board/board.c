/* Board binding: turns the generated manifest header into component configs. */

#include "board.h"

#include "dada_board_config.h"
#include "display_st7789.h"
#include "esp_log.h"
#include "touch_cst836u.h"

static const char *TAG = "board";

/* The binding refuses to build for a different board's manifest. */
#ifndef DADA_BOARD_ID
#error "dada_board_config.h is missing; run tools/gen_board_config.py"
#endif

#if !DADA_BOARD_HAS_DISPLAY
#error "this board binding requires a display section in board.json"
#endif

#define DADA_BOARD_WIDTH DADA_BOARD_DISPLAY_W
#define DADA_BOARD_HEIGHT DADA_BOARD_DISPLAY_H

dada_err_t board_init(dada_display_t **display, dada_input_t **input)
{
    ESP_LOGI(TAG, "board %s (flash %d MB)", DADA_BOARD_ID, DADA_BOARD_FLASH_MB);

    if (display) {
        display_st7789_config_t cfg = {
            .sclk_gpio = DADA_BOARD_LCD_SCLK,
            .mosi_gpio = DADA_BOARD_LCD_MOSI,
            .cs_gpio = DADA_BOARD_LCD_CS,
            .dc_gpio = DADA_BOARD_LCD_DC,
            .rst_gpio = DADA_BOARD_LCD_RST,
            .backlight_gpio = DADA_BOARD_LCD_BACKLIGHT,
            .width = DADA_BOARD_WIDTH,
            .height = DADA_BOARD_HEIGHT,
            .pclk_hz = 40 * 1000 * 1000,
            .spi_mode = 0,
            .invert_color = false,
            .bgr_order = false,
        };
        dada_err_t err = display_st7789_init(&cfg, display);
        if (err != DADA_OK) {
            ESP_LOGE(TAG, "display init failed (%d)", err);
            return err;
        }
    }

#if DADA_BOARD_HAS_TOUCH
    if (input) {
        touch_cst836u_config_t cfg = {
            .scl_gpio = DADA_BOARD_TP_SCL,
            .sda_gpio = DADA_BOARD_TP_SDA,
            .int_gpio = DADA_BOARD_TP_INT,
            .rst_gpio = DADA_BOARD_TP_RST,
            .i2c_address = 0x15, /* CST836U default; 0x5D on some batches */
            .scl_speed_hz = 400 * 1000,
            .width = DADA_BOARD_WIDTH,
            .height = DADA_BOARD_HEIGHT,
            .swap_xy = false,
            .invert_x = false,
            .invert_y = false,
        };
        dada_err_t err = touch_cst836u_init(&cfg, input);
        if (err != DADA_OK) {
            ESP_LOGE(TAG, "touch init failed (%d)", err);
            return err;
        }
    }
#else
    if (input) {
        ESP_LOGW(TAG, "board has no touch; input left NULL");
        *input = NULL;
    }
#endif

    return DADA_OK;
}

void board_deinit(dada_display_t *display, dada_input_t *input)
{
    if (display) {
        display_st7789_deinit(display);
    }
    if (input) {
        touch_cst836u_deinit(input);
    }
}
