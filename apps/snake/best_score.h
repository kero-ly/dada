#pragma once

#include "storage.h"

/** Update the in-memory best only after durable storage succeeds. */
dada_err_t snake_update_best(dada_storage_t *storage, uint32_t score, uint32_t *best);
