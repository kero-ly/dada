/* Pomodoro screen: mm:ss, progress bar and the interaction hints.
 * App-local drawing, not a platform API. */
#pragma once

#include "clock.h"
#include "display.h"

dada_err_t focus_render_start(dada_display_t *display);
dada_err_t focus_render_update(const focus_clock_t *clock);
void focus_render_stop(void);
