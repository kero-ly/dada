/* Host tests for the pixel-art bitmap rules. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "paint.h"
#include "storage.h"

static void test_init_and_bounds(void)
{
    paint_t p;
    assert(paint_init(&p, 24, 28));
    assert(p.cols == 24 && p.rows == 28);
    assert(paint_get(&p, 0, 0) == PAINT_COLOR_BG);
    assert(paint_get(&p, 23, 27) == PAINT_COLOR_BG);

    /* Out of range reads are background, writes are ignored. */
    assert(paint_get(&p, -1, 0) == PAINT_COLOR_BG);
    assert(paint_get(&p, 24, 0) == PAINT_COLOR_BG);
    assert(!paint_set(&p, -1, 0, PAINT_COLOR_1));
    assert(!paint_set(&p, 0, 28, PAINT_COLOR_1));
    assert(paint_get(&p, 0, 0) == PAINT_COLOR_BG);

    assert(!paint_init(&p, 0, 10));
    assert(!paint_init(&p, PAINT_MAX_COLS + 1, 10));
    /* A colour index outside the palette is rejected, not truncated. */
    assert(!paint_set(&p, 0, 0, PAINT_PALETTE_SIZE));
}

static void test_pack_round_trip(void)
{
    paint_t p, restored;
    assert(paint_init(&p, 48, 56));
    assert(paint_init(&restored, 48, 56));
    paint_set(&p, 0, 0, 1);
    paint_set(&p, 47, 55, 14);       /* last cell, odd index */
    paint_set(&p, 5, 6, 15 - 1);
    assert(paint_cell_count(&p) == 2688);
    assert(paint_packed_size(&p) == 1344);

    uint8_t buf[DADA_STORAGE_BLOB_MAX];
    uint32_t wrote = paint_pack(&p, buf, sizeof(buf));
    assert(wrote == 1344);
    assert(paint_unpack(&restored, buf, wrote));
    for (int i = 0; i < 48 * 56; i++) {
        assert(restored.cells[i] == p.cells[i]);
    }
    /* A wrong length must be refused rather than half-applied. */
    assert(!paint_unpack(&restored, buf, wrote - 1));
    /* Too small a buffer is refused. */
    assert(paint_pack(&p, buf, 100) == 0);
}

static void test_set_and_clear(void)
{
    paint_t p;
    assert(paint_init(&p, 8, 8));
    assert(paint_set(&p, 3, 4, PAINT_COLOR_3));
    assert(paint_get(&p, 3, 4) == PAINT_COLOR_3);

    paint_clear(&p);
    assert(paint_get(&p, 3, 4) == PAINT_COLOR_BG);
    assert(paint_cell_count(&p) == 64);
}

static void test_stroke_fills_gaps(void)
{
    paint_t p;
    assert(paint_init(&p, 10, 10));
    /* A diagonal jump must leave no holes between the endpoints. */
    assert(paint_stroke(&p, 0, 0, 9, 9, PAINT_COLOR_2));
    for (int i = 0; i <= 9; i++) {
        assert(paint_get(&p, i, i) == PAINT_COLOR_2);
    }
    assert(paint_get(&p, 0, 1) == PAINT_COLOR_BG);
}

static void test_horizontal_stroke_is_continuous(void)
{
    paint_t p;
    assert(paint_init(&p, 12, 4));
    assert(paint_stroke(&p, 11, 2, 1, 2, PAINT_COLOR_5));   /* reversed direction */
    for (int x = 1; x <= 11; x++) {
        assert(paint_get(&p, x, 2) == PAINT_COLOR_5);
    }
    assert(paint_get(&p, 0, 2) == PAINT_COLOR_BG);
}

static void test_stroke_clips_at_edges(void)
{
    paint_t p;
    assert(paint_init(&p, 8, 8));
    assert(paint_stroke(&p, -3, 4, 11, 4, PAINT_COLOR_4));
    for (int x = 0; x < 8; x++) {
        assert(paint_get(&p, x, 4) == PAINT_COLOR_4);
    }
}

static void test_bitmap_is_persistable(void)
{
    paint_t p;
    assert(paint_init(&p, 24, 28));
    paint_set(&p, 5, 6, PAINT_COLOR_6);
    /* The bitmap must fit the guaranteed blob size. */
    assert(paint_packed_size(&p) <= DADA_STORAGE_BLOB_MAX);

    /* Round trip through a byte copy, as storage would do. */
    uint8_t copy[PAINT_MAX_COLS * PAINT_MAX_ROWS];
    memcpy(copy, p.cells, paint_cell_count(&p));
    assert(copy[6 * 24 + 5] == PAINT_COLOR_6);
}

int main(void)
{
    test_init_and_bounds();
    test_set_and_clear();
    test_pack_round_trip();
    test_stroke_fills_gaps();
    test_horizontal_stroke_is_continuous();
    test_stroke_clips_at_edges();
    test_bitmap_is_persistable();
    printf("paint: all tests passed\n");
    return 0;
}
