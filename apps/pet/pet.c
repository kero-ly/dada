#include "pet.h"

#include <string.h>

#define PRESS_SINK 6          /* logical pixels the pet sinks while held */
#define FIDGET_SPAN 1         /* how far a fidget moves it */
#define FIDGET_MIN_MS 8000
#define FIDGET_MAX_MS 20000

static uint32_t next_random(pet_t *p)
{
    p->rng = p->rng * 1664525u + 1013904223u;
    return p->rng >> 16;
}

int pet_breath(uint64_t now_ms)
{
    /* Triangle wave over 3 s, expressed as a vertical scale in 1/512 units:
     * 0 12 24 24 12 0, i.e. up to ~4.7%. Scaling reads as a chest rising;
     * translating the whole image just looks like the picture is sliding. */
    uint32_t phase = (uint32_t)((now_ms / 500) % 6);
    int step = phase < 3 ? (int)phase : (int)(5 - phase);
    return step * 12;
}

void pet_init(pet_t *p, uint64_t now_ms, uint32_t seed)
{
    if (!p) {
        return;
    }
    memset(p, 0, sizeof(*p));
    p->rng = seed ? seed : 1u;
    p->next_fidget_ms = now_ms + FIDGET_MIN_MS;
    p->state = PET_IDLE;
}

void pet_touch(pet_t *p, bool pressed, uint64_t now_ms)
{
    if (!p) {
        return;
    }
    if (pressed && p->state != PET_PRESSED) {
        p->state = PET_PRESSED;
        p->pressed_since_ms = now_ms;
        p->target_y = PRESS_SINK;
    } else if (!pressed && p->state == PET_PRESSED) {
        p->state = PET_IDLE;
        p->target_y = 0;
        p->next_fidget_ms = now_ms + FIDGET_MIN_MS;
    }
}

bool pet_tick(pet_t *p, uint64_t now_ms)
{
    if (!p) {
        return false;
    }
    bool changed = false;

    /* Fidget: a short sideways nudge, then settle back. */
    if (p->state == PET_IDLE && now_ms >= p->next_fidget_ms) {
        int dir = (next_random(p) % 2) ? FIDGET_SPAN : -FIDGET_SPAN;
        p->target_x = dir;
        p->next_fidget_ms = now_ms + FIDGET_MIN_MS +
                            (next_random(p) % (FIDGET_MAX_MS - FIDGET_MIN_MS));
        changed = true;
    } else if (p->state == PET_IDLE && p->target_x != 0 &&
               now_ms + 700 >= p->next_fidget_ms) {
        p->target_x = 0;
        changed = true;
    }

    /* Ease one cell per tick toward the target. */
    if (p->offset_x != p->target_x) {
        p->offset_x += p->offset_x < p->target_x ? 1 : -1;
        changed = true;
    }
    if (p->offset_y != p->target_y) {
        p->offset_y += p->offset_y < p->target_y ? 1 : -1;
        changed = true;
    }
    return changed;
}
