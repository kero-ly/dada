/*
 * dada bring-up app.
 *
 * Draws a colour-bar test on the display and prints touch coordinates over the
 * serial monitor. This is the smallest program that proves both contracts and
 * the board binding work -- nothing here knows a pin number.
 */

#include <stdlib.h>

#include "board.h"
#include "dada_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app";

/* One horizontal strip of the test pattern. */
typedef struct {
    uint16_t color;
    const char *name;
} band_t;

static const band_t BANDS[] = {
    {0xF800, "red"},
    {0x07E0, "green"},
    {0x001F, "blue"},
    {0xFFFF, "white"},
    {0x0000, "black"},
};

void app_main(void)
{
    dada_display_t *display = NULL;
    dada_input_t *input = NULL;

    dada_err_t err = board_init(&display, &input);
    if (err != DADA_OK) {
        ESP_LOGE(TAG, "board_init failed: %d", err);
        return;
    }
    ESP_LOGI(TAG, "display %ux%u, touch %s",
             display ? display->width : 0,
             display ? display->height : 0,
             input ? "ok" : "absent");

    /* Draw bands using only the contract: one rectangle flush per band. */
    if (display) {
        const int w = display->width;
        const int bands = (int)(sizeof(BANDS) / sizeof(BANDS[0]));
        const int band_h = display->height / bands;
        uint16_t *band = malloc(sizeof(uint16_t) * w * band_h);
        if (band) {
            for (int b = 0; b < bands; b++) {
                for (int i = 0; i < w * band_h; i++) {
                    band[i] = BANDS[b].color;
                }
                int16_t top = (int16_t)(b * band_h);
                int16_t bottom = (int16_t)((b == bands - 1) ? display->height - 1
                                                            : top + band_h - 1);
                dada_area_t area = {0, top, (int16_t)(w - 1), bottom};
                dada_err_t flush_err = display->flush(display->ctx, &area, band);
                ESP_LOGI(TAG, "band %s %s", BANDS[b].name,
                         flush_err == DADA_OK ? "drawn" : "FAILED");
            }
            free(band);
            ESP_LOGI(TAG, "test pattern complete");
        } else {
            ESP_LOGE(TAG, "no memory for the band buffer");
        }
    }

    /* Report touches on state change only, so the log stays readable. */
    bool was_pressed = false;
    while (true) {
        if (input) {
            dada_point_t point = {0, 0};
            bool pressed = false;
            if (input->poll(input->ctx, &point, &pressed) && pressed != was_pressed) {
                ESP_LOGI(TAG, "%s at (%d, %d)", pressed ? "press" : "release",
                         point.x, point.y);
                was_pressed = pressed;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
