/* App-local seven-segment digits and a tiny fixed-label font. */
#include "render.h"

#include <limits.h>
#include <stdlib.h>

#define STRIPE_ROWS 8
#define BG 0x0841
#define FG 0xFFFF
#define ACCENT 0x07FF
#define RESET_BG 0x2104

static dada_display_t *panel;
static uint16_t *stripe;

static dada_err_t rect(int x, int y, int w, int h, uint16_t color)
{
    /* All coordinates originate from relative layout below. */
    if (w <= 0 || h <= 0) return DADA_OK;
    if (x < 0 || y < 0 || x + w > panel->width || y + h > panel->height)
        return DADA_ERR_INVALID_ARG;
    for (int i = 0; i < w * STRIPE_ROWS; i++) stripe[i] = color;
    for (int row = 0; row < h; row += STRIPE_ROWS) {
        int rows = h - row < STRIPE_ROWS ? h - row : STRIPE_ROWS;
        dada_area_t area = {x, y + row, x + w - 1, y + row + rows - 1};
        dada_err_t err = panel->flush(panel->ctx, &area, stripe);
        if (err != DADA_OK) return err;
    }
    return DADA_OK;
}

/* 5-bit rows for only the fixed labels this application needs. */
static const uint8_t *glyph(char c)
{
    static const uint8_t r[] = {30,17,17,30,20,18,17};
    static const uint8_t e[] = {31,16,16,30,16,16,31};
    static const uint8_t s[] = {15,16,16,14,1,1,30};
    static const uint8_t t[] = {31,4,4,4,4,4,4};
    static const uint8_t a[] = {14,17,17,31,17,17,17};
    static const uint8_t p[] = {30,17,17,30,16,16,16};
    static const uint8_t plus[] = {0,4,4,31,4,4,0};
    static const uint8_t space[] = {0,0,0,0,0,0,0};
    switch (c) {
        case 'R': return r; case 'E': return e; case 'S': return s;
        case 'T': return t; case 'A': return a; case 'P': return p;
        case '+': return plus; default: return space;
    }
}

static dada_err_t label(const char *text, int y, int scale, uint16_t color)
{
    int length = 0;
    while (text[length]) length++;
    int x = (panel->width - (length * 6 - 1) * scale) / 2;
    for (int i = 0; i < length; i++) {
        const uint8_t *rows = glyph(text[i]);
        for (int row = 0; row < 7; row++) for (int col = 0; col < 5; col++) {
            if (!(rows[row] & (1 << (4 - col)))) continue;
            dada_err_t err = rect(x + (i * 6 + col) * scale, y + row * scale,
                                  scale, scale, color);
            if (err != DADA_OK) return err;
        }
    }
    return DADA_OK;
}

void counter_render_stop(void)
{
    free(stripe);
    stripe = NULL;
    panel = NULL;
}

dada_err_t counter_render_start(dada_display_t *display)
{
    counter_render_stop();
    if (display->color_depth != 16 || display->width < 160 || display->height < 160 ||
        display->width > INT16_MAX || display->height > INT16_MAX) return DADA_ERR_NOT_SUPPORTED;
    stripe = malloc(display->width * STRIPE_ROWS * sizeof(*stripe));
    if (!stripe) return DADA_ERR_FAIL;
    panel = display;
    int reset_y = panel->height * 3 / 4;
    dada_err_t err = rect(0, 0, panel->width, panel->height, BG);
    if (err == DADA_OK) err = rect(0, reset_y, panel->width, panel->height - reset_y, RESET_BG);
    if (err == DADA_OK) err = label("TAP +", 12, 3, ACCENT);
    if (err == DADA_OK) err = label("RESET", reset_y + (panel->height - reset_y - 21) / 2, 3, FG);
    if (err != DADA_OK) counter_render_stop();
    return err;
}

dada_err_t counter_render_value(uint16_t value)
{
    static const uint8_t segments[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
    int gap = 6, w = (panel->width - 24 - 3 * gap) / 4;
    int h = panel->height / 3;
    /* A wide, short panel must not turn horizontal width into oversized strokes. */
    if (w > h * 2 / 3) w = h * 2 / 3;
    int thick = w / 7;
    int y = panel->height / 3 - h / 4;
    dada_err_t err = rect(0, y, panel->width, h, BG);
    if (err != DADA_OK) return err;
    int divisor = 1000;
    for (int d = 0; d < 4; d++, divisor /= 10) {
        int x = (panel->width - (w * 4 + gap * 3)) / 2 + d * (w + gap);
        uint8_t mask = segments[(value / divisor) % 10];
        const int parts[7][4] = {
            {x + thick, y, w - 2 * thick, thick},
            {x + w - thick, y + thick, thick, h / 2 - thick},
            {x + w - thick, y + h / 2, thick, h / 2 - thick},
            {x + thick, y + h - thick, w - 2 * thick, thick},
            {x, y + h / 2, thick, h / 2 - thick},
            {x, y + thick, thick, h / 2 - thick},
            {x + thick, y + h / 2 - thick / 2, w - 2 * thick, thick},
        };
        for (int bit = 0; bit < 7; bit++) if (mask & (1 << bit)) {
            const int *p = parts[bit];
            err = rect(p[0], p[1], p[2], p[3], FG);
            if (err != DADA_OK) return err;
        }
    }
    return DADA_OK;
}
