/**
 * @file time.h
 * @brief Time contract: monotonic clock for durations, wall clock when available.
 *
 * Kept separate on purpose. A countdown (pomodoro) only needs the monotonic
 * clock, so a board without an RTC or network can still run offline apps.
 */
#pragma once

#include "dada_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dada_time dada_time_t;

struct dada_time {
    void *ctx;

    /** Milliseconds since boot; never goes backwards. */
    uint64_t (*monotonic_ms)(void *ctx);

    /**
     * Current wall-clock time.
     *
     * @return true when a valid time is known (synced or set by the user),
     *         false when the board has no reliable wall clock
     */
    bool (*wall_clock)(void *ctx, dada_datetime_t *out);
};

#ifdef __cplusplus
}
#endif
