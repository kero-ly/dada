/* Host tests for the pomodoro timing rules: no hardware involved. */
#include <assert.h>
#include <stdio.h>

#include "clock.h"

#define MINUTE 60000u
#define SESSION (25u * MINUTE)

static void test_initial_state(void)
{
    focus_clock_t c;
    clock_init(&c, SESSION);
    assert(c.state == CLOCK_IDLE);
    assert(clock_remaining_ms(&c) == SESSION);
    assert(c.completed == 0);
    assert(!clock_is_running(&c));

    /* Idle never counts down, even if time passes. */
    clock_tick(&c, 10000);
    assert(clock_remaining_ms(&c) == SESSION);
}

static void test_start_and_count(void)
{
    focus_clock_t c;
    clock_init(&c, SESSION);
    clock_toggle(&c, 1000);
    assert(clock_is_running(&c));
    clock_tick(&c, 2000);
    assert(clock_remaining_ms(&c) == SESSION - 1000);
    clock_tick(&c, 61000);
    assert(clock_remaining_ms(&c) == SESSION - 60000);
}

static void test_pause_freezes_and_resume_continues(void)
{
    focus_clock_t c;
    clock_init(&c, SESSION);
    clock_toggle(&c, 0);
    clock_tick(&c, 10000);
    assert(clock_remaining_ms(&c) == SESSION - 10000);

    clock_toggle(&c, 10000); /* pause */
    assert(c.state == CLOCK_PAUSED);
    clock_tick(&c, 90000);   /* 80 s pass while paused */
    assert(clock_remaining_ms(&c) == SESSION - 10000);

    clock_toggle(&c, 90000); /* resume */
    clock_tick(&c, 95000);
    assert(clock_remaining_ms(&c) == SESSION - 15000);
}

static void test_finishes_exactly_at_zero(void)
{
    focus_clock_t c;
    clock_init(&c, 5000);
    clock_toggle(&c, 0);
    clock_tick(&c, 4999);
    assert(c.state == CLOCK_RUNNING);
    assert(clock_remaining_ms(&c) == 1);
    clock_tick(&c, 5000);
    assert(c.state == CLOCK_DONE);
    assert(clock_remaining_ms(&c) == 0);
    assert(c.completed == 1);

    /* Overshooting the end must not go negative or double count. */
    clock_tick(&c, 600000);
    assert(clock_remaining_ms(&c) == 0);
    assert(c.state == CLOCK_DONE);
    assert(c.completed == 1);
}

static void test_clock_going_backwards_is_ignored(void)
{
    focus_clock_t c;
    clock_init(&c, SESSION);
    clock_toggle(&c, 100000);
    clock_tick(&c, 90000); /* clock stepped back */
    assert(clock_remaining_ms(&c) == SESSION);
    clock_tick(&c, 101000);
    assert(clock_remaining_ms(&c) == SESSION - 1000);
}

static void test_done_restarts_and_reset_keeps_completed(void)
{
    focus_clock_t c;
    clock_init(&c, 1000);
    clock_toggle(&c, 0);
    clock_tick(&c, 1000);
    assert(c.state == CLOCK_DONE);

    /* Tapping after completion starts a fresh session. */
    clock_toggle(&c, 2000);
    assert(c.state == CLOCK_RUNNING);
    assert(clock_remaining_ms(&c) == 1000);
    assert(c.completed == 1);

    clock_tick(&c, 2500);
    clock_reset(&c);
    assert(c.state == CLOCK_IDLE);
    assert(clock_remaining_ms(&c) == 1000);
    assert(c.completed == 1);
}

static void test_reset_zone_matches_rendered_bottom_strip(void)
{
    assert(!clock_reset_hit(279, 320));
    assert(clock_reset_hit(280, 320));
    assert(!clock_reset_hit(159, 200));
    assert(clock_reset_hit(160, 200));
    assert(!clock_reset_hit(-1, 320));
    assert(!clock_reset_hit(0, 0));
}

int main(void)
{
    test_initial_state();
    test_start_and_count();
    test_pause_freezes_and_resume_continues();
    test_finishes_exactly_at_zero();
    test_clock_going_backwards_is_ignored();
    test_done_restarts_and_reset_keeps_completed();
    test_reset_zone_matches_rendered_bottom_strip();
    printf("clock: all tests passed\n");
    return 0;
}
