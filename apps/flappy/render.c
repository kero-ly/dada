#include "render.h"

#include <stdlib.h>

#define C_SKY_TOP 0x4E0A    /* teal */
#define C_SKY_BOT 0xB71D    /* pale cyan */
#define C_PIPE 0x73BE
#define C_PIPE_DK 0x4608
#define C_PIPE_LT 0xAA6A
#define C_SAND 0xE4DA
#define C_SAND_DK 0xC9B4
#define C_GRASS 0x7CC6
#define C_GRASS_DK 0x5896
#define C_DARK 0x262E
#define C_WHITE 0xFFFF
#define C_BEAK 0xF5C0
#define C_WING 0xF7DE

static canvas_t canvas;
static bool ready;

/* 5x7 glyphs for just the words this screen shows. */
static const uint8_t *glyph(char c)
{
    static const uint8_t G[] = {14, 17, 17, 31, 17, 17, 17};
    static const uint8_t A[] = {14, 17, 17, 31, 17, 17, 17};
    static const uint8_t M[] = {17, 27, 21, 21, 17, 17, 17};
    static const uint8_t E[] = {31, 16, 16, 30, 16, 16, 31};
    static const uint8_t O[] = {14, 17, 17, 17, 17, 17, 14};
    static const uint8_t V[] = {17, 17, 17, 17, 17, 10, 4};
    static const uint8_t R[] = {30, 17, 17, 30, 20, 18, 17};
    static const uint8_t T[] = {31, 4, 4, 4, 4, 4, 4};
    static const uint8_t P[] = {30, 17, 17, 30, 16, 16, 16};
    static const uint8_t S[] = {15, 16, 16, 14, 1, 1, 30};
    static const uint8_t C[] = {14, 17, 16, 16, 16, 17, 14};
    static const uint8_t blank[] = {0, 0, 0, 0, 0, 0, 0};
    switch (c) {
    case 'G': return G; case 'A': return A; case 'M': return M;
    case 'E': return E; case 'O': return O; case 'V': return V;
    case 'R': return R; case 'T': return T; case 'P': return P;
    case 'S': return S; case 'C': return C;
    default: return blank;
    }
}

static void text(const char *s, int cx, int y, int scale, uint16_t color)
{
    int len = 0;
    while (s[len]) {
        len++;
    }
    int x = cx - ((len * 6 - 1) * scale) / 2;
    for (int i = 0; i < len; i++) {
        const uint8_t *rows = glyph(s[i]);
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (rows[row] & (1 << (4 - col))) {
                    canvas_fill(&canvas, x + (i * 6 + col) * scale,
                                y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

dada_err_t flappy_render_init(dada_display_t *display)
{
    dada_err_t err = canvas_init(&canvas, display);
    if (err != DADA_OK) {
        return err;
    }
    ready = true;
    return DADA_OK;
}

void flappy_render_free(void)
{
    if (ready) {
        canvas_free(&canvas);
        ready = false;
    }
}

static void draw_sky(const flappy_t *f)
{
    int play = flappy_play_height(f);
    /* Cheap vertical gradient: one colour per 16-row band. */
    for (int y = 0; y < play; y += 16) {
        int t = y * 255 / (play > 0 ? play : 1);
        int r = ((C_SKY_TOP >> 11) & 0x1F) * (255 - t) / 255 + ((C_SKY_BOT >> 11) & 0x1F) * t / 255;
        int g = ((C_SKY_TOP >> 5) & 0x3F) * (255 - t) / 255 + ((C_SKY_BOT >> 5) & 0x3F) * t / 255;
        int b = (C_SKY_TOP & 0x1F) * (255 - t) / 255 + (C_SKY_BOT & 0x1F) * t / 255;
        uint16_t c = (uint16_t)((r << 11) | (g << 5) | b);
        canvas_fill(&canvas, 0, y, canvas.width, 16, c);
    }
}

static void draw_pipe(const flappy_t *f, const flappy_pipe_t *p)
{
    int gap_top = (int)p->gap_y - 48;
    int gap_bot = (int)p->gap_y + 48;
    int x = (int)p->x;
    int play = flappy_play_height(f);

    if (x + 54 < 0 || x > canvas.width) {
        return;
    }
    /* Upper column and cap. */
    canvas_fill(&canvas, x, 0, 54, gap_top, C_PIPE);
    canvas_fill(&canvas, x, 0, 4, gap_top, C_PIPE_LT);
    canvas_fill(&canvas, x + 50, 0, 4, gap_top, C_PIPE_DK);
    canvas_fill(&canvas, x - 3, gap_top - 22, 60, 22, C_PIPE);
    canvas_fill(&canvas, x - 3, gap_top - 22, 60, 3, C_PIPE_LT);
    /* Lower column and cap. */
    canvas_fill(&canvas, x, gap_bot, 54, play - gap_bot, C_PIPE);
    canvas_fill(&canvas, x, gap_bot, 4, play - gap_bot, C_PIPE_LT);
    canvas_fill(&canvas, x + 50, gap_bot, 4, play - gap_bot, C_PIPE_DK);
    canvas_fill(&canvas, x - 3, gap_bot, 60, 22, C_PIPE);
    canvas_fill(&canvas, x - 3, gap_bot + 19, 60, 3, C_PIPE_DK);
}

static void draw_ground(const flappy_t *f)
{
    int top = flappy_play_height(f);
    canvas_fill(&canvas, 0, top, canvas.width, FLAPPY_GROUND_H, C_SAND);
    canvas_fill(&canvas, 0, top, canvas.width, 12, C_GRASS);
    canvas_fill(&canvas, 0, top + 10, canvas.width, 2, C_GRASS_DK);
    for (int x = 8; x < canvas.width; x += 24) {
        canvas_fill(&canvas, x, top + 20, 10, 4, C_SAND_DK);
    }
}

static void draw_bird(const flappy_t *f)
{
    int x = 52;
    int y = (int)f->bird_y;
    /* Body. */
    canvas_fill(&canvas, x + 4, y + 6, 22, 14, C_BEAK);
    canvas_fill(&canvas, x + 6, y + 3, 18, 20, C_BEAK);
    canvas_fill(&canvas, x + 8, y + 2, 14, 3, C_WING);
    /* Wing, offset by vertical speed so flapping reads. */
    int wing = f->bird_vy < 0 ? -3 : 2;
    canvas_fill(&canvas, x + 2, y + 10 + wing, 12, 7, C_WING);
    /* Eye and beak. */
    canvas_fill(&canvas, x + 18, y + 7, 5, 5, C_WHITE);
    canvas_fill(&canvas, x + 19, y + 8, 3, 3, C_DARK);
    canvas_fill(&canvas, x + 24, y + 12, 6, 4, 0xF800);
}

dada_err_t flappy_render_frame(const flappy_t *f)
{
    if (!ready || !f) {
        return DADA_ERR_FAIL;
    }
    draw_sky(f);
    for (int i = 0; i < FLAPPY_PIPES; i++) {
        draw_pipe(f, &f->pipes[i]);
    }
    draw_ground(f);
    draw_bird(f);

    char buf[8];
    int n = 0;
    int score = f->score;
    if (score == 0) {
        buf[n++] = '0';
    } else {
        char rev[8];
        int r = 0;
        while (score > 0) {
            rev[r++] = (char)('0' + score % 10);
            score /= 10;
        }
        while (r > 0) {
            buf[n++] = rev[--r];
        }
    }
    buf[n] = 0;
    canvas_digits(&canvas, canvas.width / 2 - 10, 28, buf, 5, C_WHITE);

    if (f->state == FLAPPY_READY) {
        text("TAP", canvas.width / 2, 200, 3, C_WHITE);
    } else if (f->state == FLAPPY_DEAD) {
        text("GAME", canvas.width / 2, 120, 3, C_WHITE);
        text("OVER", canvas.width / 2, 148, 3, C_WHITE);
    }
    return canvas_flush_all(&canvas);
}
