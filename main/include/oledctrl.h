#ifndef _OLED_CTRL_H_
#define _OLED_CTRL_H_

#include "driver/spi_master.h"
#include "definition.h"
#include "fonts.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

class COLEDCtrl
{
public:
    COLEDCtrl();
    virtual ~COLEDCtrl();
    static COLEDCtrl* Instance();

public:
    bool initialize();

    void turn_on();
    void turn_off();
    void reset();
    void update();
    void set_brightness(float brightness);
    void set_pixel_value(uint16_t x, uint16_t y, uint8_t color);
    void set_entire_display_on();
    void set_entire_display_off();

    uint16_t get_width()    { return m_width;   }
    uint16_t get_height()   { return m_height;  }

    void clear();
    void draw_point(uint16_t x, uint16_t y, uint8_t color, uint8_t size);
    void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t width);
    void draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t width);
    void draw_circle(uint16_t x, uint16_t y, uint16_t rad, uint8_t color, uint8_t width);
    void draw_character(uint16_t x, uint16_t y, const char character, sFONT* font, uint8_t color);
    void draw_string(uint16_t x, uint16_t y, const char *string, sFONT* font, uint8_t color);
    // void draw_number(uint16_t x, uint16_t y, const char *number, sFONT* font, uint16_t digit, uint8_t color);

private:
    static COLEDCtrl* _instance;
    spi_device_handle_t m_spi_dev_handle;
    spi_transaction_t m_spi_transaction;
    std::vector<uint8_t> m_buffer;
    uint16_t m_width, m_height;
    size_t m_buffer_len;
    float m_brightness;
    bool m_is_on;

    bool init_gpio();
    bool init_spi_bus();
    bool add_device_spi_bus();
    bool spi_transfer(uint8_t data);
    bool spi_transfer(uint8_t* buffer, size_t buffer_len);

    void setup();
    void command(uint8_t value);
    void write_display_data();

    void display();
};

inline COLEDCtrl* GetOLEDCtrl() {
    return COLEDCtrl::Instance();
}

#ifdef __cplusplus
};
#endif
#endif