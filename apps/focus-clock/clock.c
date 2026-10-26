#include "clock.h"

#include <stddef.h>

void clock_init(focus_clock_t *c, uint32_t duration_ms)
{
    if (!c) {
        return;
    }
    c->state = CLOCK_IDLE;
    c->duration_ms = duration_ms;
    c->remaining_ms = duration_ms;
    c->last_ms = 0;
    c->completed = 0;
}

void clock_reset(focus_clock_t *c)
{
    if (!c) {
        return;
    }
    c->state = CLOCK_IDLE;
    c->remaining_ms = c->duration_ms;
    c->last_ms = 0;
}

void clock_tick(focus_clock_t *c, uint64_t now_ms)
{
    if (!c || c->state != CLOCK_RUNNING) {
        return;
    }
    if (now_ms < c->last_ms) {
        /* Monotonic clocks must not go backwards. If one does, ignore the
         * sample instead of crediting or charging time for the gap: the next
         * good reading resumes from where the countdown actually was. */
        return;
    }
    if (now_ms == c->last_ms) {
        return;
    }
    uint64_t elapsed = now_ms - c->last_ms;
    c->last_ms = now_ms;

    if (elapsed >= c->remaining_ms) {
        c->remaining_ms = 0;
        c->state = CLOCK_DONE;
        c->completed++;
    } else {
        c->remaining_ms -= (uint32_t)elapsed;
    }
}

void clock_toggle(focus_clock_t *c, uint64_t now_ms)
{
    if (!c) {
        return;
    }
    switch (c->state) {
    case CLOCK_IDLE:
        c->state = CLOCK_RUNNING;
        c->last_ms = now_ms;
        break;
    case CLOCK_RUNNING:
        clock_tick(c, now_ms);
        /* Settling may have finished the session; only pause if still running. */
        if (c->state == CLOCK_RUNNING) {
            c->state = CLOCK_PAUSED;
        }
        break;
    case CLOCK_PAUSED:
        c->state = CLOCK_RUNNING;
        c->last_ms = now_ms;
        break;
    case CLOCK_DONE:
        clock_reset(c);
        c->state = CLOCK_RUNNING;
        c->last_ms = now_ms;
        break;
    }
}

uint32_t clock_remaining_ms(const focus_clock_t *c)
{
    return c ? c->remaining_ms : 0;
}

bool clock_is_running(const focus_clock_t *c)
{
    return c && c->state == CLOCK_RUNNING;
}
