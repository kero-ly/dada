/* Pixel art: display + input + storage, no hardware knowledge.
 *
 * The whole panel is the canvas (48x64 cells of 5 px). The colours live behind
 * a small button in the bottom-right corner: tapping it slides a two-row
 * palette over the bottom of the canvas, and picking a colour puts the canvas
 * back. The bitmap is packed 4 bits per cell (1536 bytes) and saved to storage
 * when the finger lifts, then restored on start. */
#include "app.h"

#include <stdio.h>

#include "canvas.h"
#include "paint.h"

#define COLS 48
#define ROWS 64
#define SLOTS 16            /* 15 colours + clear */
#define COLORS 15
#define KEY_BITS "paint.bits"
#define PALETTE_ROWS 2
#define PALETTE_H 40
#define BUTTON 32

/* White background first, then the blues sampled from the reference art, then
 * the darker shadow, the cheek pink and a bright eye blue. */
static const uint16_t PALETTE[COLORS] = {
    0xFFFF, 0xF75C, 0xCE5A, 0x9D37, 0x7C54, 0x7458, 0x5BB6, 0x5B74,
    0x4B12, 0x42B1, 0x3A70, 0x424E, 0x29CC, 0x10C6, 0xF5D9,
};

static dada_app_services_t services;
static canvas_t canvas;
static paint_t art;
static int cell;
static int selected;
static bool active;
static bool palette_open;
static bool was_pressed;
static bool dirty;
static int last_gx;
static int last_gy;
static uint8_t packed[DADA_STORAGE_BLOB_MAX];

static int grid_x(int gx) { return gx * cell; }
static int grid_y(int gy) { return gy * cell; }

static void draw_cell(int gx, int gy)
{
    uint8_t color = paint_get(&art, gx, gy);
    canvas_fill(&canvas, grid_x(gx), grid_y(gy), cell, cell, PALETTE[color]);
}

static dada_err_t flush_cells(int x0, int y0, int x1, int y1)
{
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    return canvas_flush(&canvas, grid_x(x0), grid_y(y0),
                        (x1 - x0 + 1) * cell, (y1 - y0 + 1) * cell);
}

static int button_x(void) { return canvas.width - BUTTON - 4; }
static int button_y(void) { return canvas.height - BUTTON - 4; }

static void draw_button(void)
{
    int bx = button_x();
    int by = button_y();
    canvas_fill(&canvas, bx, by, BUTTON, BUTTON, 0x2104);
    canvas_frame(&canvas, bx, by, BUTTON, BUTTON, 2, 0xFFFF);
    /* Three swatches hint at "colours". */
    for (int i = 0; i < 3; i++) {
        canvas_fill(&canvas, bx + 6 + i * 7, by + 8, 6, 6, PALETTE[5 + i * 2]);
    }
    canvas_fill(&canvas, bx + 6, by + 18, 20, 6, PALETTE[selected]);
}

static void draw_palette_panel(void)
{
    int slot_w = canvas.width / 8;
    int slot_h = PALETTE_H / PALETTE_ROWS;
    int top = canvas.height - PALETTE_H;
    for (int i = 0; i < SLOTS; i++) {
        int x = (i % 8) * slot_w;
        int y = top + (i / 8) * slot_h;
        if (i < COLORS) {
            canvas_fill(&canvas, x + 1, y + 1, slot_w - 2, slot_h - 2, PALETTE[i]);
        } else {
            canvas_fill(&canvas, x + 1, y + 1, slot_w - 2, slot_h - 2, 0x2104);
            canvas_frame(&canvas, x + 2, y + 2, slot_w - 4, slot_h - 4, 1, 0xFFFF);
            for (int k = 0; k < slot_w - 8 && k < slot_h - 8; k++) {
                canvas_fill(&canvas, x + 4 + k, y + 4 + k, 2, 2, 0xFFFF);
                canvas_fill(&canvas, x + 4 + k, y + slot_h - 6 - k, 2, 2, 0xFFFF);
            }
        }
    }
    int sx = (selected % 8) * slot_w;
    int sy = top + (selected / 8) * slot_h;
    canvas_frame(&canvas, sx, sy, slot_w, slot_h, 2, 0x07E0);
}

/** Repaint the canvas below @p first_row from the bitmap. */
static dada_err_t restore_rows(int first_row)
{
    if (first_row < 0) {
        first_row = 0;
    }
    for (int gy = first_row; gy < ROWS; gy++) {
        for (int gx = 0; gx < COLS; gx++) {
            draw_cell(gx, gy);
        }
    }
    draw_button();
    int y = grid_y(first_row);
    return canvas_flush(&canvas, 0, y, canvas.width, canvas.height - y);
}

static dada_err_t redraw_all(void)
{
    for (int gy = 0; gy < ROWS; gy++) {
        for (int gx = 0; gx < COLS; gx++) {
            draw_cell(gx, gy);
        }
    }
    draw_button();
    return canvas_flush_all(&canvas);
}

static void open_palette(void)
{
    palette_open = true;
    draw_palette_panel();
    canvas_flush(&canvas, 0, canvas.height - PALETTE_H, canvas.width, PALETTE_H);
}

static void close_palette(void)
{
    palette_open = false;
    int first_row = (canvas.height - PALETTE_H) / cell;
    restore_rows(first_row);
}

static dada_err_t save_bitmap(void)
{
    if (!services.storage || !services.storage->set_blob) {
        return DADA_OK; /* not persisting is a valid degradation */
    }
    uint32_t length = paint_pack(&art, packed, sizeof(packed));
    if (length == 0) {
        return DADA_ERR_FAIL;
    }
    dada_err_t err = services.storage->set_blob(services.storage->ctx, KEY_BITS,
                                                packed, length);
    if (err == DADA_OK) {
        dirty = false;
    } else {
        printf("paint: save failed %d\n", err);
    }
    return err;
}

static void load_bitmap(void)
{
    if (!services.storage || !services.storage->get_blob) {
        return;
    }
    uint32_t length = 0;
    if (!services.storage->get_blob(services.storage->ctx, KEY_BITS, packed,
                                    sizeof(packed), &length)) {
        return;
    }
    if (paint_unpack(&art, packed, length)) {
        printf("paint: restored %u bytes\n", (unsigned)length);
    } else {
        printf("paint: stored bitmap has an unexpected size (%u)\n", (unsigned)length);
    }
}

static void pick_slot(int x, int y)
{
    int slot_w = canvas.width / 8;
    int slot_h = PALETTE_H / PALETTE_ROWS;
    int slot = ((y - (canvas.height - PALETTE_H)) / slot_h) * 8 + (x / slot_w);
    if (slot < 0 || slot >= SLOTS) {
        return;
    }
    if (slot >= COLORS) {
        paint_clear(&art);
        dirty = true;
        printf("paint: cleared\n");
    } else {
        selected = slot;
    }
    close_palette();
}

static void paint_at(int x, int y)
{
    int gx = x / cell;
    int gy = y / cell;
    if (gx < 0 || gy < 0 || gx >= COLS || gy >= ROWS) {
        last_gx = -1;
        return;
    }
    uint8_t color = (uint8_t)selected;
    if (last_gx >= 0 && (last_gx != gx || last_gy != gy)) {
        paint_stroke(&art, last_gx, last_gy, gx, gy, color);
        draw_cell(gx, gy);
        draw_cell(last_gx, last_gy);
        flush_cells(last_gx, last_gy, gx, gy);
    } else if (last_gx < 0) {
        paint_set(&art, gx, gy, color);
        draw_cell(gx, gy);
        flush_cells(gx, gy, gx, gy);
    }
    last_gx = gx;
    last_gy = gy;
    dirty = true;
}

dada_err_t dada_app_start(const dada_app_services_t *provided)
{
    dada_app_stop();
    if (!provided || !provided->display || !provided->input) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    if (!provided->display->flush || !provided->input->poll) {
        return DADA_ERR_NOT_SUPPORTED;
    }
    services = *provided;

    if (!paint_init(&art, COLS, ROWS)) {
        dada_app_stop();
        return DADA_ERR_FAIL;
    }
    dada_err_t err = canvas_init(&canvas, services.display);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }

    int cw = canvas.width / COLS;
    int ch = canvas.height / ROWS;
    cell = cw < ch ? cw : ch;

    load_bitmap();
    err = redraw_all();
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }

    selected = 6; /* a mid blue: the most useful starting colour on this art */
    palette_open = false;
    last_gx = last_gy = -1;
    printf("paint: %dx%d cells at %dpx, %u bytes when saved\n", COLS, ROWS, cell,
           (unsigned)paint_packed_size(&art));
    active = true;
    was_pressed = false;
    dirty = false;
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    if (!active) {
        return DADA_ERR_FAIL;
    }
    dada_point_t point;
    bool pressed;
    if (!services.input->poll(services.input->ctx, &point, &pressed)) {
        return DADA_OK;
    }
    if (!pressed) {
        if (was_pressed && dirty) {
            save_bitmap();
        }
        was_pressed = false;
        last_gx = last_gy = -1;
        return DADA_OK;
    }

    bool edge = !was_pressed;
    was_pressed = true;

    if (palette_open) {
        if (edge) {
            pick_slot(point.x, point.y);
        }
        return DADA_OK;
    }

    if (edge && point.x >= button_x() && point.y >= button_y()) {
        open_palette();
        last_gx = last_gy = -1;
        return DADA_OK;
    }
    paint_at(point.x, point.y);
    return DADA_OK;
}

void dada_app_stop(void)
{
    if (active && dirty) {
        save_bitmap();
    }
    canvas_free(&canvas);
    active = false;
    services = (dada_app_services_t){0};
}
