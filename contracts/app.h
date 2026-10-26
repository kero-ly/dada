/** Experimental static native-app lifecycle; no ABI/dynamic-loading guarantee. */
#pragma once

#include "display.h"
#include "input.h"
#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Borrowed capabilities: valid from start until stop; one app per firmware. */
typedef struct {
    dada_display_t *display;
    dada_input_t *input;
    dada_time_t *time;
} dada_app_services_t;

/** Start/render the app. A failed start must leave it safe to stop or retry. */
dada_err_t dada_app_start(const dada_app_services_t *services);
/** One cooperative iteration; synchronous capability calls may block. Host schedules. */
dada_err_t dada_app_step(void);
/** Release app-owned resources; may be called more than once. */
void dada_app_stop(void);

#ifdef __cplusplus
}
#endif
