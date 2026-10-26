/* Hardware diagnostics through the same contracts as the counter. */
#include "app.h"

#include <stdio.h>
#include <stdlib.h>

static dada_app_services_t services;
static bool active, was_pressed;

dada_err_t dada_app_start(const dada_app_services_t *provided)
{
    static const uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFFF, 0x0000};
    dada_app_stop();
    if (!provided) return DADA_ERR_INVALID_ARG;
    if (!provided->display || !provided->display->flush || !provided->input ||
        !provided->input->poll || provided->display->height < 5 ||
        provided->display->color_depth != 16) return DADA_ERR_NOT_SUPPORTED;
    dada_display_t *d = provided->display;
    int height = d->height / 5;
    uint16_t *pixels = malloc(d->width * (height + 4) * sizeof(*pixels));
    if (!pixels) return DADA_ERR_FAIL;
    dada_err_t err = DADA_OK;
    for (int band = 0; band < 5; band++) {
        int top = band * height;
        int bottom = band == 4 ? d->height - 1 : top + height - 1;
        for (int i = 0; i < d->width * (bottom - top + 1); i++) pixels[i] = colors[band];
        dada_area_t area = {0, top, d->width - 1, bottom};
        err = d->flush(d->ctx, &area, pixels);
        if (err != DADA_OK) break;
        printf("bringup: band %d transferred\n", band);
    }
    free(pixels);
    if (err != DADA_OK) return err;
    services = *provided;
    was_pressed = false;
    active = true;
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    if (!active) return DADA_ERR_FAIL;
    dada_point_t point;
    bool pressed;
    if (services.input->poll(services.input->ctx, &point, &pressed) && pressed != was_pressed) {
        printf("bringup: %s at (%d, %d)\n", pressed ? "press" : "release", point.x, point.y);
        was_pressed = pressed;
    }
    return DADA_OK;
}

void dada_app_stop(void)
{
    active = false;
    services = (dada_app_services_t){0};
}
