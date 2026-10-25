/**
 * @file display_st7789.h
 * @brief ST7789 SPI panel driver implementing the dada display contract.
 *
 * Wraps ESP-IDF's esp_lcd: bus + panel IO + panel. Board-specific pins come from
 * the generated board header, never from this component.
 */
#pragma once

#include "display.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* SPI bus */
    int sclk_gpio;
    int mosi_gpio;
    int cs_gpio;      /* -1 when CS is tied to GND on the PCB */
    int dc_gpio;
    int rst_gpio;     /* -1 when the panel is reset by the board */
    int backlight_gpio; /* -1 when backlight is not controllable */

    /* panel */
    int width;
    int height;
    uint32_t pclk_hz; /* e.g. 40 * 1000 * 1000 */
    uint8_t spi_mode; /* 0..3 */
    bool invert_color;
    bool bgr_order;   /* true for panels wired as BGR */
} display_st7789_config_t;

/**
 * Bring up the panel and return a contract handle.
 *
 * The caller owns the handle and must release it with display_st7789_deinit().
 */
dada_err_t display_st7789_init(const display_st7789_config_t *cfg, dada_display_t **out);

void display_st7789_deinit(dada_display_t *display);

#ifdef __cplusplus
}
#endif
