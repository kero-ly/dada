/**
 * @file pet.h
 * @brief Idle behaviour for the desktop pet: breathing, touch reaction, fidgets.
 *
 * Pure state, no drawing: the renderer asks for an offset in cells and shifts
 * the portrait by it. Keeping it here makes the motion testable without a panel.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PET_IDLE = 0,
    PET_PRESSED,
} pet_state_t;

typedef struct {
    pet_state_t state;
    int offset_x;        /* cells, horizontal */
    int offset_y;        /* cells, vertical */
    int target_x;
    int target_y;
    uint64_t next_fidget_ms;
    uint64_t pressed_since_ms;
    uint32_t rng;
} pet_t;

void pet_init(pet_t *p, uint64_t now_ms, uint32_t seed);

/** Press or release; the pet sinks while held and eases back afterwards. */
void pet_touch(pet_t *p, bool pressed, uint64_t now_ms);

/**
 * Advance the idle animation.
 *
 * @return true when the offset changed, so the caller knows to redraw.
 */
bool pet_tick(pet_t *p, uint64_t now_ms);

/** Breathing displacement in cells: a slow 0..2 triangle wave. */
int pet_breath(uint64_t now_ms);
