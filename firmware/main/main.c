/* Static composition root: owns hardware and scheduling, never application UI. */
#include "app.h"
#include "board.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app_host";

void app_main(void)
{
    dada_display_t *display = NULL;
    dada_input_t *input = NULL;
    dada_time_t *time = NULL;
    dada_err_t err = board_init(&display, &input, &time);
    if (err != DADA_OK) {
        ESP_LOGE(TAG, "board_init failed: %d", err);
        board_deinit(display, input);
        return;
    }
    dada_app_services_t services = {.display = display, .input = input, .time = time};
    err = dada_app_start(&services);
    if (err == DADA_OK) {
        ESP_LOGI(TAG, "%s started (%ux%u)", DADA_APP_ID, display->width, display->height);
        while ((err = dada_app_step()) == DADA_OK) {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
    ESP_LOGE(TAG, "%s stopped: %d", DADA_APP_ID, err);
    dada_app_stop();
    board_deinit(display, input);
}
