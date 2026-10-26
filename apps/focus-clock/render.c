/* App-local renderer: one full-frame canvas, flushed by region. */
#include "render.h"

#include <stdlib.h>

#define BG 0x0841
#define PANEL 0x18E3
#define FG 0xFFFF
#define RUN_COLOR 0x07FF
#define PAUSE_COLOR 0xFFE0
#define DONE_COLOR 0xF800
#define BAR_BG 0x2104
#define BAR_FG 0x07E0

static dada_display_t *panel;
static uint16_t *canvas;
static uint32_t shown_second = 0xFFFFFFFFu;
static clock_state_t shown_state = (clock_state_t)-1;

/* 5x7 glyphs for only the characters this screen uses. */
static const uint8_t *glyph(char c)
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
    static const uint8_t A[] = {14, 17, 17, 31, 17, 17, 17};
    static const uint8_t D[] = {30, 17, 17, 17, 17, 17, 30};
    static const uint8_t E[] = {31, 16, 16, 30, 16, 16, 31};
    static const uint8_t N[] = {17, 25, 21, 19, 17, 17, 17};
    static const uint8_t O[] = {14, 17, 17, 17, 17, 17, 14};
    static const uint8_t P[] = {30, 17, 17, 30, 16, 16, 16};
    static const uint8_t R[] = {30, 17, 17, 30, 20, 18, 17};
    static const uint8_t S[] = {15, 16, 16, 14, 1, 1, 30};
    static const uint8_t T[] = {31, 4, 4, 4, 4, 4, 4};
    static const uint8_t U[] = {17, 17, 17, 17, 17, 17, 14};
    static const uint8_t W[] = {17, 17, 17, 21, 21, 21, 10};
    static const uint8_t F[] = {31, 16, 16, 30, 16, 16, 16};
    static const uint8_t C[] = {14, 17, 16, 16, 16, 17, 14};
    static const uint8_t blank[] = {0, 0, 0, 0, 0, 0, 0};

    switch (c) {
    case '0': return n0; case '1': return n1; case '2': return n2;
    case '3': return n3; case '4': return n4; case '5': return n5;
    case '6': return n6; case '7': return n7; case '8': return n8;
    case '9': return n9; case ':': return colon;
    case 'A': return A; case 'D': return D; case 'E': return E;
    case 'N': return N; case 'O': return O; case 'P': return P;
    case 'R': return R; case 'S': return S; case 'T': return T;
    case 'U': return U; case 'W': return W; case 'F': return F; case 'C': return C;
    default: return blank;
    }
}

static void put(int x, int y, uint16_t color)
{
    if (x >= 0 && x < panel->width && y >= 0 && y < panel->height) {
        canvas[y * panel->width + x] = color;
    }
}

static void fill(int x, int y, int w, int h, uint16_t color)
{
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            put(x + col, y + row, color);
        }
    }
}

static void text(const char *s, int cx, int y, int scale, uint16_t color)
{
    int length = 0;
    while (s[length]) {
        length++;
    }
    int width = (length * 6 - 1) * scale;
    int x = cx - width / 2;
    for (int i = 0; i < length; i++) {
        const uint8_t *rows = glyph(s[i]);
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (rows[row] & (1 << (4 - col))) {
                    fill(x + (i * 6 + col) * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

static dada_err_t flush_rect(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) {
        return DADA_OK;
    }
    uint16_t *packed = malloc((size_t)w * h * sizeof(uint16_t));
    if (!packed) {
        return DADA_ERR_FAIL;
    }
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            packed[row * w + col] = canvas[(y + row) * panel->width + (x + col)];
        }
    }
    dada_area_t area = {(int16_t)x, (int16_t)y, (int16_t)(x + w - 1), (int16_t)(y + h - 1)};
    dada_err_t err = panel->flush(panel->ctx, &area, packed);
    free(packed);
    return err;
}

static const char *state_word(const focus_clock_t *clock)
{
    switch (clock->state) {
    case CLOCK_RUNNING: return "RUN";
    case CLOCK_PAUSED: return "PAUSE";
    case CLOCK_DONE: return "DONE";
    default: return "TAP";
    }
}

static uint16_t state_color(const focus_clock_t *clock)
{
    switch (clock->state) {
    case CLOCK_RUNNING: return RUN_COLOR;
    case CLOCK_PAUSED: return PAUSE_COLOR;
    case CLOCK_DONE: return DONE_COLOR;
    default: return FG;
    }
}

dada_err_t focus_render_start(dada_display_t *display)
{
    focus_render_stop();
    if (!display || !display->flush || display->color_depth != 16) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    panel = display;
    size_t pixels = (size_t)panel->width * panel->height;
    canvas = malloc(pixels * sizeof(uint16_t));
    if (!canvas) {
        panel = NULL;
        return DADA_ERR_FAIL;
    }

    shown_second = 0xFFFFFFFFu;
    shown_state = (clock_state_t)-1;
    fill(0, 0, panel->width, panel->height, BG);
    /* Title strip and the reset zone hint at the bottom. */
    fill(0, 0, panel->width, 34, PANEL);
    text("DADA FOCUS", panel->width / 2, 9, 2, FG);
    fill(0, panel->height - 40, panel->width, 40, PANEL);
    text("RESET", panel->width / 2, panel->height - 29, 2, FG);

    dada_area_t full = {0, 0, (int16_t)(panel->width - 1), (int16_t)(panel->height - 1)};
    return panel->flush(panel->ctx, &full, canvas);
}

dada_err_t focus_render_update(const focus_clock_t *clock)
{
    if (!panel || !canvas || !clock) {
        return DADA_ERR_FAIL;
    }

    uint32_t seconds = (clock_remaining_ms(clock) + 999) / 1000;
    if (seconds == shown_second && clock->state == shown_state) {
        return DADA_OK;
    }
    shown_second = seconds;
    shown_state = clock->state;

    uint32_t minutes = seconds / 60;
    uint32_t secs = seconds % 60;
    static char buf[6];
    buf[0] = (char)('0' + (minutes / 10) % 10);
    buf[1] = (char)('0' + minutes % 10);
    buf[2] = ':';
    buf[3] = (char)('0' + (secs / 10) % 10);
    buf[4] = (char)('0' + secs % 10);
    buf[5] = 0;

    int scale = panel->width >= 200 ? 7 : 5;
    int digits_w = (5 * 6 - 1) * scale;
    int digits_x = (panel->width - digits_w) / 2;
    int digits_y = panel->height / 4;

    /* Redraw the time block on the canvas, then push only that block. */
    fill(digits_x - 4, digits_y - 4, digits_w + 8, 7 * scale + 8, BG);
    text(buf, panel->width / 2, digits_y, scale, state_color(clock));
    dada_err_t err = flush_rect(digits_x - 4, digits_y - 4, digits_w + 8, 7 * scale + 8);
    if (err != DADA_OK) {
        return err;
    }

    /* Progress bar: elapsed portion in green. */
    int bar_x = panel->width / 8;
    int bar_w = panel->width - 2 * bar_x;
    int bar_y = digits_y + 7 * scale + 20;
    int filled = clock->duration_ms ? (int)((uint64_t)bar_w *
                 (clock->duration_ms - clock_remaining_ms(clock)) / clock->duration_ms) : 0;
    fill(bar_x, bar_y, bar_w, 10, BAR_BG);
    fill(bar_x, bar_y, filled, 10, BAR_FG);
    err = flush_rect(bar_x, bar_y, bar_w, 10);
    if (err != DADA_OK) {
        return err;
    }

    /* State word and completed count. */
    int word_y = bar_y + 26;
    fill(panel->width / 4, word_y - 4, panel->width / 2, 7 * 3 + 8, BG);
    text(state_word(clock), panel->width / 2, word_y, 3, state_color(clock));
    return flush_rect(panel->width / 4, word_y - 4, panel->width / 2, 7 * 3 + 8);
}

void focus_render_stop(void)
{
    if (canvas) {
        free(canvas);
        canvas = NULL;
    }
    panel = NULL;
}
