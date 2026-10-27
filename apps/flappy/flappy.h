/**
 * @file flappy.h
 * @brief Flappy Bird rules: gravity, pipes, scoring, collision. No UI.
 *
 * Ported from the author's Arduino build; the physics constants are the ones
 * tuned there so the game feels the same.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FLAPPY_PIPES 3
#define FLAPPY_GROUND_H 46

typedef enum {
    FLAPPY_READY = 0,
    FLAPPY_PLAY,
    FLAPPY_DEAD,
} flappy_state_t;

typedef struct {
    float x;
    float gap_y;
    bool scored;
} flappy_pipe_t;

typedef struct {
    int width;
    int height;
    float bird_y;
    float bird_vy;
    float speed;
    int score;
    int best;
    flappy_state_t state;
    uint64_t state_ms;
    flappy_pipe_t pipes[FLAPPY_PIPES];
    uint32_t rng;
} flappy_t;

/** Ground-inclusive playfield height (screen height minus the ground strip). */
int flappy_play_height(const flappy_t *f);

bool flappy_init(flappy_t *f, int width, int height, uint32_t seed);

/** Back to the title state with fresh pipes. */
void flappy_reset(flappy_t *f, uint64_t now_ms);

/** Tap: start, flap, or retry once the death pause has elapsed. */
void flappy_flap(flappy_t *f, uint64_t now_ms);

/** Advance the simulation by dt_ms; now_ms stamps the death pause. */
void flappy_update(flappy_t *f, float dt_ms, uint64_t now_ms);
