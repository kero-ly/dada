/**
 * @file snake.h
 * @brief Game rules only: grid, movement, food, collision. No UI, no hardware.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SNAKE_READY = 0,
    SNAKE_RUNNING,
    SNAKE_OVER,
} snake_state_t;

typedef enum {
    SNAKE_UP = 0,
    SNAKE_RIGHT,
    SNAKE_DOWN,
    SNAKE_LEFT,
} snake_dir_t;

typedef struct {
    int width;
    int height;
    int16_t *body_x;
    int16_t *body_y;
    int length;
    int capacity;
    snake_dir_t dir;
    int food_x;
    int food_y;
    snake_state_t state;
    uint32_t score;
    uint32_t step_ms;
    uint64_t last_step_ms;
    uint32_t rng;
    /* Queued turns, applied one per step. Two slots cover the fastest realistic
     * swipe sequence without letting a player reverse into the neck. */
    snake_dir_t turn_queue[2];
    int turn_count;
} snake_t;

/** Allocate the body; w and h are grid cells. Returns DADA_OK on success. */
int snake_init(snake_t *s, int w, int h, uint32_t seed);
void snake_free(snake_t *s);

/** Begin a new round: length 3 in the middle, score cleared, food placed. */
void snake_start(snake_t *s, uint64_t now_ms);

/**
 * Request a direction change.
 *
 * The request is queued and applied on the next step, so a fast swipe sequence
 * ("up then left") is honoured over two steps instead of being dropped.
 *
 * @return true when the turn was queued, false when it was impossible
 *         (reversal into the neck, duplicate direction, or a full queue)
 */
bool snake_turn(snake_t *s, snake_dir_t dir);

/**
 * Advance the simulation if enough time has passed.
 *
 * @return true when a step was taken, so the caller knows to redraw.
 */
bool snake_tick(snake_t *s, uint64_t now_ms);

/** True when the snake body occupies this cell. */
bool snake_body_at(const snake_t *s, int x, int y);
