/**
 * @file input.h
 * @brief Sink-level pointer input contract (touch or pointing device).
 *
 * Answers exactly two questions: is something pressed, and where.
 * Gestures, buttons and widgets belong to the UI layer above.
 */
#pragma once

#include "dada_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dada_input dada_input_t;

struct dada_input {
    void *ctx;

    /**
     * Sample the current pointer state. Cheap enough to call every frame.
     *
     * @param point   filled with the last known position (valid even when not pressed)
     * @param pressed true while the panel is being touched
     * @return true when the sample is valid, false when the device did not answer
     */
    bool (*poll)(void *ctx, dada_point_t *point, bool *pressed);
};

#ifdef __cplusplus
}
#endif
