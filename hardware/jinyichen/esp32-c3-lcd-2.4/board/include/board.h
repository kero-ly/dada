/**
 * @file board.h
 * @brief Board binding for jinyichen-esp32-c3-lcd-2.4.
 *
 * This is the only place in the tree allowed to know the board's concrete
 * pin numbers and controller models. It reads them from the generated
 * dada_board_config.h, which is produced from board.json at build time.
 */
#pragma once

#include "display.h"
#include "input.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bring up the display and touch panel described by this board's manifest.
 *
 * Either output may be NULL if the caller does not need that capability:
 * the function then returns DADA_OK without initialising it.
 */
dada_err_t board_init(dada_display_t **display, dada_input_t **input);

void board_deinit(dada_display_t *display, dada_input_t *input);

#ifdef __cplusplus
}
#endif
