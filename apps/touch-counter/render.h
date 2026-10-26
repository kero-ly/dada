/* Private counter UI; not a platform drawing API. */
#pragma once
#include "display.h"
dada_err_t counter_render_start(dada_display_t *display);
dada_err_t counter_render_value(uint16_t value);
void counter_render_stop(void);
