/**
 * @file storage_nvs.h
 * @brief NVS-backed implementation of the storage contract.
 */
#pragma once

#include "storage.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open the application namespace in NVS.
 *
 * Initialises the NVS partition if needed. Either output may be NULL if the
 * caller does not need the capability.
 */
dada_err_t storage_nvs_init(dada_storage_t **out);

void storage_nvs_deinit(dada_storage_t *storage);

#ifdef __cplusplus
}
#endif
