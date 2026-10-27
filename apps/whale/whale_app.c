/* A blinking pixel portrait: display + time + storage.
 *
 * Reads two pre-rendered frames (open / closed) from storage, draws the open
 * one, and swaps to the closed one for a fraction of a second every few
 * seconds. No image data lives in this repository: without stored frames the
 * app shows a placeholder. */
#include "app.h"

#include <stdio.h>

#include "canvas.h"

/* The artwork is landscape: 64x48 cells of 5 px fills a 320x240 area, which
 * is then rotated onto the 240x320 panel. */
#define GRID_W 80
#define GRID_H 60
#define CELL_PX 4
#define FRAME_CELLS (GRID_W * GRID_H)
#define FRAME_BYTES ((FRAME_CELLS + 1) / 2)
#define FRAMES 1
#define KEY_FRAME0 "whale.f0"

static const uint16_t PALETTE[15] = {
    0xFFFF, 0xF75C, 0xCE5A, 0x9D37, 0x7C54, 0x7458, 0x5BB6, 0x5B74,
    0x4B12, 0x42B1, 0x3A70, 0x424E, 0x29CC, 0x10C6, 0xF5D9,
};

static dada_app_services_t services;
static canvas_t canvas;
static uint8_t frames[FRAMES][FRAME_CELLS];
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

/* Paint the landscape frame rotated onto the panel, so the picture is upright
 * when the board is held sideways. The inverse map avoids a second 150 KB
 * buffer: logical (x, y) = (py, width-1-px). */
static dada_err_t draw_frame(int slot)
{
    const int lw = GRID_W * CELL_PX;
    const int lh = GRID_H * CELL_PX;
    for (int py = 0; py < canvas.height; py++) {
        for (int px = 0; px < canvas.width; px++) {
            int lx = py;
            int ly = canvas.width - 1 - px;
            if (lx >= lw || ly >= lh) {
                canvas.pixels[py * canvas.width + px] = 0x0000;
                continue;
            }
            int gx = lx / CELL_PX;
            int gy = ly / CELL_PX;
            canvas.pixels[py * canvas.width + px] = PALETTE[frames[slot][gy * GRID_W + gx]];
        }
    }
    return canvas_flush_all(&canvas);
}

static dada_err_t draw_placeholder(void)
{
    canvas_clear(&canvas, 0x0000);
    canvas_frame(&canvas, 8, 8, canvas.width - 16, canvas.height - 16, 2, 0x7C54);
    return canvas_flush_all(&canvas);
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

    dada_err_t err = canvas_init(&canvas, services.display);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }
    canvas_clear(&canvas, 0x0000);

    have_frames = load_frame(0, KEY_FRAME0);
    if (!have_frames) {
        printf("whale: no stored frames; draw one in paint and re-seed\n");
        draw_placeholder();
    } else {
        err = draw_frame(0);
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
    canvas_free(&canvas);
    services = (dada_app_services_t){0};
}
