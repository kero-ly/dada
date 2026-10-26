#include "app.h"
#include "counter.h"
#include "render.h"

static dada_app_services_t services;
static uint16_t count;
static bool active, was_pressed;

uint16_t counter_count(void) { return count; }

dada_err_t dada_app_start(const dada_app_services_t *provided)
{
    dada_app_stop();
    if (!provided) return DADA_ERR_INVALID_ARG;
    if (!provided->display || !provided->display->flush ||
        !provided->input || !provided->input->poll) return DADA_ERR_NOT_SUPPORTED;
    dada_err_t err = counter_render_start(provided->display);
    if (err != DADA_OK) return err;
    services = *provided;
    count = 0;
    was_pressed = false;
    err = counter_render_value(count);
    if (err != DADA_OK) {
        dada_app_stop();
        return err;
    }
    active = true;
    return DADA_OK;
}

dada_err_t dada_app_step(void)
{
    if (!active) return DADA_ERR_FAIL;
    dada_point_t point;
    bool pressed;
    if (!services.input->poll(services.input->ctx, &point, &pressed)) return DADA_OK;
    bool clicked = pressed && !was_pressed;
    was_pressed = pressed;
    if (!clicked || point.x < 0 || point.y < 0 ||
        point.x >= services.display->width || point.y >= services.display->height) return DADA_OK;
    uint16_t next = point.y >= services.display->height * 3 / 4
                        ? 0 : (count < 9999 ? count + 1 : count);
    if (next == count) return DADA_OK;
    count = next;
    return counter_render_value(count);
}

void dada_app_stop(void)
{
    counter_render_stop();
    active = false;
    services = (dada_app_services_t){0};
}
