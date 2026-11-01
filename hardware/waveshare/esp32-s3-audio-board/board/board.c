/* Board binding: turns the generated manifest header into component configs. */

#include "board.h"

#include "dada_board_config.h"
#include "display_st7789.h"
#include "esp_log.h"
#if CONFIG_SPIRAM
#include "esp_psram.h"
#endif
#include "esp_timer.h"
#include "storage_nvs.h"

static const char *TAG = "board";

#ifndef DADA_BOARD_ID
#error "dada_board_config.h is missing; run tools/gen_board_config.py"
#endif

#if !DADA_BOARD_HAS_DISPLAY
#error "this board binding requires a display section in board.json"
#endif

#define DADA_BOARD_WIDTH DADA_BOARD_DISPLAY_W
#define DADA_BOARD_HEIGHT DADA_BOARD_DISPLAY_H

/* Monotonic milliseconds from the SoC timer: no RTC, no network needed. */
static uint64_t board_monotonic_ms(void *ctx)
{
    (void)ctx;
    return (uint64_t)(esp_timer_get_time() / 1000);
}

/* The PCF85063 on this board is not driven yet; report "no wall clock" rather
 * than a fabricated time. */
static bool board_wall_clock(void *ctx, dada_datetime_t *out)
{
    (void)ctx;
    (void)out;
    return false;
}

static dada_time_t board_time = {
    .ctx = NULL,
    .monotonic_ms = board_monotonic_ms,
    .wall_clock = board_wall_clock,
};

dada_err_t board_init(dada_display_t **display, dada_input_t **input,
                      dada_time_t **time, dada_storage_t **storage)
{
    ESP_LOGI(TAG, "board %s (flash %d MB, psram %d MB)", DADA_BOARD_ID,
             DADA_BOARD_FLASH_MB, DADA_BOARD_PSRAM_MB);
#if CONFIG_SPIRAM
    size_t psram = esp_psram_get_size();
    if (psram) {
        ESP_LOGI(TAG, "psram %u KB usable", (unsigned)(psram / 1024));
    } else {
        ESP_LOGW(TAG, "psram not initialised");
    }
#else
    ESP_LOGW(TAG, "built without psram");
#endif

    if (time) {
        *time = &board_time;
    }

    if (storage) {
        dada_err_t err = storage_nvs_init(storage);
        if (err != DADA_OK) {
            ESP_LOGE(TAG, "storage init failed (%d)", err);
            return err;
        }
    }

    if (display) {
        display_st7789_config_t cfg = {
            .sclk_gpio = DADA_BOARD_LCD_SCLK,
            .mosi_gpio = DADA_BOARD_LCD_MOSI,
            .cs_gpio = DADA_BOARD_LCD_CS,
            .dc_gpio = DADA_BOARD_LCD_DC,
            .rst_gpio = DADA_BOARD_LCD_RST,   /* -1: software reset */
            .backlight_gpio = DADA_BOARD_LCD_BACKLIGHT,
            .width = DADA_BOARD_WIDTH,
            .height = DADA_BOARD_HEIGHT,
            .pclk_hz = 40 * 1000 * 1000,
            /* Same panel family as the C3 board: mode 3, inverted, RGB565. */
            .spi_mode = 3,
            .invert_color = true,
            .bgr_order = false,
        };
        dada_err_t err = display_st7789_init(&cfg, display);
        if (err != DADA_OK) {
            ESP_LOGE(TAG, "display init failed (%d)", err);
            return err;
        }
    }

    /* No touch panel on this build: tell the caller so instead of failing. */
    if (input) {
        *input = NULL;
        ESP_LOGI(TAG, "no touch panel on this board; input left NULL");
    }

    return DADA_OK;
}

void board_deinit(dada_display_t *display, dada_input_t *input,
                  dada_storage_t *storage)
{
    if (storage) {
        storage_nvs_deinit(storage);
    }
    if (display) {
        display_st7789_deinit(display);
    }
    (void)input;
}
