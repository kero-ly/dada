/* Flappy Bird: display + input + time (+ storage for the best score).
 *
 * A fixed 60 Hz simulation step with an accumulator, so the physics do not
 * change with frame rate. Rendering is a full repaint; the panel is small
 * enough that this still runs at a playable rate. */
#include "app.h"

#include <stdio.h>

#include "flappy.h"
#include "render.h"

#define STEP_MS 20          /* 50 Hz simulation */
#define KEY_BEST "flappy.best"

static dada_app_services_t services;
static flappy_t game;
static bool active;
static bool was_pressed;
static uint64_t last_ms;
static uint32_t accumulator_ms;

static void persist_best(void)
{
    if (services.storage && services.storage->set_u32) {
        services.storage->set_u32(services.storage->ctx, KEY_BEST, (uint32_t)game.best);
    }
}

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

    uint32_t best = 0;
    if (services.storage && services.storage->get_u32) {
        services.storage->get_u32(services.storage->ctx, KEY_BEST, &best);
    }
    if (!flappy_init(&game, services.display->width, services.display->height, 0xB157u)) {
        dada_app_stop();
        return DADA_ERR_FAIL;
    }
    game.best = (int)best;

    dada_err_t err = flappy_render_init(services.display);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }
    err = flappy_render_frame(&game);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }

    last_ms = services.time->monotonic_ms(services.time->ctx);
    accumulator_ms = 0;
    was_pressed = false;
    active = true;
    printf("flappy: ready (%dx%d), best %d\n", game.width, game.height, game.best);
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    if (!active) {
        return DADA_ERR_FAIL;
    }
    uint64_t now = services.time->monotonic_ms(services.time->ctx);
    uint64_t elapsed = now - last_ms;
    last_ms = now;
    if (elapsed > 100) {
        elapsed = 100; /* do not fast-forward after a stall */
    }

    flappy_state_t before = game.state;
    int score_before = game.score;

    dada_point_t point;
    bool pressed;
    if (services.input->poll(services.input->ctx, &point, &pressed)) {
        bool tapped = pressed && !was_pressed;
        was_pressed = pressed;
        if (tapped) {
            flappy_flap(&game, now);
        }
    }

    accumulator_ms += (uint32_t)elapsed;
    while (accumulator_ms >= STEP_MS) {
        accumulator_ms -= STEP_MS;
        flappy_update(&game, (float)STEP_MS, now);
    }

    if (game.state == FLAPPY_DEAD && before != FLAPPY_DEAD) {
        printf("flappy: game over, score %d best %d\n", game.score, game.best);
        persist_best();
    }
    if (game.score != score_before) {
        printf("flappy: score %d\n", game.score);
    }
    return flappy_render_frame(&game);
}

void dada_app_stop(void)
{
    if (active) {
        persist_best();
    }
    flappy_render_free();
    active = false;
    services = (dada_app_services_t){0};
}
