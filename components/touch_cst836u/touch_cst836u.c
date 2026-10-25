/* CST836U touch driver: I2C polling, exposed through the input contract. */

#include "touch_cst836u.h"

#include <stdlib.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "touch_cst836u";

#define CST_REG_STATUS 0x02
#define CST_READ_LEN 5

typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t dev;
    int rst_gpio;
    uint16_t width;
    uint16_t height;
    bool swap_xy;
    bool invert_x;
    bool invert_y;
    dada_point_t last;
} touch_ctx_t;

static bool contract_poll(void *opaque, dada_point_t *point, bool *pressed)
{
    touch_ctx_t *ctx = (touch_ctx_t *)opaque;
    if (!ctx || !point || !pressed) {
        return false;
    }

    uint8_t reg = CST_REG_STATUS;
    uint8_t buf[CST_READ_LEN] = {0};
    if (i2c_master_transmit_receive(ctx->dev, &reg, 1, buf, CST_READ_LEN, 20) != ESP_OK) {
        *pressed = false;
        *point = ctx->last;
        return false;
    }

    uint8_t points = buf[0] & 0x0F;
    if (points == 0) {
        *pressed = false;
        *point = ctx->last;
        return true;
    }

    uint16_t raw_x = (uint16_t)(((buf[1] & 0x0F) << 8) | buf[2]);
    uint16_t raw_y = (uint16_t)(((buf[3] & 0x0F) << 8) | buf[4]);

    if (raw_x > ctx->width || raw_y > ctx->height) {
        *pressed = false;
        *point = ctx->last;
        return true;
    }

    int16_t x = (int16_t)raw_x;
    int16_t y = (int16_t)raw_y;
    if (ctx->swap_xy) {
        int16_t tmp = x;
        x = y;
        y = tmp;
    }
    if (ctx->invert_x) {
        x = (int16_t)(ctx->width - 1 - x);
    }
    if (ctx->invert_y) {
        y = (int16_t)(ctx->height - 1 - y);
    }

    ctx->last.x = x;
    ctx->last.y = y;
    *point = ctx->last;
    *pressed = true;
    return true;
}

dada_err_t touch_cst836u_init(const touch_cst836u_config_t *cfg, dada_input_t **out)
{
    if (!cfg || !out) {
        return DADA_ERR_INVALID_ARG;
    }

    touch_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) {
        return DADA_ERR_FAIL;
    }
    ctx->rst_gpio = cfg->rst_gpio;
    ctx->width = cfg->width;
    ctx->height = cfg->height;
    ctx->swap_xy = cfg->swap_xy;
    ctx->invert_x = cfg->invert_x;
    ctx->invert_y = cfg->invert_y;

    if (cfg->rst_gpio >= 0) {
        gpio_config_t io = {
            .pin_bit_mask = 1ULL << cfg->rst_gpio,
            .mode = GPIO_MODE_OUTPUT,
        };
        gpio_config(&io);
        gpio_set_level(cfg->rst_gpio, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level(cfg->rst_gpio, 1);
        vTaskDelay(pdMS_TO_TICKS(60));
    }

    if (cfg->int_gpio >= 0) {
        gpio_config_t io = {
            .pin_bit_mask = 1ULL << cfg->int_gpio,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
        };
        gpio_config(&io);
    }

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = cfg->sda_gpio,
        .scl_io_num = cfg->scl_gpio,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    if (i2c_new_master_bus(&bus_cfg, &ctx->bus) != ESP_OK) {
        ESP_LOGE(TAG, "i2c bus init failed");
        free(ctx);
        return DADA_ERR_FAIL;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = cfg->i2c_address,
        .scl_speed_hz = cfg->scl_speed_hz,
    };
    if (i2c_master_bus_add_device(ctx->bus, &dev_cfg, &ctx->dev) != ESP_OK) {
        ESP_LOGE(TAG, "i2c device add failed");
        i2c_del_master_bus(ctx->bus);
        free(ctx);
        return DADA_ERR_FAIL;
    }

    /* Probe once so a wrong address fails at bring-up, not on first touch. */
    uint8_t reg = CST_REG_STATUS;
    uint8_t probe[CST_READ_LEN] = {0};
    esp_err_t err = i2c_master_transmit_receive(ctx->dev, &reg, 1, probe, CST_READ_LEN, 50);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "no answer at 0x%02X (%s)", cfg->i2c_address, esp_err_to_name(err));
        i2c_master_bus_rm_device(ctx->dev);
        i2c_del_master_bus(ctx->bus);
        free(ctx);
        return DADA_ERR_FAIL;
    }
    ESP_LOGI(TAG, "CST836U at 0x%02X, touches=%u", cfg->i2c_address, probe[0] & 0x0F);

    dada_input_t *input = calloc(1, sizeof(*input));
    if (!input) {
        i2c_master_bus_rm_device(ctx->dev);
        i2c_del_master_bus(ctx->bus);
        free(ctx);
        return DADA_ERR_FAIL;
    }
    input->ctx = ctx;
    input->poll = contract_poll;
    *out = input;
    return DADA_OK;
}

void touch_cst836u_deinit(dada_input_t *input)
{
    if (!input) {
        return;
    }
    touch_ctx_t *ctx = (touch_ctx_t *)input->ctx;
    if (ctx) {
        if (ctx->dev) {
            i2c_master_bus_rm_device(ctx->dev);
        }
        if (ctx->bus) {
            i2c_del_master_bus(ctx->bus);
        }
        free(ctx);
    }
    free(input);
}
