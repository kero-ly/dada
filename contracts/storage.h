/**
 * @file storage.h
 * @brief Small persistent key/value store for application state.
 *
 * Deliberately tiny: unsigned 32-bit values keyed by a short name. That covers
 * high scores, counters, settings flags and progress markers without dragging
 * allocation semantics or a filesystem API into the contract.
 *
 * Rules:
 *  - No ESP-IDF, FreeRTOS or filesystem types may appear here.
 *  - Keys are NUL-terminated ASCII, at most 15 characters (the NVS limit);
 *    applications should prefix their own namespace, e.g. "snake.best".
 *  - Values survive power loss. Implementations may cache, but must not lose a
 *    value that set_u32 reported as written.
 */
#pragma once

#include "dada_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dada_storage dada_storage_t;

struct dada_storage {
    /** Implementation context; opaque to callers. */
    void *ctx;

    /**
     * Read a stored value.
     *
     * @param key  NUL-terminated key, at most 15 characters
     * @param out  written only when the key exists
     * @return true when the key exists, false when it is absent or on error
     */
    bool (*get_u32)(void *ctx, const char *key, uint32_t *out);

    /**
     * Store or overwrite a value.
     *
     * @return DADA_OK once the value is durable
     */
    dada_err_t (*set_u32)(void *ctx, const char *key, uint32_t value);

    /**
     * Remove a key. Erasing an absent key is not an error.
     */
    dada_err_t (*erase)(void *ctx, const char *key);
};

#ifdef __cplusplus
}
#endif
