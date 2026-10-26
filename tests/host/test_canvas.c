/* Host tests for the framing helper: striped flushes and bounds. */
#include <assert.h>
#include <stdio.h>

#include "canvas.h"

static int flush_calls;
static int flushed_pixels;
static int max_rows_per_call;

static dada_err_t flush(void *ctx, const dada_area_t *a, const void *pixels)
{
    (void)ctx;
    assert(pixels);
    assert(a->x1 >= 0 && a->y1 >= 0);
    assert(a->x2 >= a->x1 && a->y2 >= a->y1);
    assert(a->x2 < 240 && a->y2 < 320);
    int rows = a->y2 - a->y1 + 1;
    int cols = a->x2 - a->x1 + 1;
    flushed_pixels += rows * cols;
    if (rows > max_rows_per_call) max_rows_per_call = rows;
    flush_calls++;
    return DADA_OK;
}

static dada_display_t display = {
    .width = 240, .height = 320, .color_depth = 16, .flush = flush};

static void test_full_region_is_striped(void)
{
    flush_calls = flushed_pixels = max_rows_per_call = 0;
    canvas_t c;
    assert(canvas_init(&c, &display) == DADA_OK);

    assert(canvas_flush(&c, 0, 0, 240, 320) == DADA_OK);
    /* Everything must go out exactly once. */
    assert(flushed_pixels == 240 * 320);
    /* A whole-frame flush must not need a whole-frame temporary buffer. */
    assert(max_rows_per_call <= CANVAS_STRIPE_ROWS);
    assert(flush_calls == (320 + CANVAS_STRIPE_ROWS - 1) / CANVAS_STRIPE_ROWS);
    canvas_free(&c);
}

static void test_odd_height_is_covered(void)
{
    flush_calls = flushed_pixels = max_rows_per_call = 0;
    canvas_t c;
    assert(canvas_init(&c, &display) == DADA_OK);
    assert(canvas_flush(&c, 5, 7, 100, 37) == DADA_OK);   /* 37 = 2*16 + 5 */
    assert(flushed_pixels == 100 * 37);
    assert(max_rows_per_call <= CANVAS_STRIPE_ROWS);
    canvas_free(&c);
}

static void test_rejects_out_of_range(void)
{
    canvas_t c;
    assert(canvas_init(&c, &display) == DADA_OK);
    assert(canvas_flush(&c, 200, 0, 80, 10) == DADA_ERR_INVALID_ARG);
    assert(canvas_flush(&c, 0, 300, 10, 40) == DADA_ERR_INVALID_ARG);
    assert(canvas_flush(&c, -1, 0, 10, 10) == DADA_ERR_INVALID_ARG);
    /* Empty rectangles are a no-op, not an error. */
    assert(canvas_flush(&c, 0, 0, 0, 0) == DADA_OK);
    canvas_free(&c);
}

static void test_pixels_come_from_the_right_place(void)
{
    canvas_t c;
    assert(canvas_init(&c, &display) == DADA_OK);
    canvas_clear(&c, 0x0000);
    canvas_fill(&c, 10, 10, 4, 4, 0xABCD);
    assert(c.pixels[10 * 240 + 10] == 0xABCD);
    assert(c.pixels[13 * 240 + 13] == 0xABCD);
    assert(c.pixels[9 * 240 + 10] == 0x0000);
    assert(c.pixels[14 * 240 + 13] == 0x0000);
    canvas_free(&c);
}

int main(void)
{
    test_full_region_is_striped();
    test_odd_height_is_covered();
    test_rejects_out_of_range();
    test_pixels_come_from_the_right_place();
    printf("canvas: all tests passed\n");
    return 0;
}
