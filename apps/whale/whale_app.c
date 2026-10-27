/* A blinking pixel portrait: display + time + storage.
 *
 * Reads two pre-rendered frames (open / closed) from storage, draws the open
 * one, and swaps to the closed one for a fraction of a second every few
 * seconds. No image data lives in this repository: without stored frames the
 * app shows a placeholder. */
#include "app.h"

#include <stdio.h>

#include "canvas.h"

#define GRID_W 56
#define GRID_H 48
#define FRAME_CELLS (GRID_W * GRID_H)
#define FRAME_BYTES ((FRAME_CELLS + 1) / 2)
#define FRAMES 3
#define KEY_FRAME0 "whale.f0"
#define KEY_FRAME1 "whale.f1"
#define KEY_FRAME2 "whale.f2"
/* A blink is a short sequence rather than a single swap: open -> half -> shut
 * -> half -> open, one step every BLINK_STEP_MS. */
static const int BLINK_SEQ[] = {1, 2, 1};
#define BLINK_STEPS ((int)(sizeof(BLINK_SEQ) / sizeof(BLINK_SEQ[0])))
#define BLINK_STEP_MS 45

static const uint16_t PALETTE[15] = {
    0xFFFF, 0xF75C, 0xCE5A, 0x9D37, 0x7C54, 0x7458, 0x5BB6, 0x5B74,
    0x4B12, 0x42B1, 0x3A70, 0x424E, 0x29CC, 0x10C6, 0xF5D9,
};

static dada_app_services_t services;
static canvas_t canvas;
static uint8_t frames[FRAMES][FRAME_CELLS];
static bool have_frames;
static int cell;
static int origin_x;
static int origin_y;
static int shown;              /* frame currently on screen */
static int blink_step;         /* index into BLINK_SEQ, -1 when not blinking */
static uint64_t blink_next_ms;
static uint64_t next_blink;
static uint32_t rng = 12345u;

static uint32_t next_random(void)
{
    rng = rng * 1664525u + 1013904223u;
    return rng >> 16;
}

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

static dada_err_t draw_frame(int slot)
{
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            uint8_t c = frames[slot][y * GRID_W + x];
            canvas_fill(&canvas, origin_x + x * cell, origin_y + y * cell,
                        cell, cell, PALETTE[c]);
        }
    }
    return canvas_flush(&canvas, origin_x, origin_y, GRID_W * cell, GRID_H * cell);
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

    int cw = canvas.width / GRID_W;
    int ch = canvas.height / GRID_H;
    cell = cw < ch ? cw : ch;
    origin_x = (canvas.width - cell * GRID_W) / 2;
    origin_y = (canvas.height - cell * GRID_H) / 2;

    have_frames = load_frame(0, KEY_FRAME0) && load_frame(1, KEY_FRAME1) &&
                  load_frame(2, KEY_FRAME2);
    if (!have_frames) {
        printf("whale: no stored frames; draw one in paint and re-seed\n");
        draw_placeholder();
    } else {
        err = draw_frame(0);
        if (err != DADA_OK) {
            dada_app_stop();
            return err;
        }
        uint64_t now = services.time->monotonic_ms(services.time->ctx);
        next_blink = now + 2500;
        printf("whale: %dx%d at %dpx, blinking\n", GRID_W, GRID_H, cell);
    }
    shown = 0;
    blink_step = -1;
    blink_next_ms = 0;
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    uint64_t now = services.time->monotonic_ms(services.time->ctx);

    if (!have_frames) {
        return DADA_OK;
    }
    if (!have_frames) {
        return DADA_OK;
    }
    if (blink_step >= 0) {
        if (now < blink_next_ms) {
            return DADA_OK;
        }
        blink_step++;
        if (blink_step >= BLINK_STEPS) {
            /* Sequence done: reopen and schedule the next blink. */
            blink_step = -1;
            dada_err_t err = draw_frame(0);
            if (err != DADA_OK) {
                return err;
            }
            shown = 0;
            next_blink = now + 2200 + (next_random() % 2600);
            return DADA_OK;
        }
        int target = BLINK_SEQ[blink_step];
        dada_err_t err = draw_frame(target);
        if (err != DADA_OK) {
            return err;
        }
        shown = target;
        blink_next_ms = now + BLINK_STEP_MS;
        return DADA_OK;
    }
    if (now >= next_blink) {
        blink_step = 0;
        int target = BLINK_SEQ[0];
        dada_err_t err = draw_frame(target);
        if (err != DADA_OK) {
            return err;
        }
        shown = target;
        blink_next_ms = now + BLINK_STEP_MS;
    }
    return DADA_OK;
}

void dada_app_stop(void)
{
    canvas_free(&canvas);
    services = (dada_app_services_t){0};
}
