#include "flappy.h"

#include <string.h>

/* Physics and layout tuned in the original build. */
#define GRAVITY 1750.0f
#define FLAP_VY (-435.0f)
#define TERMINAL_VY 540.0f
#define PIPE_W 54
#define PIPE_GAP 96
#define PIPE_SPACING 150
#define SPEED_BASE 112.0f
#define BIRD_X 52
#define BIRD_W 30
#define BIRD_H 24
#define READY_DELAY_MS 700

int flappy_play_height(const flappy_t *f)
{
    return f ? f->height - FLAPPY_GROUND_H : 0;
}

static uint32_t next_random(flappy_t *f)
{
    f->rng = f->rng * 1664525u + 1013904223u;
    return f->rng >> 16;
}

static void place_pipes(flappy_t *f)
{
    int play = flappy_play_height(f);
    for (int i = 0; i < FLAPPY_PIPES; i++) {
        f->pipes[i].x = (float)(f->width + 70 + i * PIPE_SPACING);
        /* Keep the gap away from the very top and bottom. */
        int span = play - 148;
        f->pipes[i].gap_y = (float)(74 + (int)(next_random(f) % (uint32_t)(span > 1 ? span : 2)));
        f->pipes[i].scored = false;
    }
}

bool flappy_init(flappy_t *f, int width, int height, uint32_t seed)
{
    if (!f || width < 120 || height < 160) {
        return false;
    }
    memset(f, 0, sizeof(*f));
    f->width = width;
    f->height = height;
    f->rng = seed ? seed : 1u;
    flappy_reset(f, 0);
    return true;
}

void flappy_reset(flappy_t *f, uint64_t now_ms)
{
    if (!f) {
        return;
    }
    f->state = FLAPPY_READY;
    f->score = 0;
    f->speed = SPEED_BASE;
    f->bird_vy = 0;
    f->state_ms = now_ms;
    f->bird_y = flappy_play_height(f) * 0.46f;
    place_pipes(f);
}

void flappy_flap(flappy_t *f, uint64_t now_ms)
{
    if (!f) {
        return;
    }
    switch (f->state) {
    case FLAPPY_READY:
        f->state = FLAPPY_PLAY;
        f->bird_vy = FLAP_VY;
        f->state_ms = now_ms;
        break;
    case FLAPPY_PLAY:
        f->bird_vy = FLAP_VY;
        break;
    case FLAPPY_DEAD:
        if (now_ms > f->state_ms + READY_DELAY_MS) {
            flappy_reset(f, now_ms);
        }
        break;
    }
}

static bool hits_pipe(const flappy_t *f, const flappy_pipe_t *p)
{
    int bx1 = BIRD_X + 5;
    int bx2 = BIRD_X + BIRD_W - 5;
    int by1 = (int)f->bird_y + 4;
    int by2 = (int)f->bird_y + BIRD_H - 4;
    int px1 = (int)p->x;
    int px2 = (int)p->x + PIPE_W;

    if (bx2 <= px1 || bx1 >= px2) {
        return false;
    }
    int gap_top = (int)p->gap_y - PIPE_GAP / 2;
    int gap_bottom = (int)p->gap_y + PIPE_GAP / 2;
    return by1 < gap_top || by2 > gap_bottom;
}

static void die(flappy_t *f, uint64_t now_ms)
{
    if (f->state != FLAPPY_PLAY) {
        return;
    }
    f->state = FLAPPY_DEAD;
    f->state_ms = now_ms;
    if (f->score > f->best) {
        f->best = f->score;
    }
}

void flappy_update(flappy_t *f, float dt_ms, uint64_t now_ms)
{
    if (!f || f->state != FLAPPY_PLAY) {
        return;
    }
    float dt = dt_ms / 1000.0f;
    int play = flappy_play_height(f);

    f->bird_vy += GRAVITY * dt;
    if (f->bird_vy > TERMINAL_VY) {
        f->bird_vy = TERMINAL_VY;
    }
    f->bird_y += f->bird_vy * dt;

    /* Gentle ramp so the game gets harder as the score climbs. */
    f->speed = SPEED_BASE + (float)f->score * 1.6f;

    for (int i = 0; i < FLAPPY_PIPES; i++) {
        flappy_pipe_t *p = &f->pipes[i];
        p->x -= f->speed * dt;

        if (!p->scored && p->x + PIPE_W < BIRD_X) {
            p->scored = true;
            f->score++;
        }
        if (p->x + PIPE_W < -10) {
            /* Recycle to the right of the furthest pipe. */
            float far = f->pipes[0].x;
            for (int j = 1; j < FLAPPY_PIPES; j++) {
                if (f->pipes[j].x > far) {
                    far = f->pipes[j].x;
                }
            }
            p->x = far + PIPE_SPACING;
            int span = play - 148;
            p->gap_y = (float)(74 + (int)(next_random(f) % (uint32_t)(span > 1 ? span : 2)));
            p->scored = false;
        }
        if (hits_pipe(f, p)) {
            die(f, now_ms);
            return;
        }
    }

    if (f->bird_y <= 0) {
        f->bird_y = 0;
        f->bird_vy = 0;
    }
    int floor = play - BIRD_H;
    if (f->bird_y >= floor) {
        f->bird_y = (float)floor;
        die(f, now_ms);
    }
}
