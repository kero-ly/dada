/**
 * @file paint.h
 * @brief Pixel-art bitmap rules: a grid of colour indices. No UI, no hardware.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PAINT_MAX_COLS 48
#define PAINT_MAX_ROWS 64
#define PAINT_PALETTE_SIZE 16

typedef struct {
    int cols;
    int rows;
    uint8_t cells[PAINT_MAX_COLS * PAINT_MAX_ROWS];
} paint_t;

/** Index 0 is the background; 1..6 are the palette; 7 is the clear action. */
typedef enum {
    PAINT_COLOR_BG = 0,
    PAINT_COLOR_1,
    PAINT_COLOR_2,
    PAINT_COLOR_3,
    PAINT_COLOR_4,
    PAINT_COLOR_5,
    PAINT_COLOR_6,
    PAINT_ACTION_CLEAR = 7,
} paint_color_t;

bool paint_init(paint_t *p, int cols, int rows);

/** Cell access; out-of-range reads are background and writes are ignored. */
uint8_t paint_get(const paint_t *p, int x, int y);
bool paint_set(paint_t *p, int x, int y, uint8_t color);

/** Draw a line between two cells so fast swipes leave no gaps. */
bool paint_stroke(paint_t *p, int x0, int y0, int x1, int y1, uint8_t color);

void paint_clear(paint_t *p);

/** Number of cells in the bitmap. */
uint32_t paint_cell_count(const paint_t *p);

/** Bytes needed to persist the bitmap: two 4-bit cells per byte. */
uint32_t paint_packed_size(const paint_t *p);

/** Pack into @p out; returns bytes written, or 0 when the buffer is too small. */
uint32_t paint_pack(const paint_t *p, uint8_t *out, uint32_t capacity);

/** Restore from packed bytes; returns false on a size mismatch. */
bool paint_unpack(paint_t *p, const uint8_t *in, uint32_t length);
