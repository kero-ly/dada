/* Snake: display + input + time + storage, no hardware knowledge.
 *
 * Controls: swipe to steer, tap to start or restart. The high score is kept in
 * storage under "snake.best" so it survives a power cycle. */
#include "app.h"

#include <stdio.h>

#include "render.h"
#include "snake.h"

#define GRID_W 12
#define GRID_H 14
#define SWIPE_MIN 18 /* pixels of travel before it counts as a swipe */

static dada_app_services_t services;
static snake_view_t view;
static snake_t game;
static bool active;
static bool was_pressed;
static dada_point_t press_point;
static uint32_t best_score;
static uint64_t started_ms;

static dada_err_t start_round(uint64_t now)
{
    snake_start(&game, now);
    dada_err_t err = snake_view_redraw(&view, &game);
    if (err != DADA_OK) {
        return err;
    }
    return DADA_OK;
}

static dada_err_t handle_tap(uint64_t now)
{
    if (game.state == SNAKE_READY || game.state == SNAKE_OVER) {
        return start_round(now);
    }
    return DADA_OK;
}

static void handle_swipe(dada_point_t release)
{
    int dx = (int)release.x - (int)press_point.x;
    int dy = (int)release.y - (int)press_point.y;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    if (adx < SWIPE_MIN && ady < SWIPE_MIN) {
        return; /* a tap, handled elsewhere */
    }
    if (adx >= ady) {
        snake_turn(&game, dx > 0 ? SNAKE_RIGHT : SNAKE_LEFT);
    } else {
        snake_turn(&game, dy > 0 ? SNAKE_DOWN : SNAKE_UP);
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

    if (snake_init(&game, GRID_W, GRID_H, 0x5EEDu) != DADA_OK) {
        dada_app_stop();
        return DADA_ERR_FAIL;
    }
    dada_err_t err = snake_view_init(&view, services.display);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }

    best_score = 0;
    if (services.storage &&
        services.storage->get_u32(services.storage->ctx, "snake.best", &best_score)) {
        printf("snake: best %u\n", (unsigned)best_score);
    }

    /* Show the board in READY state with an empty body. */
    game.length = 0;
    game.state = SNAKE_READY;
    game.food_x = GRID_W / 2;
    game.food_y = GRID_H / 2;
    err = snake_view_redraw(&view, &game);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }
    err = snake_view_overlay(&view, &game);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }

    printf("snake: ready (%dx%d cells), tap to start\n", GRID_W, GRID_H);
    started_ms = services.time->monotonic_ms(services.time->ctx);
    active = true;
    was_pressed = false;
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    if (!active) {
        return DADA_ERR_FAIL;
    }
    uint64_t now = services.time->monotonic_ms(services.time->ctx);

    /* Ignore input for a moment: the touch controller can report a spurious
     * press while it settles after power-on, which would start the game. */
    if (now - started_ms < 400) {
        return DADA_OK;
    }

    /* Input first so a tap can start the round before it advances. */
    dada_point_t point;
    bool pressed;
    if (services.input->poll(services.input->ctx, &point, &pressed)) {
        if (pressed && !was_pressed) {
            press_point = point;
        } else if (!pressed && was_pressed) {
            int dx = (int)point.x - (int)press_point.x;
            int dy = (int)point.y - (int)press_point.y;
            int adx = dx < 0 ? -dx : dx;
            int ady = dy < 0 ? -dy : dy;
            if (adx < SWIPE_MIN && ady < SWIPE_MIN) {
                dada_err_t err = handle_tap(now);
                if (err != DADA_OK) {
                    return err;
                }
            } else {
                handle_swipe(point);
            }
        }
        was_pressed = pressed;
    }

    int tail_x = 0, tail_y = 0, old_head_x = 0, old_head_y = 0, length_before = game.length;
    if (game.length > 0) {
        tail_x = game.body_x[game.length - 1];
        tail_y = game.body_y[game.length - 1];
        old_head_x = game.body_x[0];
        old_head_y = game.body_y[0];
    }
    uint32_t score_before = game.score;
    snake_state_t state_before = game.state;

    if (!snake_tick(&game, now)) {
        if (game.state != state_before && game.state == SNAKE_OVER) {
            /* Hit a wall or itself on this step. */
        } else {
            return DADA_OK;
        }
    }

    if (game.state == SNAKE_OVER) {
        printf("snake: game over, score %u\n", (unsigned)game.score);
        if (game.score > best_score) {
            best_score = game.score;
            if (services.storage) {
                services.storage->set_u32(services.storage->ctx, "snake.best", best_score);
                printf("snake: new best %u\n", (unsigned)best_score);
            }
        }
        return snake_view_overlay(&view, &game);
    }

    /* Incremental repaint: old head becomes body, tail cell clears. */
    dada_err_t err;
    if (length_before > 0 && game.length == length_before) {
        err = snake_view_cell(&view, tail_x, tail_y, SNAKE_C_EMPTY);
        if (err != DADA_OK) return err;
        err = snake_view_cell(&view, old_head_x, old_head_y, SNAKE_C_BODY);
        if (err != DADA_OK) return err;
    }
    err = snake_view_cell(&view, game.body_x[0], game.body_y[0], SNAKE_C_HEAD);
    if (err != DADA_OK) return err;

    if (game.score != score_before) {
        err = snake_view_food(&view, &game);
        if (err != DADA_OK) return err;
        err = snake_view_score(&view, game.score);
        if (err != DADA_OK) return err;
    }
    return DADA_OK;
}

void dada_app_stop(void)
{
    snake_view_free(&view);
    snake_free(&game);
    active = false;
    services = (dada_app_services_t){0};
}
