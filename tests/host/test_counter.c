#include <assert.h>
#include <stdio.h>
#include "app.h"
#include "counter.h"

static dada_point_t point;
static bool pressed, valid;
static unsigned flushes, white_digit_row;
static int fail_after;
static dada_display_t display;

static dada_err_t flush(void *ctx, const dada_area_t *a, const void *pixels)
{
    (void)ctx;
    assert(pixels && a->x1 >= 0 && a->y1 >= 0);
    assert(a->x2 >= a->x1 && a->y2 >= a->y1);
    assert(a->x2 < display.width && a->y2 < display.height);
    /* At one row through the upper vertical strokes, zero must have white pixels. */
    int digit_row = display.height / 3;
    if (a->y1 <= digit_row && a->y2 >= digit_row) {
        const uint16_t *data = pixels;
        int width = a->x2 - a->x1 + 1;
        int offset = (digit_row - a->y1) * width;
        for (int x = 0; x < width; x++) if (data[offset + x] == 0xffff) white_digit_row++;
    }
    flushes++;
    if (fail_after > 0 && flushes >= (unsigned)fail_after) return DADA_ERR_FAIL;
    return DADA_OK;
}

static bool poll(void *ctx, dada_point_t *p, bool *down)
{
    (void)ctx;
    *p = point;
    *down = pressed;
    return valid;
}

static dada_input_t input = {.poll = poll};
static dada_app_services_t services = {.display = &display, .input = &input};

static void sample(int x, int y, bool down, bool ok)
{
    point = (dada_point_t){x, y}; pressed = down; valid = ok;
    assert(dada_app_step() == DADA_OK);
}

static void start(int w, int h)
{
    dada_app_stop();
    display = (dada_display_t){.width = w, .height = h, .color_depth = 16, .flush = flush};
    flushes = 0; white_digit_row = 0; fail_after = 0; pressed = false; valid = true;
    assert(dada_app_start(&services) == DADA_OK);
    assert(counter_count() == 0 && flushes > 0);
}

int main(void)
{
    start(240, 320);
    unsigned initial = flushes;
    sample(30, 40, false, true);
    assert(flushes == initial); /* idle does not redraw */
    sample(30, 40, true, true);
    assert(counter_count() == 1);
    for (int i = 0; i < 20; i++) sample(30, 40, true, true);
    assert(counter_count() == 1); /* held touch is one click */
    sample(30, 300, true, true);
    assert(counter_count() == 1); /* dragging into reset is not a click */
    sample(30, 300, false, true);
    sample(30, 300, true, true);
    assert(counter_count() == 0);
    sample(30, 40, false, true);
    sample(-1, 40, true, true);
    sample(30, 40, true, true);
    assert(counter_count() == 0); /* outside press cannot drag in */
    sample(30, 40, false, true);
    sample(240, 40, true, true);
    assert(counter_count() == 0);
    sample(30, 40, false, true);
    sample(30, 40, true, true);
    sample(30, 40, false, false); /* invalid sample must not rearm */
    sample(30, 40, true, true);
    assert(counter_count() == 1);
    sample(30, 40, false, true);
    sample(30, 240, true, true); /* reset boundary is inclusive */
    assert(counter_count() == 0);

    start(160, 160);
    sample(159, 119, true, true);
    assert(counter_count() == 1);
    sample(159, 119, false, true);
    sample(159, 120, true, true);
    assert(counter_count() == 0);
    for (int i = 0; i < 10005; i++) {
        sample(20, 20, false, true);
        sample(20, 20, true, true);
    }
    assert(counter_count() == 9999);

    start(800, 160);
    assert(white_digit_row > 0 && white_digit_row < display.width / 4); /* distinct strokes */
    start(4000, 160);
    assert(white_digit_row > 0); /* accepted geometry must render within bounds */
    start(160, 160);

    dada_app_stop(); dada_app_stop();
    assert(dada_app_step() != DADA_OK);
    assert(dada_app_start(NULL) == DADA_ERR_INVALID_ARG);
    dada_app_services_t missing = {.display = &display};
    assert(dada_app_start(&missing) == DADA_ERR_NOT_SUPPORTED);
    display.width = 100;
    assert(dada_app_start(&services) == DADA_ERR_NOT_SUPPORTED);
    display.width = 160; display.color_depth = 24;
    assert(dada_app_start(&services) == DADA_ERR_NOT_SUPPORTED);
    display.color_depth = 16;
    flushes = 0; fail_after = 2;
    assert(dada_app_start(&services) == DADA_ERR_FAIL);
    fail_after = 0;
    assert(dada_app_start(&services) == DADA_OK); /* recover after failed start */
    flushes = 0; fail_after = 1;
    pressed = true; point = (dada_point_t){20, 20}; valid = true;
    assert(dada_app_step() == DADA_ERR_FAIL); /* rendering errors propagate */
    dada_app_stop();
    puts("counter: click, hold, drag, reset, bounds, invalid samples, saturation, errors PASS");
    return 0;
}
