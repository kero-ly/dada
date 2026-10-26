#include "canvas.h"

#include <stdlib.h>

static const uint8_t *glyph(char ch)
{
    static const uint8_t n0[] = {14, 17, 19, 21, 25, 17, 14};
    static const uint8_t n1[] = {4, 12, 4, 4, 4, 4, 14};
    static const uint8_t n2[] = {14, 17, 1, 2, 4, 8, 31};
    static const uint8_t n3[] = {31, 2, 4, 2, 1, 17, 14};
    static const uint8_t n4[] = {2, 6, 10, 18, 31, 2, 2};
    static const uint8_t n5[] = {31, 16, 30, 1, 1, 17, 14};
    static const uint8_t n6[] = {6, 8, 16, 30, 17, 17, 14};
    static const uint8_t n7[] = {31, 1, 2, 4, 8, 8, 8};
    static const uint8_t n8[] = {14, 17, 17, 14, 17, 17, 14};
    static const uint8_t n9[] = {14, 17, 17, 15, 1, 2, 12};
    static const uint8_t colon[] = {0, 4, 4, 0, 4, 4, 0};
    static const uint8_t blank[] = {0, 0, 0, 0, 0, 0, 0};

    switch (ch) {
    case '0': return n0; case '1': return n1; case '2': return n2;
    case '3': return n3; case '4': return n4; case '5': return n5;
    case '6': return n6; case '7': return n7; case '8': return n8;
    case '9': return n9; case ':': return colon;
    default: return blank;
    }
}

dada_err_t canvas_init(canvas_t *c, dada_display_t *display)
{
    if (!c || !display || !display->flush || display->color_depth != 16) {
        return DADA_ERR_INVALID_ARG;
    }
    c->display = display;
    c->width = display->width;
    c->height = display->height;
    size_t count = (size_t)c->width * c->height;
    c->pixels = malloc(count * sizeof(uint16_t));
    return c->pixels ? DADA_OK : DADA_ERR_FAIL;
}

void canvas_free(canvas_t *c)
{
    if (c && c->pixels) {
        free(c->pixels);
        c->pixels = NULL;
    }
}

void canvas_clear(canvas_t *c, uint16_t color)
{
    size_t count = (size_t)c->width * c->height;
    for (size_t i = 0; i < count; i++) {
        c->pixels[i] = color;
    }
}

void canvas_fill(canvas_t *c, int x, int y, int w, int h, uint16_t color)
{
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w > c->width ? c->width : x + w;
    int y1 = y + h > c->height ? c->height : y + h;
    for (int row = y0; row < y1; row++) {
        for (int col = x0; col < x1; col++) {
            c->pixels[row * c->width + col] = color;
        }
    }
}

void canvas_frame(canvas_t *c, int x, int y, int w, int h, int thickness, uint16_t color)
{
    canvas_fill(c, x, y, w, thickness, color);
    canvas_fill(c, x, y + h - thickness, w, thickness, color);
    canvas_fill(c, x, y, thickness, h, color);
    canvas_fill(c, x + w - thickness, y, thickness, h, color);
}

dada_err_t canvas_flush(canvas_t *c, int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) {
        return DADA_OK;
    }
    if (x < 0 || y < 0 || x + w > c->width || y + h > c->height) {
        return DADA_ERR_INVALID_ARG;
    }
    uint16_t *packed = malloc((size_t)w * h * sizeof(uint16_t));
    if (!packed) {
        return DADA_ERR_FAIL;
    }
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            packed[row * w + col] = c->pixels[(y + row) * c->width + (x + col)];
        }
    }
    dada_area_t area = {(int16_t)x, (int16_t)y, (int16_t)(x + w - 1), (int16_t)(y + h - 1)};
    dada_err_t err = c->display->flush(c->display->ctx, &area, packed);
    free(packed);
    return err;
}

dada_err_t canvas_flush_all(canvas_t *c)
{
    dada_area_t area = {0, 0, (int16_t)(c->width - 1), (int16_t)(c->height - 1)};
    return c->display->flush(c->display->ctx, &area, c->pixels);
}

int canvas_digits_width(const char *text, int scale)
{
    int length = 0;
    while (text[length]) {
        length++;
    }
    return length ? (length * 6 - 1) * scale : 0;
}

void canvas_digits(canvas_t *c, int x, int y, const char *text, int scale, uint16_t color)
{
    for (int i = 0; text[i]; i++) {
        const uint8_t *rows = glyph(text[i]);
        int gx = x + i * 6 * scale;
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (rows[row] & (1 << (4 - col))) {
                    canvas_fill(c, gx + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}
