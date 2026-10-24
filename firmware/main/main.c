#include "esp_log.h"

/* Integration smoke test only. Do not initialize unknown board GPIOs here. */
void app_main(void)
{
    ESP_LOGI("bootstrap", "C3 bootstrap ready; display and touch are not configured.");
}
