#include "best_score.h"

dada_err_t snake_update_best(dada_storage_t *storage, uint32_t score, uint32_t *best)
{
    if (!best) {
        return DADA_ERR_INVALID_ARG;
    }
    if (score <= *best) {
        return DADA_OK;
    }
    if (!storage || !storage->set_u32) {
        *best = score;
        return DADA_ERR_NOT_SUPPORTED;
    }
    dada_err_t err = storage->set_u32(storage->ctx, "snake.best", score);
    if (err == DADA_OK) {
        *best = score;
    }
    return err;
}
