/**
 * @file canvas.h
 * @brief Application-side framebuffer helper. Not a platform contract.
 *
 * Keeps a full-frame RGB565 buffer and pushes only the regions an application
 * changed. Shared by applications only; drivers and contracts never use it.
 */
#pragma once

#include "display.h"

/* Rows packed per transfer; keeps the temporary buffer small. */
#define CANVAS_STRIPE_ROWS 16

typedef struct {
    dada_display_t *display;
    uint16_t *pixels;
    int width;
    int height;
} canvas_t;

dada_err_t canvas_init(canvas_t *c, dada_display_t *display);
void canvas_free(canvas_t *c);

void canvas_clear(canvas_t *c, uint16_t color);
void canvas_fill(canvas_t *c, int x, int y, int w, int h, uint16_t color);
void canvas_frame(canvas_t *c, int x, int y, int w, int h, int thickness, uint16_t color);

/** Push a rectangle of the canvas to the panel. */
dada_err_t canvas_flush(canvas_t *c, int x, int y, int w, int h);
dada_err_t canvas_flush_all(canvas_t *c);

/** 5x7 digits and colon; other characters are drawn as blank space. */
void canvas_digits(canvas_t *c, int x, int y, const char *text, int scale, uint16_t color);
int canvas_digits_width(const char *text, int scale);
