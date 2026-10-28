/* A pixel portrait: display + storage.
 *
 * Draws one 160x120 frame of 15 colours, rotated onto a portrait panel so it is
 * upright when the board is held sideways. The frame comes from storage when
 * present, otherwise from the built-in copy in frames/, so a fresh clone shows
 * the artwork without any seeding step. */
#include "app.h"

#include <stdio.h>

#include "display.h"
#include "frames/whale-f0.h"


/* The artwork is landscape: 64x48 cells of 5 px fills a 320x240 area, which
 * is then rotated onto the 240x320 panel. */
#define GRID_W 160
#define GRID_H 120
#define CELL_PX 2
#define FRAME_CELLS (GRID_W * GRID_H)
#define FRAME_BYTES ((FRAME_CELLS + 1) / 2)
#define FRAMES 1
#define KEY_FRAME0 "whale.f0"

static const uint16_t PALETTE[15] = {
    0xFFFF, 0xF75C, 0xCE5A, 0x9D37, 0x7C54, 0x7458, 0x5BB6, 0x5B74,
    0x4B12, 0x42B1, 0x3A70, 0x424E, 0x29CC, 0x10C6, 0xF5D9,
};

#define STRIPE_ROWS 8

static dada_app_services_t services;
static uint8_t frames[FRAMES][FRAME_CELLS];
static uint16_t stripe[240 * STRIPE_ROWS];
static bool have_frames;
static bool load_frame(int slot, const char *key)
{
    if (!services.storage || !services.storage->get_blob) {
        return false;
    }
    static uint8_t packed[FRAME_BYTES];
    uint32_t length = 0;
    if (!services.storage->get_blob(services.storage->ctx, key, packed,
                                    sizeof(packed), &length)) {
        return false;
    }
    if (length != FRAME_BYTES) {
        printf("whale: %s has %u bytes, expected %u\n", key, (unsigned)length,
               (unsigned)FRAME_BYTES);
        return false;
    }
    for (uint32_t i = 0; i < length; i++) {
        frames[slot][i * 2] = (uint8_t)(packed[i] >> 4);
        if (i * 2 + 1 < FRAME_CELLS) {
            frames[slot][i * 2 + 1] = (uint8_t)(packed[i] & 0x0F);
        }
    }
    return true;
}

/* Paint the frame rotated onto the panel in horizontal stripes.
 *
 * A full 240x320 framebuffer needs 150 KB of *contiguous* heap, which the
 * allocator cannot supply once the frame array is resident (the total is fine,
 * the largest block is not). Eight rows need 3.8 KB and are flushed as we go.
 *
 * Mapping: physical (px, py) shows logical (lx = py, ly = width-1-px). */
static dada_err_t draw_frame(int slot)
{
    const int lw = GRID_W * CELL_PX;
    const int lh = GRID_H * CELL_PX;
    const int phys_w = services.display->width;
    const int phys_h = services.display->height;

    for (int py0 = 0; py0 < phys_h; py0 += STRIPE_ROWS) {
        int rows = phys_h - py0 < STRIPE_ROWS ? phys_h - py0 : STRIPE_ROWS;
        for (int r = 0; r < rows; r++) {
            int py = py0 + r;
            for (int px = 0; px < phys_w; px++) {
                int lx = py;
                int ly = phys_w - 1 - px;
                uint16_t color = 0x0000;
                if (lx < lw && ly < lh && lx >= 0 && ly >= 0) {
                    color = PALETTE[frames[slot][(ly / CELL_PX) * GRID_W + (lx / CELL_PX)]];
                }
                stripe[r * phys_w + px] = color;
            }
        }
        dada_area_t area = {0, (int16_t)py0, (int16_t)(phys_w - 1),
                            (int16_t)(py0 + rows - 1)};
        dada_err_t err = services.display->flush(services.display->ctx, &area, stripe);
        if (err != DADA_OK) {
            return err;
        }
    }
    return DADA_OK;
}

static dada_err_t draw_placeholder(void)
{
    /* Flat dark screen: frame data is stored separately from the firmware. */
    const int phys_w = services.display->width;
    for (int i = 0; i < phys_w * STRIPE_ROWS; i++) {
        stripe[i] = 0x0000;
    }
    for (int py0 = 0; py0 < services.display->height; py0 += STRIPE_ROWS) {
        int rows = services.display->height - py0 < STRIPE_ROWS
                       ? services.display->height - py0 : STRIPE_ROWS;
        dada_area_t area = {0, (int16_t)py0, (int16_t)(phys_w - 1),
                            (int16_t)(py0 + rows - 1)};
        dada_err_t err = services.display->flush(services.display->ctx, &area, stripe);
        if (err != DADA_OK) {
            return err;
        }
    }
    return DADA_OK;
}

dada_err_t dada_app_start(const dada_app_services_t *provided)
{
    dada_app_stop();
    if (!provided || !provided->display || !provided->time) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    if (!provided->display->flush || !provided->time->monotonic_ms) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    services = *provided;


    have_frames = load_frame(0, KEY_FRAME0);
    if (!have_frames && sizeof(WHALE_FRAME) == FRAME_BYTES) {
        /* Built-in frame: a clone of this repository works without seeding. */
        for (uint32_t i = 0; i < FRAME_BYTES; i++) {
            frames[0][i * 2] = (uint8_t)(WHALE_FRAME[i] >> 4);
            if (i * 2 + 1 < FRAME_CELLS) {
                frames[0][i * 2 + 1] = (uint8_t)(WHALE_FRAME[i] & 0x0F);
            }
        }
        have_frames = true;
        printf("whale: using built-in frame\n");
    }
    if (!have_frames) {
        printf("whale: no stored frames; draw one in paint and re-seed\n");
        draw_placeholder();
    } else {
        dada_err_t err = draw_frame(0);
        if (err != DADA_OK) {
            dada_app_stop();
            return err;
        }
        printf("whale: %dx%d cells at %dpx\n", GRID_W, GRID_H, CELL_PX);
    }
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    /* Static picture: the frame is drawn once in start(). */
    return DADA_OK;
}

void dada_app_stop(void)
{
    services = (dada_app_services_t){0};
}
