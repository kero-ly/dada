/* Pomodoro timing rules, kept free of hardware and UI so tests can drive it. */
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    CLOCK_IDLE = 0,
    CLOCK_RUNNING,
    CLOCK_PAUSED,
    CLOCK_DONE,
} clock_state_t;

typedef struct {
    clock_state_t state;
    uint32_t duration_ms;
    uint32_t remaining_ms;
    uint64_t last_ms;
    uint32_t completed;
} focus_clock_t;

/** Full initialisation: idle, full duration, zero sessions completed. */
void clock_init(focus_clock_t *c, uint32_t duration_ms);

/** Back to idle at full duration; the completed counter is kept. */
void clock_reset(focus_clock_t *c);

/** Start, pause, resume, or restart a finished session. */
void clock_toggle(focus_clock_t *c, uint64_t now_ms);

/** Advance the countdown; does nothing unless running. */
void clock_tick(focus_clock_t *c, uint64_t now_ms);

uint32_t clock_remaining_ms(const focus_clock_t *c);

bool clock_is_running(const focus_clock_t *c);
