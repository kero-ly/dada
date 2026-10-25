/**
 * @file dada_types.h
 * @brief Shared value types for all dada capability contracts.
 *
 * Rules (see docs/adr/0001-layering.md):
 *  - Contracts must not include ESP-IDF or FreeRTOS headers.
 *  - Errors are plain `int`: 0 = success, non-zero = failure.
 *  - Coordinates are logical screen coordinates, origin at the top-left.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Error type for every contract call. 0 means success. */
typedef int dada_err_t;

#define DADA_OK 0
#define DADA_ERR_FAIL 1
#define DADA_ERR_INVALID_ARG 2
#define DADA_ERR_NOT_SUPPORTED 3
#define DADA_ERR_TIMEOUT 4

/** A point in logical screen coordinates. */
typedef struct {
    int16_t x;
    int16_t y;
} dada_point_t;

/** A rectangle in logical screen coordinates, inclusive on both corners. */
typedef struct {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} dada_area_t;

/** Wall-clock date and time. */
typedef struct {
    int16_t year;   /**< e.g. 2026 */
    int8_t month;   /**< 1..12 */
    int8_t day;     /**< 1..31 */
    int8_t hour;    /**< 0..23 */
    int8_t minute;  /**< 0..59 */
    int8_t second;  /**< 0..59 */
} dada_datetime_t;

#ifdef __cplusplus
}
#endif
