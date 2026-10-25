/**
 * @file touch_cst836u.h
 * @brief CST836U capacitive touch driver (I2C) implementing the input contract.
 *
 * Register layout used here comes from the vendor's CST836U datasheet:
 *   0x02 status / touch count, 0x03..0x06 X and Y (12-bit, split high/low).
 */
#pragma once

#include "input.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int scl_gpio;
    int sda_gpio;
    int int_gpio;  /* -1 when the interrupt line is not wired */
    int rst_gpio;  /* -1 when reset is not controllable */
    uint8_t i2c_address; /* 0x15 on the reference board, 0x5D on some batches */
    uint32_t scl_speed_hz;
    uint16_t width;
    uint16_t height;
    bool swap_xy;
    bool invert_x;
    bool invert_y;
} touch_cst836u_config_t;

dada_err_t touch_cst836u_init(const touch_cst836u_config_t *cfg, dada_input_t **out);

void touch_cst836u_deinit(dada_input_t *input);

#ifdef __cplusplus
}
#endif
