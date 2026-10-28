/* Host tests for the snake rules: grid, movement, growth, collision. */
#include <assert.h>
#include <stdio.h>

#include "snake.h"

static snake_t make(int w, int h)
{
    snake_t s;
    assert(snake_init(&s, w, h, 12345u) == 0);
    return s;
}

static void test_init_and_start(void)
{
    snake_t s = make(12, 14);
    assert(s.length == 0);
    assert(s.state == SNAKE_READY);

    snake_start(&s, 1000);
    assert(s.length == 3);
    assert(s.state == SNAKE_RUNNING);
    assert(s.score == 0);
    /* Head starts in the middle, body trails to the left. */
    assert(s.body_x[0] == 6 && s.body_y[0] == 7);
    assert(s.body_x[1] == 5 && s.body_x[2] == 4);
    /* Food must never spawn on the snake. */
    assert(!snake_body_at(&s, s.food_x, s.food_y));
    snake_free(&s);
}

static void test_tick_respects_interval(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 1000);
    int head = s.body_x[0];

    assert(!snake_tick(&s, 1000 + 1));          /* too early */
    assert(s.body_x[0] == head);
    assert(snake_tick(&s, 1000 + s.step_ms));   /* exactly due */
    assert(s.body_x[0] == head + 1);
    snake_free(&s);
}

static void test_turn_rejects_reversal(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 0);                        /* moving right */
    assert(!snake_turn(&s, SNAKE_LEFT));       /* would reverse */
    assert(s.dir == SNAKE_RIGHT);
    assert(snake_turn(&s, SNAKE_UP));
    /* Turns are queued: the heading only changes when the next step runs. */
    assert(s.dir == SNAKE_RIGHT);
    snake_free(&s);
}

static void test_turn_is_applied_on_the_next_step(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    assert(snake_turn(&s, SNAKE_UP));
    snake_tick(&s, s.last_step_ms + s.step_ms);
    assert(s.dir == SNAKE_UP);
    snake_free(&s);
}

static void test_two_quick_turns_are_both_honoured(void)
{
    /* Moving right, a fast "up, left" swipe: both turns must land, one per step,
     * instead of the second one being dropped. */
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    assert(snake_turn(&s, SNAKE_UP));
    assert(snake_turn(&s, SNAKE_LEFT));

    snake_tick(&s, s.last_step_ms + s.step_ms);
    assert(s.dir == SNAKE_UP);
    snake_tick(&s, s.last_step_ms + s.step_ms);
    assert(s.dir == SNAKE_LEFT);
    assert(s.state == SNAKE_RUNNING);
    snake_free(&s);
}

static void test_turn_queue_drops_extras(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    assert(snake_turn(&s, SNAKE_UP));
    assert(snake_turn(&s, SNAKE_LEFT));
    assert(!snake_turn(&s, SNAKE_DOWN));   /* queue holds two */
    snake_free(&s);
}

static void test_queued_turn_cannot_fold_the_neck(void)
{
    /* "up then down" would reverse relative to the queued heading. */
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    assert(snake_turn(&s, SNAKE_UP));
    assert(!snake_turn(&s, SNAKE_DOWN));
    snake_free(&s);
}

static void test_eating_grows_and_scores(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    int target_x = s.body_x[0] + 1;
    int target_y = s.body_y[0];
    s.food_x = target_x;
    s.food_y = target_y;
    uint32_t step = s.step_ms;

    assert(snake_tick(&s, s.last_step_ms + step));
    assert(s.score == 10);
    assert(s.length == 4);
    /* The head moved onto where the food was, not onto the new food. */
    assert(s.body_x[0] == target_x && s.body_y[0] == target_y);
    /* A new food appeared somewhere the body is not. */
    assert(!snake_body_at(&s, s.food_x, s.food_y));
    snake_free(&s);
}

static void test_wall_collision_ends_game(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    snake_turn(&s, SNAKE_UP);
    for (int i = 0; i < 20 && s.state == SNAKE_RUNNING; i++) {
        snake_tick(&s, s.last_step_ms + s.step_ms);
    }
    assert(s.state == SNAKE_OVER);
    snake_free(&s);
}

static void test_self_collision_ends_game(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    /* Build a body where the cell in front of the head is occupied. */
    s.length = 5;
    s.body_x[0] = 5; s.body_y[0] = 8;   /* head */
    s.body_x[1] = 5; s.body_y[1] = 7;
    s.body_x[2] = 6; s.body_y[2] = 7;
    s.body_x[3] = 6; s.body_y[3] = 8;   /* straight ahead */
    s.body_x[4] = 6; s.body_y[4] = 9;
    s.dir = SNAKE_RIGHT;

    snake_tick(&s, s.last_step_ms + s.step_ms);
    assert(s.state == SNAKE_OVER);
    snake_free(&s);
}

static void test_moving_into_tail_cell_is_allowed(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 0);
    /* Head at (4,7) moving up into (4,6), which is the tail and vacates. */
    s.length = 4;
    s.body_x[0] = 4; s.body_y[0] = 7;
    s.body_x[1] = 3; s.body_y[1] = 7;
    s.body_x[2] = 3; s.body_y[2] = 6;
    s.body_x[3] = 4; s.body_y[3] = 6;
    s.dir = SNAKE_UP;

    s.food_x = 0; s.food_y = 0;
    snake_tick(&s, s.last_step_ms + s.step_ms);
    assert(s.state == SNAKE_RUNNING);   /* (4,6) is the tail and moves away */
    snake_free(&s);
}

static void test_food_never_lands_on_body(void)
{
    snake_t s = make(12, 14);
    snake_start(&s, 7);
    for (int round = 0; round < 50; round++) {
        assert(!snake_body_at(&s, s.food_x, s.food_y));
        s.food_x = s.body_x[0] + 1;
        s.food_y = s.body_y[0];
        if (s.food_x >= s.width) s.food_x = 0;
        snake_tick(&s, s.last_step_ms + s.step_ms);
        if (s.state != SNAKE_RUNNING) break;
    }
    snake_free(&s);
}

int main(void)
{
    test_init_and_start();
    test_tick_respects_interval();
    test_turn_rejects_reversal();
    test_turn_is_applied_on_the_next_step();
    test_two_quick_turns_are_both_honoured();
    test_turn_queue_drops_extras();
    test_queued_turn_cannot_fold_the_neck();
    test_eating_grows_and_scores();
    test_wall_collision_ends_game();
    test_self_collision_ends_game();
    test_moving_into_tail_cell_is_allowed();
    test_food_never_lands_on_body();
    printf("snake: all tests passed\n");
    return 0;
}
