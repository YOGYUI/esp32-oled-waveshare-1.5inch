#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "logger.h"
#include "oledctrl.h"
#include "fonts.h"

extern "C" void app_main(void)
{
    esp_err_t err;

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create event loop (ret: %d)", err);
    }

    COLEDCtrl *oled = GetOLEDCtrl();
    oled->initialize();

    /*
    uint8_t value;
    for (uint16_t y = 0; y < oled->get_height(); y++) {
        value = (uint8_t)(float(y) / float(oled->get_height()) * 16.f);
        for (uint16_t x = 0; x < oled->get_width(); x++) {
            oled->set_pixel_value(x, y, value);
        }
    }
    oled->update();
    */

    oled->draw_point(20, 10, 0xFF, 1);
    oled->draw_point(30, 10, 0xFF, 2);
    oled->draw_point(40, 10, 0xFF, 3);
    oled->draw_line(10, 10, 10, 20, 0xFF, 1);
    oled->draw_line(20, 20, 20, 30, 0xFF, 1);
    oled->draw_line(30, 30, 30, 40, 0xFF, 1);
    oled->draw_line(40, 40, 40, 50, 0xFF, 1);
    oled->draw_circle(60, 30, 15, 0xFF, 1);
    oled->draw_circle(100, 40, 20, 0xFF, 1);
    oled->draw_rect(50, 30, 60, 40, 0xFF, 1);
    
    oled->draw_string(10, 50, "waveshare", &Font16, 0xFF);
    oled->draw_string(10, 67, "hello world", &Font8, 0xFF);
    oled->draw_string(10, 80, "YOGYUI", &Font24, 0xFF);
    oled->draw_string(10, 105, "192.168.0.1", &Font12, 0xFF);

    oled->update();
}