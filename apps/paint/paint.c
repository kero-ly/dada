#include "paint.h"

#include <string.h>

bool paint_init(paint_t *p, int cols, int rows)
{
    if (!p || cols <= 0 || rows <= 0 ||
        cols > PAINT_MAX_COLS || rows > PAINT_MAX_ROWS) {
        return false;
    }
    p->cols = cols;
    p->rows = rows;
    memset(p->cells, PAINT_COLOR_BG, sizeof(p->cells));
    return true;
}

uint8_t paint_get(const paint_t *p, int x, int y)
{
    if (!p || x < 0 || y < 0 || x >= p->cols || y >= p->rows) {
        return PAINT_COLOR_BG;
    }
    return p->cells[y * p->cols + x];
}

bool paint_set(paint_t *p, int x, int y, uint8_t color)
{
    if (!p || x < 0 || y < 0 || x >= p->cols || y >= p->rows) {
        return false;
    }
    if (color >= PAINT_PALETTE_SIZE) {
        return false;
    }
    p->cells[y * p->cols + x] = color;
    return true;
}

/* Bresenham: a swipe samples far apart cells, so join them.
 * Endpoints may fall outside the bitmap (a finger leaving the panel); the line
 * is clipped instead of rejected. Returns true if any cell was painted. */
bool paint_stroke(paint_t *p, int x0, int y0, int x1, int y1, uint8_t color)
{
    if (!p) {
        return false;
    }
    bool painted = paint_set(p, x0, y0, color);
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    while (x0 != x1 || y0 != y1) {
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
        painted = paint_set(p, x0, y0, color) || painted;
    }
    return painted;
}

void paint_clear(paint_t *p)
{
    if (!p) {
        return;
    }
    memset(p->cells, PAINT_COLOR_BG, sizeof(p->cells));
}

uint32_t paint_cell_count(const paint_t *p)
{
    return p ? (uint32_t)(p->cols * p->rows) : 0;
}

uint32_t paint_packed_size(const paint_t *p)
{
    return p ? (paint_cell_count(p) + 1) / 2 : 0;
}

uint32_t paint_pack(const paint_t *p, uint8_t *out, uint32_t capacity)
{
    if (!p || !out) {
        return 0;
    }
    uint32_t needed = paint_packed_size(p);
    if (needed == 0 || capacity < needed) {
        return 0;
    }
    uint32_t cells = paint_cell_count(p);
    for (uint32_t i = 0; i < needed; i++) {
        uint8_t hi = (uint8_t)(p->cells[i * 2] & 0x0F);
        uint8_t lo = (i * 2 + 1 < cells) ? (uint8_t)(p->cells[i * 2 + 1] & 0x0F) : 0;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return needed;
}

bool paint_unpack(paint_t *p, const uint8_t *in, uint32_t length)
{
    if (!p || !in) {
        return false;
    }
    if (length != paint_packed_size(p)) {
        return false;
    }
    uint32_t cells = paint_cell_count(p);
    for (uint32_t i = 0; i < length; i++) {
        p->cells[i * 2] = (uint8_t)(in[i] >> 4);
        if (i * 2 + 1 < cells) {
            p->cells[i * 2 + 1] = (uint8_t)(in[i] & 0x0F);
        }
    }
    return true;
}
