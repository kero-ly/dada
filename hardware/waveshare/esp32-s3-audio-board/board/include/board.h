/**
 * @file board.h
 * @brief Board binding for the Waveshare ESP32-S3-AUDIO-Board with a 2-inch
 *        ST7789 module wired to the general-purpose header.
 *
 * This board has no touch panel: the input service is left NULL. The built-in
 * 18-pin LCD connector routes RST through a TCA9555 expander, so the display is
 * wired with RST tied high and reset through the driver's software reset.
 */
#pragma once

#include "display.h"
#include "input.h"
#include "storage.h"
#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

dada_err_t board_init(dada_display_t **display, dada_input_t **input,
                      dada_time_t **time, dada_storage_t **storage);

void board_deinit(dada_display_t *display, dada_input_t *input,
                  dada_storage_t *storage);

#ifdef __cplusplus
}
#endif
