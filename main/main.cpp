#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "logger.h"

extern "C" void app_main(void)
{
    esp_err_t err;

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create event loop (ret: %d)", err);
    }
}