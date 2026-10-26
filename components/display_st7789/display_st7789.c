/* ST7789 panel driver built on esp_lcd, exposed through the display contract. */

#include "display_st7789.h"

#include <stdlib.h>
#include <string.h>

#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "display_st7789";

#define LCD_HOST SPI2_HOST
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define BACKLIGHT_LEDC_CHANNEL LEDC_CHANNEL_0
#define BACKLIGHT_LEDC_TIMER LEDC_TIMER_0
#define BACKLIGHT_LEDC_MODE LEDC_LOW_SPEED_MODE

typedef struct {
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
    SemaphoreHandle_t trans_done;
    int backlight_gpio;
    bool spi_bus_owned;
} display_st7789_ctx_t;

static bool on_color_trans_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    (void)io;
    (void)edata;
    display_st7789_ctx_t *ctx = (display_st7789_ctx_t *)user_ctx;
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(ctx->trans_done, &woken);
    return woken == pdTRUE;
}

static dada_err_t contract_flush(void *opaque, const dada_area_t *area, const void *pixels)
{
    display_st7789_ctx_t *ctx = (display_st7789_ctx_t *)opaque;
    if (!ctx || !area || !pixels) {
        return DADA_ERR_INVALID_ARG;
    }

    /* esp_lcd expects exclusive end coordinates. */
    esp_err_t err = esp_lcd_panel_draw_bitmap(ctx->panel, area->x1, area->y1,
                                              area->x2 + 1, area->y2 + 1, pixels);
    if (err != ESP_OK) {
        return DADA_ERR_FAIL;
    }

    /* Stay synchronous with the contract: return only when the panel has the data. */
    if (xSemaphoreTake(ctx->trans_done, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGW(TAG, "flush timeout");
        return DADA_ERR_TIMEOUT;
    }
    return DADA_OK;
}

static dada_err_t contract_set_backlight(void *opaque, uint8_t percent)
{
    display_st7789_ctx_t *ctx = (display_st7789_ctx_t *)opaque;
    if (!ctx) {
        return DADA_ERR_INVALID_ARG;
    }
    if (ctx->backlight_gpio < 0) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    if (percent > 100) {
        percent = 100;
    }
    uint32_t duty = (uint32_t)((255u * percent) / 100u);
    ledc_set_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL, duty);
    ledc_update_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL);
    return DADA_OK;
}

static void backlight_ledc_init(int gpio)
{
    ledc_timer_config_t timer = {
        .speed_mode = BACKLIGHT_LEDC_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = BACKLIGHT_LEDC_TIMER,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t ch = {
        .gpio_num = gpio,
        .speed_mode = BACKLIGHT_LEDC_MODE,
        .channel = BACKLIGHT_LEDC_CHANNEL,
        .timer_sel = BACKLIGHT_LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch));
}

dada_err_t display_st7789_init(const display_st7789_config_t *cfg, dada_display_t **out)
{
    if (!cfg || !out || cfg->width <= 0 || cfg->height <= 0) {
        return DADA_ERR_INVALID_ARG;
    }

    display_st7789_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) {
        return DADA_ERR_FAIL;
    }
    ctx->backlight_gpio = cfg->backlight_gpio;

    ctx->trans_done = xSemaphoreCreateBinary();
    if (!ctx->trans_done) {
        free(ctx);
        return DADA_ERR_FAIL;
    }

    /* Backlight first: an unlit panel looks like a broken driver. */
    if (cfg->backlight_gpio >= 0) {
        backlight_ledc_init(cfg->backlight_gpio);
    }

    spi_bus_config_t bus_cfg = {
        .sclk_io_num = cfg->sclk_gpio,
        .mosi_io_num = cfg->mosi_gpio,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = cfg->width * cfg->height * 2,
    };
    esp_err_t err = spi_bus_initialize(LCD_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (err == ESP_OK) {
        ctx->spi_bus_owned = true;
    } else if (err != ESP_ERR_INVALID_STATE) { /* already initialized is fine */
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(err));
        vSemaphoreDelete(ctx->trans_done);
        free(ctx);
        return DADA_ERR_FAIL;
    }

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = cfg->cs_gpio,
        .dc_gpio_num = cfg->dc_gpio,
        .spi_mode = cfg->spi_mode,
        .pclk_hz = cfg->pclk_hz,
        .trans_queue_depth = 10,
        .on_color_trans_done = on_color_trans_done,
        .user_ctx = ctx,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };
    if (esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_cfg, &ctx->io) != ESP_OK) {
        ESP_LOGE(TAG, "panel io init failed");
        goto fail;
    }

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = cfg->rst_gpio,
        .bits_per_pixel = 16,
        /* The display contract supplies little-endian RGB565 pixels. */
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
    };
    panel_cfg.rgb_ele_order = cfg->bgr_order ? LCD_RGB_ELEMENT_ORDER_BGR : LCD_RGB_ELEMENT_ORDER_RGB;
    if (esp_lcd_new_panel_st7789(ctx->io, &panel_cfg, &ctx->panel) != ESP_OK) {
        ESP_LOGE(TAG, "panel init failed");
        goto fail;
    }

    if (esp_lcd_panel_reset(ctx->panel) != ESP_OK ||
        esp_lcd_panel_init(ctx->panel) != ESP_OK) {
        ESP_LOGE(TAG, "panel reset/init failed");
        goto fail;
    }

    esp_lcd_panel_invert_color(ctx->panel, cfg->invert_color);
    esp_lcd_panel_disp_on_off(ctx->panel, true);

    dada_display_t *display = calloc(1, sizeof(*display));
    if (!display) {
        goto fail;
    }
    display->width = (uint16_t)cfg->width;
    display->height = (uint16_t)cfg->height;
    display->color_depth = 16;
    display->ctx = ctx;
    display->flush = contract_flush;
    display->set_backlight = contract_set_backlight;

    if (cfg->backlight_gpio >= 0) {
        contract_set_backlight(ctx, 100);
    }

    *out = display;
    ESP_LOGI(TAG, "ST7789 %dx%d ready", cfg->width, cfg->height);
    return DADA_OK;

fail:
    if (ctx->trans_done) {
        vSemaphoreDelete(ctx->trans_done);
    }
    free(ctx);
    return DADA_ERR_FAIL;
}

void display_st7789_deinit(dada_display_t *display)
{
    if (!display) {
        return;
    }
    display_st7789_ctx_t *ctx = (display_st7789_ctx_t *)display->ctx;
    if (ctx) {
        if (ctx->panel) {
            esp_lcd_panel_del(ctx->panel);
        }
        if (ctx->io) {
            esp_lcd_panel_io_del(ctx->io);
        }
        if (ctx->spi_bus_owned) {
            spi_bus_free(LCD_HOST);
        }
        if (ctx->trans_done) {
            vSemaphoreDelete(ctx->trans_done);
        }
        free(ctx);
    }
    free(display);
}
