/* Flappy presentation: sky, pipes, ground, bird, HUD. App-local. */
#pragma once

#include "canvas.h"
#include "flappy.h"

dada_err_t flappy_render_init(dada_display_t *display);
void flappy_render_free(void);

/** Full repaint of the current frame. */
dada_err_t flappy_render_frame(const flappy_t *f);
