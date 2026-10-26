#include "render.h"

#include <string.h>

#define BG 0x0320       /* board */
#define BAR 0x18E3      /* score bar */
#define HEAD 0x07E0     /* snake head */
#define BODY 0x05E0     /* snake body */
#define FOOD 0xF800     /* food */
#define GRID 0x0420     /* faint grid */
#define TEXT 0xFFFF
#define OVER_BORDER 0xF800

dada_err_t snake_view_init(snake_view_t *v, dada_display_t *display)
{
    memset(v, 0, sizeof(*v));
    dada_err_t err = canvas_init(&v->canvas, display);
    if (err != DADA_OK) {
        return err;
    }
    /* 12x14 cells, sized to whatever panel the board provides. */
    int bar = 40;
    int cell_x_ = display->width / 12;
    int cell_y_ = (display->height - bar) / 14;
    v->cell = cell_x_ < cell_y_ ? cell_x_ : cell_y_;
    v->origin_x = (display->width - v->cell * 12) / 2;
    v->origin_y = bar + ((display->height - bar) - v->cell * 14) / 2;
    v->grid_w = 12;
    v->grid_h = 14;
    v->bar = bar;
    return DADA_OK;
}

void snake_view_free(snake_view_t *v)
{
    canvas_free(&v->canvas);
}

static int cell_x(const snake_view_t *v, int gx) { return v->origin_x + gx * v->cell; }
static int cell_y(const snake_view_t *v, int gy) { return v->origin_y + gy * v->cell; }

dada_err_t snake_view_cell(snake_view_t *v, int gx, int gy, uint16_t color)
{
    int x = cell_x(v, gx);
    int y = cell_y(v, gy);
    canvas_fill(&v->canvas, x + 1, y + 1, v->cell - 2, v->cell - 2, color);
    return canvas_flush(&v->canvas, x, y, v->cell, v->cell);
}

static void draw_board(snake_view_t *v)
{
    canvas_fill(&v->canvas, 0, 0, v->canvas.width, v->canvas.height, BG);
    canvas_fill(&v->canvas, 0, 0, v->canvas.width, v->bar, BAR);
}

static int put_number(char *out, uint32_t value);

dada_err_t snake_view_redraw(snake_view_t *v, const snake_t *s)
{
    draw_board(v);
    for (int gy = 0; gy < s->height; gy++) {
        for (int gx = 0; gx < s->width; gx++) {
            canvas_fill(&v->canvas, cell_x(v, gx) + 1, cell_y(v, gy) + 1,
                        v->cell - 2, v->cell - 2, GRID);
        }
    }
    for (int i = s->length - 1; i >= 0; i--) {
        int x = cell_x(v, s->body_x[i]);
        int y = cell_y(v, s->body_y[i]);
        canvas_fill(&v->canvas, x + 1, y + 1, v->cell - 2, v->cell - 2,
                    i == 0 ? HEAD : BODY);
    }
    int fx = cell_x(v, s->food_x);
    int fy = cell_y(v, s->food_y);
    canvas_fill(&v->canvas, fx + 4, fy + 4, v->cell - 8, v->cell - 8, FOOD);

    char buf[12];
    put_number(buf, s->score);
    canvas_digits(&v->canvas, 8, 10, buf, 3, TEXT);
    return canvas_flush_all(&v->canvas);
}

dada_err_t snake_view_food(snake_view_t *v, const snake_t *s)
{
    int fx = cell_x(v, s->food_x);
    int fy = cell_y(v, s->food_y);
    canvas_fill(&v->canvas, fx + 1, fy + 1, v->cell - 2, v->cell - 2, GRID);
    canvas_fill(&v->canvas, fx + 4, fy + 4, v->cell - 8, v->cell - 8, FOOD);
    return canvas_flush(&v->canvas, fx, fy, v->cell, v->cell);
}

static int put_number(char *out, uint32_t value)
{
    char rev[10];
    int n = 0, r = 0;
    if (value == 0) {
        rev[r++] = '0';
    }
    while (value) {
        rev[r++] = (char)('0' + value % 10);
        value /= 10;
    }
    while (r) {
        out[n++] = rev[--r];
    }
    out[n] = 0;
    return n;
}

dada_err_t snake_view_score(snake_view_t *v, uint32_t score)
{
    canvas_fill(&v->canvas, 0, 0, v->canvas.width, v->bar, BAR);
    char buf[12];
    put_number(buf, score);
    canvas_digits(&v->canvas, 8, 10, buf, 3, TEXT);
    return canvas_flush(&v->canvas, 0, 0, v->canvas.width, v->bar);
}

dada_err_t snake_view_overlay(snake_view_t *v, const snake_t *s)
{
    int board_y = v->origin_y;
    int board_h = v->cell * v->grid_h;
    if (s->state == SNAKE_READY) {
        /* White frame plus a play triangle: no font needed. */
        int box_w = v->canvas.width - 80;
        int box_h = 110;
        int box_x = 40;
        int box_y = board_y + (board_h - box_h) / 2;
        canvas_frame(&v->canvas, box_x, box_y, box_w, box_h, 3, TEXT);
        int cx = v->canvas.width / 2;
        int cy = box_y + box_h / 2;
        for (int i = 0; i < 22; i++) {
            canvas_fill(&v->canvas, cx - 10 + i, cy - 22 + i, 2, 44 - i * 2, TEXT);
        }
        return canvas_flush(&v->canvas, box_x, box_y, box_w, box_h);
    }
    /* Game over: red frame around the board, clipped to the canvas. */
    canvas_frame(&v->canvas, 2, board_y - 2, v->canvas.width - 4, board_h + 4, 3, OVER_BORDER);
    int flush_y = board_y - 4;
    if (flush_y < 0) {
        flush_y = 0;
    }
    int flush_h = board_y + board_h + 4 - flush_y;
    if (flush_y + flush_h > v->canvas.height) {
        flush_h = v->canvas.height - flush_y;
    }
    return canvas_flush(&v->canvas, 0, flush_y, v->canvas.width, flush_h);
}
