/* NVS implementation of the storage contract. */

#include "storage_nvs.h"

#include <stdlib.h>

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "storage_nvs";
#define NAMESPACE "dada"

typedef struct {
    nvs_handle_t handle;
} nvs_ctx_t;

static bool impl_get_u32(void *opaque, const char *key, uint32_t *out)
{
    nvs_ctx_t *ctx = (nvs_ctx_t *)opaque;
    if (!ctx || !key || !out) {
        return false;
    }
    uint32_t value = 0;
    if (nvs_get_u32(ctx->handle, key, &value) != ESP_OK) {
        return false;
    }
    *out = value;
    return true;
}

static dada_err_t impl_set_u32(void *opaque, const char *key, uint32_t value)
{
    nvs_ctx_t *ctx = (nvs_ctx_t *)opaque;
    if (!ctx || !key) {
        return DADA_ERR_INVALID_ARG;
    }
    if (nvs_set_u32(ctx->handle, key, value) != ESP_OK) {
        ESP_LOGW(TAG, "write failed for '%s'", key);
        return DADA_ERR_FAIL;
    }
    if (nvs_commit(ctx->handle) != ESP_OK) {
        ESP_LOGW(TAG, "commit failed for '%s'", key);
        return DADA_ERR_FAIL;
    }
    return DADA_OK;
}

static dada_err_t impl_erase(void *opaque, const char *key)
{
    nvs_ctx_t *ctx = (nvs_ctx_t *)opaque;
    if (!ctx || !key) {
        return DADA_ERR_INVALID_ARG;
    }
    esp_err_t err = nvs_erase_key(ctx->handle, key);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return DADA_OK; /* erasing an absent key is not an error */
    }
    if (err != ESP_OK) {
        return DADA_ERR_FAIL;
    }
    return nvs_commit(ctx->handle) == ESP_OK ? DADA_OK : DADA_ERR_FAIL;
}

dada_err_t storage_nvs_init(dada_storage_t **out)
{
    if (!out) {
        return DADA_ERR_INVALID_ARG;
    }
    *out = NULL;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* First boot after a partition or format change: rebuild the partition. */
        ESP_LOGW(TAG, "erasing NVS partition (%s)", esp_err_to_name(err));
        if (nvs_flash_erase() != ESP_OK) {
            return DADA_ERR_FAIL;
        }
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init: %s", esp_err_to_name(err));
        return DADA_ERR_FAIL;
    }

    nvs_ctx_t *ctx = calloc(1, sizeof(*ctx));
    dada_storage_t *storage = calloc(1, sizeof(*storage));
    if (!ctx || !storage) {
        free(ctx);
        free(storage);
        return DADA_ERR_FAIL;
    }
    if (nvs_open(NAMESPACE, NVS_READWRITE, &ctx->handle) != ESP_OK) {
        free(ctx);
        free(storage);
        return DADA_ERR_FAIL;
    }

    storage->ctx = ctx;
    storage->get_u32 = impl_get_u32;
    storage->set_u32 = impl_set_u32;
    storage->erase = impl_erase;

    *out = storage;
    ESP_LOGI(TAG, "NVS namespace '%s' ready", NAMESPACE);
    return DADA_OK;
}

void storage_nvs_deinit(dada_storage_t *storage)
{
    if (!storage) {
        return;
    }
    nvs_ctx_t *ctx = (nvs_ctx_t *)storage->ctx;
    if (ctx) {
        nvs_close(ctx->handle);
        free(ctx);
    }
    free(storage);
}
