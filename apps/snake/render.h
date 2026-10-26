/* Snake presentation: grid, body, food and overlays. App-local. */
#pragma once

#include "canvas.h"
#include "snake.h"

typedef struct {
    canvas_t canvas;
    int cell;
    int origin_x;
    int origin_y;
    int grid_w;
    int grid_h;
    int bar;
    uint16_t best;
} snake_view_t;

dada_err_t snake_view_init(snake_view_t *v, dada_display_t *display);
void snake_view_free(snake_view_t *v);

/** Full repaint: score bar, board, body, food. */
dada_err_t snake_view_redraw(snake_view_t *v, const snake_t *s);

/** Repaint only a cell. */
dada_err_t snake_view_cell(snake_view_t *v, int gx, int gy, uint16_t color);

dada_err_t snake_view_food(snake_view_t *v, const snake_t *s);
dada_err_t snake_view_score(snake_view_t *v, uint32_t score);

/* Cell colours owned by the view, so the app never hardcodes palettes. */
#define SNAKE_C_EMPTY 0x0420
#define SNAKE_C_BODY 0x05E0
#define SNAKE_C_HEAD 0x07E0
dada_err_t snake_view_overlay(snake_view_t *v, const snake_t *s);
