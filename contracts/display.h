/**
 * @file display.h
 * @brief Sink-level display contract.
 *
 * The contract moves pixels and controls a backlight. It deliberately does NOT
 * expose drawing primitives (lines, text, rounded rectangles): those belong to a
 * UI framework sitting above the contract. See docs/adr/0001-layering.md.
 */
#pragma once

#include "dada_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dada_display dada_display_t;

struct dada_display {
    /** Panel geometry in pixels, in the current orientation. */
    uint16_t width;
    uint16_t height;

    /** Bits per pixel of the buffer passed to ::flush (16 = RGB565). */
    uint8_t color_depth;

    /** Implementation context; opaque to callers. */
    void *ctx;

    /**
     * Push a rectangle of pixels to the panel.
     *
     * @param area   target rectangle, inclusive coordinates
     * @param pixels row-major RGB565, little endian, `width*height` entries
     * @return DADA_OK on success
     */
    dada_err_t (*flush)(void *ctx, const dada_area_t *area, const void *pixels);

    /**
     * Set backlight brightness.
     *
     * @param percent 0 (off) .. 100 (full)
     * @return DADA_OK on success, DADA_ERR_NOT_SUPPORTED when the board has no
     *         controllable backlight
     */
    dada_err_t (*set_backlight)(void *ctx, uint8_t percent);
};

#ifdef __cplusplus
}
#endif
