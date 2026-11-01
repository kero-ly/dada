/* Desktop pet: display + input + time + storage.
 *
 * Shows the shared portrait and keeps it alive: it breathes, sinks while the
 * panel is touched, and occasionally shifts sideways. All motion is a cell
 * offset applied while scanning out stripes, so no extra frames are needed.
 *
 * The frame comes from storage (`pet.frame`) when present, otherwise the shared
 * built-in portrait. */
#include "app.h"

#include <stdio.h>

#include "display.h"
#include "pet.h"
#include "portrait/frame.h"

#define GRID_W 160
#define GRID_H 120
#define CELL_PX 2
#define FRAME_CELLS (GRID_W * GRID_H)
#define FRAME_BYTES ((FRAME_CELLS + 1) / 2)
#define KEY_FRAME "pet.frame"
#define STRIPE_COLS 16
#define TICK_MS 40

static dada_app_services_t services;
static pet_t pet;
static uint8_t cells[FRAME_CELLS];
static uint16_t stripe[STRIPE_COLS * 320];
static bool have_frame;
static bool was_pressed;
static uint64_t last_tick_ms;
static int drawn_x;
static int drawn_y;
static int drawn_breath;

static const uint16_t PALETTE[15] = {
    0xFFFF, 0xF75C, 0xCE5A, 0x9D37, 0x7C54, 0x7458, 0x5BB6, 0x5B74,
    0x4B12, 0x42B1, 0x3A70, 0x424E, 0x29CC, 0x10C6, 0xF5D9,
};

static bool load_frame(void)
{
    if (services.storage && services.storage->get_blob) {
        static uint8_t packed[FRAME_BYTES];
        uint32_t length = 0;
        if (services.storage->get_blob(services.storage->ctx, KEY_FRAME, packed,
                                       sizeof(packed), &length) &&
            length == FRAME_BYTES) {
            for (uint32_t i = 0; i < length; i++) {
                cells[i * 2] = (uint8_t)(packed[i] >> 4);
                if (i * 2 + 1 < FRAME_CELLS) {
                    cells[i * 2 + 1] = (uint8_t)(packed[i] & 0x0F);
                }
            }
            return true;
        }
    }
    if (sizeof(PORTRAIT_FRAME) == FRAME_BYTES) {
        for (uint32_t i = 0; i < FRAME_BYTES; i++) {
            cells[i * 2] = (uint8_t)(PORTRAIT_FRAME[i] >> 4);
            if (i * 2 + 1 < FRAME_CELLS) {
                cells[i * 2 + 1] = (uint8_t)(PORTRAIT_FRAME[i] & 0x0F);
            }
        }
        return true;
    }
    return false;
}

/* Draw the portrait rotated onto the panel, shifted by the pet's offsets. */
static dada_err_t draw(int off_x, int off_y, int breath_q10)
{
    const int lw = GRID_W * CELL_PX;
    const int lh = GRID_H * CELL_PX;
    const int phys_w = services.display->width;
    const int phys_h = services.display->height;

    /* Stripes run along the axis the pet moves on (physical x, which is the
     * user's vertical once the board is held sideways). Scanning the other way
     * reads as a wipe; scanning along the motion reads as a scroll. */
    for (int px0 = 0; px0 < phys_w; px0 += STRIPE_COLS) {
        int cols = phys_w - px0 < STRIPE_COLS ? phys_w - px0 : STRIPE_COLS;
        for (int px = px0; px < px0 + cols; px++) {
            /* Breathing scales about the centre so the image expands both ways
             * like a chest rising; the press offset is applied afterwards so
             * being held still reads as sinking. */
            int src = phys_w - 1 - px - off_x;
            int mid = lh / 2;
            int ly = mid + ((src - mid) * (512 + breath_q10)) / 512 - off_y;
            for (int py = 0; py < phys_h; py++) {
                /* Rotation maps logical x to physical y, so the vertical motion
                 * the user sees (breathing, sinking) is an offset in logical y,
                 * and the sideways fidget is an offset in logical x. */
                int lx = py - off_x;
                uint16_t color = 0x0000;
                if (lx >= 0 && lx < lw && ly >= 0 && ly < lh) {
                    color = PALETTE[cells[(ly / CELL_PX) * GRID_W + (lx / CELL_PX)]];
                }
                stripe[(px - px0) + py * cols] = color;
            }
        }
        dada_area_t area = {(int16_t)px0, 0, (int16_t)(px0 + cols - 1),
                            (int16_t)(phys_h - 1)};
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
    /* Input is optional: without a touch panel the pet still breathes and
     * fidgets, it just cannot be petted. */
    if (provided->input && !provided->input->poll) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    services = *provided;

    have_frame = load_frame();
    if (!have_frame) {
        printf("pet: no frame available\n");
        return DADA_ERR_FAIL;
    }
    uint64_t now = services.time->monotonic_ms(services.time->ctx);
    pet_init(&pet, now, 0x9E77u);
    last_tick_ms = now;
    was_pressed = false;

    drawn_x = 0;
    drawn_y = 0;
    drawn_breath = pet_breath(now);
    dada_err_t err = draw(drawn_x, drawn_y, drawn_breath);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }
    printf("pet: awake (%dx%d cells), touch to pet\n", GRID_W, GRID_H);
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    uint64_t now = services.time->monotonic_ms(services.time->ctx);

    if (services.input) {
        dada_point_t point;
        bool pressed;
        if (services.input->poll(services.input->ctx, &point, &pressed) &&
            pressed != was_pressed) {
            was_pressed = pressed;
            pet_touch(&pet, pressed, now);
        }
    }

    if (now - last_tick_ms < TICK_MS) {
        return DADA_OK;
    }
    last_tick_ms = now;

    bool moved = pet_tick(&pet, now);
    int breath = pet_breath(now);
    int want_x = pet.offset_x;
    int want_y = pet.offset_y;

    /* Breathing changes every 500 ms, so compare against what is on screen. */
    if (!moved && want_x == drawn_x && want_y == drawn_y &&
        breath == drawn_breath) {
        return DADA_OK;
    }
    drawn_x = want_x;
    drawn_y = want_y;
    drawn_breath = breath;
    return draw(drawn_x, drawn_y, drawn_breath);
}

void dada_app_stop(void)
{
    services = (dada_app_services_t){0};
    have_frame = false;
}
