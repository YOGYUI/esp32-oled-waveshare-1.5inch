#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oledctrl.h"
#include "driver/gpio.h"
#include "definition.h"
#include "logger.h"

COLEDCtrl* COLEDCtrl::_instance = nullptr;

COLEDCtrl::COLEDCtrl()
{
    m_spi_dev_handle = nullptr;
    memset(&m_spi_transaction, 0, sizeof(m_spi_transaction));
    m_spi_transaction.length = 8;
    m_spi_transaction.rxlength = 0;
    m_spi_transaction.user = nullptr;

    m_is_on = false;
    m_width = 128;
    m_height = 128;
    m_buffer_len = size_t(m_width) * size_t(m_height) / SSD1327_PIXELSPERBYTE;
    m_brightness = 0.f;
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
    if (!init_gpio()) {
        return false;
    }
    if (!init_spi_bus()) {
        return false;
    }
    if (!add_device_spi_bus()) {
        return false;
    }

    setup();
    return true;
}

bool COLEDCtrl::init_gpio()
{
    esp_err_t ret;
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = 1ULL << PIN_OLED_RST;
    cfg.mode = GPIO_MODE_OUTPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;

    ret = gpio_config(&cfg);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to initialize gpio reset pin (ret: %d)", ret);
        return false;
    }

    cfg.pin_bit_mask = 1ULL << PIN_OLED_DC;
    ret = gpio_config(&cfg);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to initialize gpio dat/cmd pin (ret: %d)", ret);
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

bool COLEDCtrl::spi_transfer(uint8_t data)
{
    if (!m_spi_dev_handle) {
        GetLogger(eLogType::Error)->Log("spi handle is not initialized");
        return false;
    }

    m_spi_transaction.tx_buffer = &data;
    esp_err_t ret = spi_device_polling_transmit(m_spi_dev_handle, &m_spi_transaction);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to set potentiometer value");
        return false;
    }

    return true;
}

void COLEDCtrl::turn_on()
{
    command(SSD1327_DISPLAYON);
    m_is_on = true;
}

void COLEDCtrl::turn_off()
{
    command(SSD1327_DISPLAYOFF);
    m_is_on = false;
}

void COLEDCtrl::reset()
{
    gpio_set_level((gpio_num_t)PIN_OLED_RST, 1);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)PIN_OLED_RST, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)PIN_OLED_RST, 1);
}

void COLEDCtrl::setup()
{
    reset();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    turn_off();                             // display OFF
    command(SSD1327_SETFRONTCLOCKDIVIDER);  // set osc division
    command(0xF1);                          // 145
    command(SSD1327_SETMULTIPLEX);          // multiplex ratio
    command(0x7f);                          // duty = height - 1
    command(SSD1327_SETOFFSET);             // set display offset
    command(0x00);                          // 0
    command(SSD1327_SETSTARTLINE);          // set start line
    command(0x00);                          // ...
    command(SSD1327_SETREMAP);              // set segment remapping
    command(0x53);                          //  COM bottom-up, split odd/even, enable column and nibble remapping
    command(SSD1327_SETGRAYSCALETABLE);

    command(0);
    command(1);
    command(2);
    command(3);
    command(6);
    command(8);
    command(12);
    command(16);
    command(20);
    command(26);
    command(32);
    command(39);
    command(46);
    command(54);
    command(63);
    command(SSD1327_SETPHASELENGTH);
    command(0x55);
    command(SSD1327_SETVCOMHVOLTAGE);   // Set High Voltage Level of COM Pin
    command(0x1C);
    command(SSD1327_SETGPIO);           // Switch voltage converter on (for Aliexpress display)
    command(0x03);
    command(SSD1327_NORMALDISPLAY);     // set display mode

    set_brightness(1.f);
    // fill(Color::BLACK);                 // clear display - ensures we do not see garbage at power-on
    display();                          // ...write buffer, which actually clears the display's memory
    turn_on();                          // display ON
}

void COLEDCtrl::command(uint8_t value)
{
    gpio_set_level((gpio_num_t)PIN_OLED_DC, 0);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    spi_transfer(value);
}

void COLEDCtrl::write_display_data()
{

}

void COLEDCtrl::display()
{
    command(SSD1327_SETCOLUMNADDRESS);  // set column address
    command(0x00);                      // set column start address
    command(0x3F);                      // set column end address
    command(SSD1327_SETROWADDRESS);     // set row address
    command(0x00);                      // set row start address
    command(127);                       // set last row

    write_display_data();
}

void COLEDCtrl::set_brightness(float brightness)
{
    m_brightness = MAX(0.f, MIN(1.f, brightness));
    this->command(SSD1327_SETCONTRAST);
    this->command(int(SSD1327_MAX_CONTRAST * m_brightness));
}