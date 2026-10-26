/* Hardware diagnostics through the same contracts as any app.
 *
 * Draws a numbered 3x3 grid and paints a marker where the panel is touched, so
 * touch coordinates can be checked against what the user actually sees.
 * Everything goes through display/input contracts: no pin numbers here.
 */
#include "app.h"

#include <stdio.h>
#include <stdlib.h>

#define GRID 3
#define MARKER 11 /* side of the red square drawn at the touch point */

static dada_app_services_t services;
static uint16_t *canvas;
static int width, height;
static bool active, was_pressed;

/* 5x7 bitmap font for 1..9, one row per byte, MSB is the left pixel. */
static const uint8_t digit_rows[9][7] = {
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* 1 */
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, /* 2 */
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, /* 3 */
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, /* 4 */
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, /* 5 */
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, /* 6 */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, /* 7 */
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, /* 8 */
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, /* 9 */
};

static void put_pixel(int x, int y, uint16_t color)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        canvas[y * width + x] = color;
    }
}

static void fill_rect(int x, int y, int w, int h, uint16_t color)
{
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            put_pixel(x + i, y + j, color);
        }
    }
}

static void draw_digit(int value, int cx, int cy, int scale, uint16_t color)
{
    if (value < 1 || value > 9) {
        return;
    }
    const uint8_t *rows = digit_rows[value - 1];
    int x0 = cx - (5 * scale) / 2;
    int y0 = cy - (7 * scale) / 2;
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (rows[row] & (0x10 >> col)) {
                fill_rect(x0 + col * scale, y0 + row * scale, scale, scale, color);
            }
        }
    }
}

/* The contract wants packed w*h pixels, so a sub-rectangle has to be copied
 * out of the full-frame canvas first. */
static dada_err_t flush_rect(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) {
        return DADA_ERR_INVALID_ARG;
    }
    uint16_t *packed = malloc((size_t)w * h * sizeof(uint16_t));
    if (!packed) {
        return DADA_ERR_FAIL;
    }
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            packed[row * w + col] = canvas[(y + row) * width + (x + col)];
        }
    }
    dada_area_t area = {(int16_t)x, (int16_t)y, (int16_t)(x + w - 1), (int16_t)(y + h - 1)};
    dada_err_t err = services.display->flush(services.display->ctx, &area, packed);
    free(packed);
    return err;
}

static void paint_full_frame(void)
{
    const uint16_t bg = 0xFFFF, line = 0x0000, ink = 0x001F;
    int cell_w = width / GRID;
    int cell_h = height / GRID;

    fill_rect(0, 0, width, height, bg);

    for (int i = 1; i < GRID; i++) {
        fill_rect(i * cell_w - 1, 0, 2, height, line);
        fill_rect(0, i * cell_h - 1, width, 2, line);
    }
    fill_rect(0, 0, width, 2, line);
    fill_rect(0, height - 2, width, 2, line);
    fill_rect(0, 0, 2, height, line);
    fill_rect(width - 2, 0, 2, height, line);

    int scale = (cell_h < cell_w ? cell_h : cell_w) / 14;
    if (scale < 2) {
        scale = 2;
    }
    for (int row = 0; row < GRID; row++) {
        for (int col = 0; col < GRID; col++) {
            int n = row * GRID + col + 1;
            draw_digit(n, col * cell_w + cell_w / 2, row * cell_h + cell_h / 2, scale, ink);
        }
    }
}

dada_err_t dada_app_start(const dada_app_services_t *provided)
{
    dada_app_stop();
    if (!provided || !provided->display || !provided->display->flush ||
        !provided->input || !provided->input->poll) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    if (provided->display->color_depth != 16) {
        return DADA_ERR_NOT_SUPPORTED;
    }

    width = provided->display->width;
    height = provided->display->height;
    canvas = malloc((size_t)width * height * sizeof(uint16_t));
    if (!canvas) {
        return DADA_ERR_FAIL;
    }

    services = *provided;
    paint_full_frame();

    dada_area_t full = {0, 0, (int16_t)(width - 1), (int16_t)(height - 1)};
    dada_err_t err = services.display->flush(services.display->ctx, &full, canvas);
    if (err != DADA_OK) {
        free(canvas);
        canvas = NULL;
        return err;
    }

    printf("bringup: %dx%d grid ready, tap each numbered cell\n", width, height);
    was_pressed = false;
    active = true;
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    if (!active) {
        return DADA_ERR_FAIL;
    }
    dada_point_t point;
    bool pressed;
    if (!services.input->poll(services.input->ctx, &point, &pressed)) {
        return DADA_OK;
    }
    if (pressed == was_pressed) {
        return DADA_OK;
    }
    was_pressed = pressed;

    if (!pressed) {
        printf("bringup: release\n");
        return DADA_OK;
    }

    int col = (int)point.x * GRID / width + 1;
    int row = (int)point.y * GRID / height + 1;
    if (col > GRID) col = GRID;
    if (row > GRID) row = GRID;

    /* Paint the marker on top of the stored frame and push only that area. */
    int mx = point.x - MARKER / 2;
    int my = point.y - MARKER / 2;
    int x0 = mx < 0 ? 0 : mx;
    int y0 = my < 0 ? 0 : my;
    int x1 = mx + MARKER > width ? width : mx + MARKER;
    int y1 = my + MARKER > height ? height : my + MARKER;
    fill_rect(x0, y0, x1 - x0, y1 - y0, 0xF800);
    flush_rect(x0, y0, x1 - x0, y1 - y0);

    printf("bringup: tap (%d, %d) -> cell %d (row %d col %d)\n", point.x, point.y,
           (row - 1) * GRID + col, row, col);
    return DADA_OK;
}

void dada_app_stop(void)
{
    active = false;
    if (canvas) {
        free(canvas);
        canvas = NULL;
    }
    services = (dada_app_services_t){0};
}
