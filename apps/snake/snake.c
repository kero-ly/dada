#include "snake.h"

#include <stdlib.h>

#include "dada_types.h"

static uint32_t next_random(snake_t *s)
{
    /* Small LCG: deterministic per seed, good enough for food placement. */
    s->rng = s->rng * 1664525u + 1013904223u;
    return s->rng >> 16;
}

static bool cell_free(const snake_t *s, int x, int y)
{
    return !snake_body_at(s, x, y);
}

static void place_food(snake_t *s)
{
    int free_cells = s->width * s->height - s->length;
    if (free_cells <= 0) {
        s->state = SNAKE_OVER;
        return;
    }
    int target = (int)(next_random(s) % (uint32_t)free_cells);
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            if (!cell_free(s, x, y)) {
                continue;
            }
            if (target-- == 0) {
                s->food_x = x;
                s->food_y = y;
                return;
            }
        }
    }
    /* Unreachable unless the snake covers the board; treat as a win. */
    s->state = SNAKE_OVER;
}

int snake_init(snake_t *s, int w, int h, uint32_t seed)
{
    if (!s || w < 4 || h < 4) {
        return DADA_ERR_INVALID_ARG;
    }
    int capacity = w * h;
    s->body_x = malloc((size_t)capacity * sizeof(int16_t));
    s->body_y = malloc((size_t)capacity * sizeof(int16_t));
    if (!s->body_x || !s->body_y) {
        free(s->body_x);
        free(s->body_y);
        s->body_x = s->body_y = NULL;
        return DADA_ERR_FAIL;
    }
    s->width = w;
    s->height = h;
    s->capacity = capacity;
    s->length = 0;
    s->dir = SNAKE_RIGHT;
    s->state = SNAKE_READY;
    s->score = 0;
    s->step_ms = 160;
    s->last_step_ms = 0;
    s->rng = seed ? seed : 1u;
    s->turn_count = 0;
    s->food_x = s->food_y = 0;
    return DADA_OK;
}

void snake_free(snake_t *s)
{
    if (!s) {
        return;
    }
    free(s->body_x);
    free(s->body_y);
    s->body_x = s->body_y = NULL;
    s->length = 0;
}

void snake_start(snake_t *s, uint64_t now_ms)
{
    if (!s || !s->body_x) {
        return;
    }
    s->length = 3;
    int cx = s->width / 2;
    int cy = s->height / 2;
    /* Head first: body[0] is the head. */
    s->body_x[0] = (int16_t)cx;
    s->body_y[0] = (int16_t)cy;
    s->body_x[1] = (int16_t)(cx - 1);
    s->body_y[1] = (int16_t)cy;
    s->body_x[2] = (int16_t)(cx - 2);
    s->body_y[2] = (int16_t)cy;
    s->dir = SNAKE_RIGHT;
    s->score = 0;
    s->state = SNAKE_RUNNING;
    s->last_step_ms = now_ms;
    s->step_ms = 160;
    s->turn_count = 0;
    place_food(s);
}

bool snake_turn(snake_t *s, snake_dir_t dir)
{
    if (!s || s->state != SNAKE_RUNNING || dir < SNAKE_UP || dir > SNAKE_LEFT) {
        return false;
    }
    if (s->turn_count >= (int)(sizeof(s->turn_queue) / sizeof(s->turn_queue[0]))) {
        return false; /* queue full: the player is spamming swipes */
    }
    /* Validate against the last direction that will actually be in effect, so
     * "up then left" is accepted but "left while moving right" is not. */
    int base = (int)(s->turn_count ? s->turn_queue[s->turn_count - 1] : s->dir);
    int wanted = (int)dir;
    if ((base + 2) % 4 == wanted || base == wanted) {
        return false;
    }
    s->turn_queue[s->turn_count++] = dir;
    return true;
}

bool snake_body_at(const snake_t *s, int x, int y)
{
    if (!s || !s->body_x) {
        return false;
    }
    for (int i = 0; i < s->length; i++) {
        if (s->body_x[i] == x && s->body_y[i] == y) {
            return true;
        }
    }
    return false;
}

bool snake_tick(snake_t *s, uint64_t now_ms)
{
    if (!s || s->state != SNAKE_RUNNING) {
        return false;
    }
    if (now_ms < s->last_step_ms + s->step_ms) {
        return false;
    }
    /* Drop any backlog: the game must not fast-forward after a long stall. */
    s->last_step_ms = now_ms;

    /* Apply at most one queued turn per step. */
    if (s->turn_count > 0) {
        s->dir = s->turn_queue[0];
        for (int i = 1; i < s->turn_count; i++) {
            s->turn_queue[i - 1] = s->turn_queue[i];
        }
        s->turn_count--;
    }

    int head_x = s->body_x[0];
    int head_y = s->body_y[0];
    switch (s->dir) {
    case SNAKE_UP: head_y -= 1; break;
    case SNAKE_RIGHT: head_x += 1; break;
    case SNAKE_DOWN: head_y += 1; break;
    case SNAKE_LEFT: head_x -= 1; break;
    }

    bool eating = (head_x == s->food_x && head_y == s->food_y);

    if (head_x < 0 || head_y < 0 || head_x >= s->width || head_y >= s->height) {
        s->state = SNAKE_OVER;
        return false;
    }
    /* The tail cell is about to move away, so it is legal unless we grow. */
    for (int i = 0; i < s->length - (eating ? 0 : 1); i++) {
        if (s->body_x[i] == head_x && s->body_y[i] == head_y) {
            s->state = SNAKE_OVER;
            return false;
        }
    }

    int new_length = s->length + (eating ? 1 : 0);
    if (new_length > s->capacity) {
        s->state = SNAKE_OVER;
        return false;
    }
    for (int i = new_length - 1; i > 0; i--) {
        s->body_x[i] = s->body_x[i - 1];
        s->body_y[i] = s->body_y[i - 1];
    }
    s->body_x[0] = (int16_t)head_x;
    s->body_y[0] = (int16_t)head_y;
    s->length = new_length;

    if (eating) {
        s->score += 10;
        /* Speed up gradually, but never below 70 ms per step. */
        if (s->step_ms > 70) {
            s->step_ms -= 4;
        }
        place_food(s);
    }
    return true;
}
