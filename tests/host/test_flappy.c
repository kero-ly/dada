/* Host tests for the Flappy Bird rules. */
#include <assert.h>
#include <stdio.h>

#include "flappy.h"

static flappy_t make(void)
{
    flappy_t f;
    assert(flappy_init(&f, 240, 320, 1234u));
    return f;
}

static void test_initial_state(void)
{
    flappy_t f = make();
    assert(f.state == FLAPPY_READY);
    assert(f.score == 0);
    assert(f.bird_vy == 0);
    assert(flappy_play_height(&f) == 320 - FLAPPY_GROUND_H);
    /* Bird starts around the middle of the playfield, not on the floor. */
    assert(f.bird_y > 0 && f.bird_y < flappy_play_height(&f));
}

static void test_first_tap_starts_and_flaps(void)
{
    flappy_t f = make();
    flappy_flap(&f, 100);
    assert(f.state == FLAPPY_PLAY);
    assert(f.bird_vy < 0);           /* moving up */

    /* A second tap while playing only re-flaps. */
    flappy_update(&f, 100, 200);
    flappy_flap(&f, 200);
    assert(f.state == FLAPPY_PLAY);
    assert(f.bird_vy < 0);
}

static void test_gravity_pulls_the_bird_down(void)
{
    flappy_t f = make();
    flappy_flap(&f, 0);
    float v0 = f.bird_vy;
    assert(v0 < 0);                       /* the flap pushes up */
    flappy_update(&f, 16, 16);
    /* Gravity increases vy every step, even while still rising. */
    assert(f.bird_vy > v0);
    float v1 = f.bird_vy;
    flappy_update(&f, 16, 32);
    assert(f.bird_vy > v1);
}

static void test_falling_to_the_ground_ends_the_run(void)
{
    flappy_t f = make();
    flappy_flap(&f, 0);
    uint64_t now = 0;
    for (int i = 0; i < 400 && f.state == FLAPPY_PLAY; i++) {
        now += 16;
        flappy_update(&f, 16, now);
    }
    assert(f.state == FLAPPY_DEAD);
}

static void test_pipes_move_left_and_recycle(void)
{
    flappy_t f = make();
    flappy_flap(&f, 0);
    float first_x = f.pipes[0].x;
    flappy_update(&f, 16, 16);
    assert(f.pipes[0].x < first_x);

    /* Run long enough that at least one pipe wraps around. */
    uint64_t now = 16;
    float far = 0;
    for (int i = 0; i < 4000; i++) {
        now += 16;
        flappy_update(&f, 16, now);
        for (int j = 0; j < FLAPPY_PIPES; j++) {
            if (f.pipes[j].x > far) far = f.pipes[j].x;
        }
        if (f.state != FLAPPY_PLAY) {
            /* Keep the sim alive by resetting the bird to mid-screen. */
            f.state = FLAPPY_PLAY;
            f.bird_y = flappy_play_height(&f) * 0.4f;
            f.bird_vy = 0;
            for (int j = 0; j < FLAPPY_PIPES; j++) f.pipes[j].scored = true;
        }
    }
    /* Recycling must keep pipes inside a sane band around the screen. */
    for (int j = 0; j < FLAPPY_PIPES; j++) {
        assert(f.pipes[j].x > -60 && f.pipes[j].x < 240 + 70 + 3 * 150 + 10);
    }
}

static void test_scoring_counts_passed_pipes(void)
{
    flappy_t f = make();
    flappy_flap(&f, 0);
    /* Put one pipe just ahead of the bird and let it pass. */
    f.pipes[0].x = 52 - 54 - 1;
    f.pipes[0].gap_y = flappy_play_height(&f) / 2.0f;
    f.pipes[0].scored = false;
    int before = f.score;
    flappy_update(&f, 16, 16);
    assert(f.score == before + 1);
}

static void test_retry_is_blocked_briefly(void)
{
    flappy_t f = make();
    flappy_flap(&f, 0);
    uint64_t now = 0;
    for (int i = 0; i < 400 && f.state == FLAPPY_PLAY; i++) {
        now += 16;
        flappy_update(&f, 16, now);
    }
    assert(f.state == FLAPPY_DEAD);

    /* Tapping immediately must not restart. */
    flappy_flap(&f, now);
    assert(f.state == FLAPPY_DEAD);
    /* After the pause it restarts. */
    flappy_flap(&f, now + 800);
    assert(f.state == FLAPPY_READY || f.state == FLAPPY_PLAY);
}

int main(void)
{
    test_initial_state();
    test_first_tap_starts_and_flaps();
    test_gravity_pulls_the_bird_down();
    test_falling_to_the_ground_ends_the_run();
    test_pipes_move_left_and_recycle();
    test_scoring_counts_passed_pipes();
    test_retry_is_blocked_briefly();
    printf("flappy: all tests passed\n");
    return 0;
}
