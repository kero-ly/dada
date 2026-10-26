/* Offline pomodoro: countdown only, no network, no RTC.
 * Interaction: tap anywhere above the bottom strip to start/pause,
 * tap the bottom strip to reset. */
#include "app.h"

#include <stdio.h>

#include "clock.h"
#include "render.h"

#ifndef DADA_FOCUS_MINUTES
#define DADA_FOCUS_MINUTES 25
#endif
#define SESSION_MS ((uint32_t)DADA_FOCUS_MINUTES * 60u * 1000u)

static dada_app_services_t services;
static focus_clock_t clock;
static bool active, was_pressed;
static clock_state_t reported_state = (clock_state_t)-1;

dada_err_t dada_app_start(const dada_app_services_t *provided)
{
    dada_app_stop();
    if (!provided || !provided->display || !provided->input || !provided->time) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    if (!provided->display->flush || !provided->input->poll ||
        !provided->time->monotonic_ms) {
        return DADA_ERR_NOT_SUPPORTED;
    }

    services = *provided;
    clock_init(&clock, SESSION_MS);

    dada_err_t err = focus_render_start(services.display);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }
    err = focus_render_update(&clock);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }
    printf("focus: ready %u:%02u, tap to start\n",
           (unsigned)(SESSION_MS / 60000u), (unsigned)((SESSION_MS / 1000u) % 60u));
    reported_state = clock.state;
    was_pressed = false;
    active = true;
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    if (!active) {
        return DADA_ERR_FAIL;
    }
    uint64_t now = services.time->monotonic_ms(services.time->ctx);

    clock_tick(&clock, now);

    dada_point_t point;
    bool pressed;
    if (services.input->poll(services.input->ctx, &point, &pressed)) {
        bool clicked = pressed && !was_pressed;
        was_pressed = pressed;
        if (clicked) {
            if (point.y >= services.display->height * 3 / 4) {
                clock_reset(&clock);
                printf("focus: reset\n");
            } else {
                clock_toggle(&clock, now);
                printf("focus: toggle -> state %d, %u s left\n", (int)clock.state,
                       (unsigned)((clock_remaining_ms(&clock) + 999) / 1000));
            }
        }
    }

    if (clock.state != reported_state) {
        reported_state = clock.state;
        if (clock.state == CLOCK_DONE) {
            printf("focus: session done, %u completed\n", (unsigned)clock.completed);
        }
    }

    return focus_render_update(&clock);
}

void dada_app_stop(void)
{
    focus_render_stop();
    active = false;
    services = (dada_app_services_t){0};
}
