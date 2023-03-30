#include "oledctrl.h"
#include "definition.h"
#include "logger.h"
#include "driver/gpio.h"

COLEDCtrl* COLEDCtrl::_instance = nullptr;

COLEDCtrl::COLEDCtrl()
{
    m_spi_dev_handle = nullptr;
    memset(&m_spi_transaction, 0, sizeof(m_spi_transaction));
    m_spi_transaction.length = 8;
    m_spi_transaction.rxlength = 0;
    m_spi_transaction.user = nullptr;
}

COLEDCtrl::~COLEDCtrl()
{
}

COLEDCtrl* COLEDCtrl::Instance()
{
    if (!_instance) {
        _instance = new COLEDCtrl();
    }

    return _instance;
}

bool COLEDCtrl::initialize()
{
    if (!init_spi_bus()) {
        return false;
    }

    if (!add_device_spi_bus()) {
        return false;
    }

    return true;
}

bool COLEDCtrl::init_spi_bus()
{
    esp_err_t ret;
    spi_bus_config_t cfg_bus;
    memset(&cfg_bus, 0, sizeof(cfg_bus));

    cfg_bus.mosi_io_num = PIN_OLED_MOSI;
    cfg_bus.miso_io_num = PIN_OLED_MISO;
    cfg_bus.sclk_io_num = PIN_OLED_SCLK;
    cfg_bus.quadwp_io_num = -1;
    cfg_bus.quadhd_io_num = -1;
    cfg_bus.data4_io_num = -1;
    cfg_bus.data5_io_num = -1;
    cfg_bus.data6_io_num = -1;
    cfg_bus.data7_io_num = -1;
    cfg_bus.max_transfer_sz = 4092;
    // cfg_bus.isr_cpu_id = INTR_CPU_ID_AUTO;   // esp-idf >= v5.0
    cfg_bus.intr_flags = 0;

    ret = spi_bus_initialize(OLED_SPI_HOST, &cfg_bus, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to initialize spi bus (ret: %d)", ret);
        return false;
    }

    return true;
}

bool COLEDCtrl::add_device_spi_bus()
{
    esp_err_t ret;
    spi_device_interface_config_t cfg_dev_if;
    memset(&cfg_dev_if, 0, sizeof(cfg_dev_if));
    // cfg_dev_if.clock_source = SPI_CLK_SRC_DEFAULT;   // esp-idf >= v5.0
    cfg_dev_if.clock_speed_hz = 10 * 1000 * 1000;    // Max 25MHz
    cfg_dev_if.mode = 0;
    cfg_dev_if.spics_io_num = PIN_OLED_CS;
    cfg_dev_if.pre_cb = nullptr;
    cfg_dev_if.post_cb = nullptr;
    cfg_dev_if.queue_size = 7;
    ret = spi_bus_add_device(OLED_SPI_HOST, &cfg_dev_if, &m_spi_dev_handle);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to add device to spi bus (ret: %d)", ret);
        return false;
    }


    return true;
}