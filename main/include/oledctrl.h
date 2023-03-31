#ifndef _OLED_CTRL_H_
#define _OLED_CTRL_H_

#include "driver/spi_master.h"
#include "definition.h"

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

private:
    static COLEDCtrl* _instance;
    spi_device_handle_t m_spi_dev_handle;
    spi_transaction_t m_spi_transaction;
    uint16_t m_width, m_height;
    size_t m_buffer_len;
    float m_brightness;
    bool m_is_on;

    bool init_gpio();
    bool init_spi_bus();
    bool add_device_spi_bus();
    bool spi_transfer(uint8_t data);

    void setup();
    void command(uint8_t value);
    void write_display_data();

    void display();
    void set_brightness(float brightness);
};

inline COLEDCtrl* GetOLEDCtrl() {
    return COLEDCtrl::Instance();
}

#ifdef __cplusplus
};
#endif
#endif