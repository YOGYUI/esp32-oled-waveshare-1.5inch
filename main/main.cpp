#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "logger.h"
#include "oledctrl.h"

extern "C" void app_main(void)
{
    esp_err_t err;

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create event loop (ret: %d)", err);
    }

    COLEDCtrl *oled = GetOLEDCtrl();
    oled->initialize();

    uint8_t value;
    /*
    for (uint16_t y = 0; y < oled->get_height(); y++) {
        value = (uint8_t)(float(y) / float(oled->get_height()) * 255.f);
        for (uint16_t x = 0; x < oled->get_width(); x++) {
            oled->set_pixel_value(x, y, value);
        }
    }
    */
   /*
   for (uint16_t x = 0; x < oled->get_width(); x++) {
        value = (uint8_t)(float(x) / float(oled->get_width()) * 255.f);
        for (uint16_t y = 0; y < oled->get_height(); y++) {
            oled->set_pixel_value(x, y, value);
        }
    }
    */
   /*
    for (uint16_t x = 0; x < oled->get_width(); x++) {
        for (uint16_t y = 0; y < oled->get_height(); y++) {
            oled->set_pixel_value(x, y, 255);
        }
    }
    oled->update();
    */
    for (;;) {
        oled->set_entire_display_on();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        oled->set_entire_display_off();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}