#ifndef _OLED_CTRL_H_
#define _OLED_CTRL_H_

#include "driver/spi_master.h"

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

private:
    static COLEDCtrl* _instance;
    spi_device_handle_t m_spi_dev_handle;
    spi_transaction_t m_spi_transaction;

    bool init_spi_bus();
    bool add_device_spi_bus();
};

inline COLEDCtrl* GetOLEDCtrl() {
    return COLEDCtrl::Instance();
}

#ifdef __cplusplus
};
#endif
#endif