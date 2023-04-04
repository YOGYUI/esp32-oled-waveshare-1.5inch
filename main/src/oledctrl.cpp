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
    m_spi_transaction.rxlength = 0;
    m_spi_transaction.user = nullptr;

    m_is_on = false;
    m_width = 128;
    m_height = 128;
    m_buffer_len = size_t(m_width) * size_t(m_height) / SSD1327_PIXELSPERBYTE;
    m_buffer.resize(m_buffer_len);
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
    // cfg_bus.max_transfer_sz = 4092;
    cfg_bus.max_transfer_sz = m_buffer.size();
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
    cfg_dev_if.clock_speed_hz = 8 * 1000 * 1000;    // Max 25MHz
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

    m_spi_transaction.length = 8;
    m_spi_transaction.tx_buffer = &data;
    esp_err_t ret = spi_device_polling_transmit(m_spi_dev_handle, &m_spi_transaction);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to transfer");
        return false;
    }

    return true;
}

bool COLEDCtrl::spi_transfer(uint8_t* buffer, size_t buffer_len)
{
    if (!m_spi_dev_handle) {
        GetLogger(eLogType::Error)->Log("spi handle is not initialized");
        return false;
    }

    m_spi_transaction.length = 8 * buffer_len;
    m_spi_transaction.tx_buffer = buffer;
    esp_err_t ret = spi_device_polling_transmit(m_spi_dev_handle, &m_spi_transaction);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("failed to transfer");
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

void COLEDCtrl::update()
{
    display();
}

void COLEDCtrl::setup()
{
    reset();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    turn_off();                             // display OFF
    command(SSD1327_FUNCTIONSELECTIONA);
    command(0x01);
    command(SSD1327_SETFRONTCLOCKDIVIDER);  // set osc division
    command(0xF0);
    command(SSD1327_SETPRECHARGEVOLTAGE);
    command(0x08);

    command(SSD1327_SETMULTIPLEX);          // multiplex ratio (16 ~ 128)
    command(0x7F);                          // 128 MUX
    command(SSD1327_SETOFFSET);             // set display offset
    command(0x00);                          // 0
    command(SSD1327_SETSTARTLINE);          // set start line
    command(0x00);                          // ...
    command(SSD1327_SETREMAP);              // set segment remapping
    uint8_t value = 0;
    value |= 0x01;      // Enable Column Addres Remapping (bit0)
    //value |= 0x02;      // Enable Nibble Remapping (bit1)
    //value |= 0x04;      // Address increment mode (bit2)
    value |= 0x10;      // COM remapping (bit4)
    value |= 0x40;      // Split odd/even COM signal (bit6)
    command(value);
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
    // command(0x1C);
    command(0x07);
    command(SSD1327_SETSECONDPRECHARGEPERIOD);
    command(0x01);
    command(SSD1327_SETGPIO);           // Switch voltage converter on (for Aliexpress display)
    // command(0x03);
    command(0x00);
    command(SSD1327_NORMALDISPLAY);     // set display mode

    set_brightness(1.f);
    memset(&m_buffer[0], 0x00, m_buffer.size());
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
    gpio_set_level((gpio_num_t)PIN_OLED_DC, 1);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    spi_transfer(&m_buffer[0], m_buffer.size());
}

void COLEDCtrl::display()
{
    command(SSD1327_SETCOLUMNADDRESS);  // set column address
    command(0x00);                      // set column start address
    command(0x3F);                      // set column end address
    command(SSD1327_SETROWADDRESS);     // set row address
    command(0x00);                      // set row start address
    command(0x7F);                      // set last row

    write_display_data();
}

void COLEDCtrl::set_brightness(float brightness)
{
    m_brightness = MAX(0.f, MIN(1.f, brightness));
    this->command(SSD1327_SETCONTRAST);
    this->command(int(SSD1327_MAX_CONTRAST * m_brightness));
}

void COLEDCtrl::set_pixel_value(uint16_t x, uint16_t y, uint8_t color)
{
    if (x >= m_width || y >= m_height) {
        return;
    }

    uint16_t pos = (x / SSD1327_PIXELSPERBYTE) + (y * m_width / SSD1327_PIXELSPERBYTE);
    uint8_t shift = (x % SSD1327_PIXELSPERBYTE) * SSD1327_COLORSHIFT;
    uint32_t temp = (uint32_t(color) & SSD1327_COLORMASK) << shift;
    m_buffer[pos] &= (~SSD1327_COLORMASK >> shift);
    m_buffer[pos] |= temp;
}

void COLEDCtrl::set_entire_display_on()
{
    command(SSD1327_DISPLAYALLON);
}

void COLEDCtrl::set_entire_display_off()
{
    command(SSD1327_DISPLAYALLOFF);
}

void COLEDCtrl::clear()
{
    memset(&m_buffer[0], 0x00, m_buffer.size());
    update();
}

void COLEDCtrl::draw_point(uint16_t x, uint16_t y, uint8_t color, uint8_t size)
{
    int16_t xtemp , ytemp;
    for (xtemp = 0; xtemp < 2 * size - 1; xtemp++) {
        for (ytemp = 0; ytemp < 2 * size - 1; ytemp++) {
            if(x + xtemp - size < 0 || y + ytemp - size < 0)
                break;
            set_pixel_value(x + xtemp - size, y + ytemp - size, color);
        }
    }
}

void COLEDCtrl::draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t width)
{
    if (x1 >= m_width || y1 >= m_height || x2 >= m_width || y2 >= m_height) {
        return;
    }

    uint16_t xtemp = x1;
    uint16_t ytemp = y1;
    int dx = (int)x2 - (int)x1 >= 0 ? x2 - x1 : x1 - x2;
    int dy = (int)y2 - (int)y1 <= 0 ? y2 - y1 : y1 - y2;

    int xstep = x1 < x2 ? 1 : -1;
    int ystep = y1 < y2 ? 1 : -1;
    int esp = dx + dy;

    for (;;) {
        draw_point(xtemp, ytemp, color, width);
        if (2 * esp >= dy) {
            if (xtemp == x2)
                break;
            esp += dy;
            xtemp += xstep;
        }

        if (2 * esp <= dx) {
            if (ytemp == y2)
                break;
            esp += dx;
            ytemp += ystep;
        }
    }
}

void COLEDCtrl::draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t width)
{
    draw_line(x1, y1, x2, y1, color, width);
    draw_line(x1, y1, x1, y2, color, width);
    draw_line(x2, y2, x2, y1, color, width);
    draw_line(x2, y2, x1, y2, color, width);
}

void COLEDCtrl::draw_circle(uint16_t x, uint16_t y, uint16_t rad, uint8_t color, uint8_t width)
{
    int16_t xtemp = 0;
    int16_t ytemp = rad;
    int16_t esp = 3 - (rad << 1);

    while (xtemp <= ytemp) {
        draw_point(x + xtemp, y + ytemp, color, width);
        draw_point(x - xtemp, y + ytemp, color, width);
        draw_point(x - ytemp, y + xtemp, color, width);
        draw_point(x - ytemp, y - xtemp, color, width);
        draw_point(x - xtemp, y - ytemp, color, width);
        draw_point(x + xtemp, y - ytemp, color, width);
        draw_point(x + ytemp, y - xtemp, color, width);
        draw_point(x + ytemp, y + xtemp, color, width);

        if (esp < 0)
            esp += 4 * xtemp + 6;
        else {
            esp += 10 + 4 * (xtemp - ytemp);
            ytemp --;
        }
        xtemp ++;
    }
}

void COLEDCtrl::draw_character(uint16_t x, uint16_t y, const char character, sFONT* font, uint8_t color)
{
    uint32_t offset = (character - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &font->table[offset];

    for (uint16_t page = 0; page < font->Height; page ++) {
        for (uint16_t column = 0; column < font->Width; column ++) {
            if (*ptr & (0x80 >> (column % 8)))
                set_pixel_value(x + column, y + page, color);
            else
                set_pixel_value(x + column, y + page, 0x00);

            if (column % 8 == 7) {
                ptr++;
            }
        }

        if (font->Width % 8 != 0) {
            ptr++;
        }
    }
}

void COLEDCtrl::draw_string(uint16_t x, uint16_t y, const char *string, sFONT* font, uint8_t color)
{
    uint16_t xtemp = x;
    uint16_t ytemp = y;

    while (*string != '\0') {
        if ((xtemp + font->Width) > m_width) {
            xtemp = x;
            ytemp += font->Height;
        }

        if ((ytemp  + font->Height) > m_height) {
            xtemp = x;
            ytemp = y;
        }
        draw_character(xtemp, ytemp, *string, font, color);

        string++;
        xtemp += font->Width;
    }
}

/*
void COLEDCtrl::draw_number(uint16_t x, uint16_t y, const char *number, sFONT* font, uint16_t digit, uint8_t color)
{
    uint8_t Str_Array[100] = {0};
    uint8_t *pStr = Str_Array;
    uint8_t i, len = 0;
    int16_t arr[3] = {0, 0, 0};
    int16_t *p = arr;

    while(number[len] != '\0') {  
      len++;                                    //get total length
      (*p)++;                                   //get the integer part length 
      if(number[len] == '.') {
        arr[2] = 1;
        arr[0]--;
        p++;               //get fractional part length
      }
    }

    if(digit > 0) {    
      if(digit <= arr[1]) {                     
        for(i=0; i<=len-(arr[1]-digit); i++)      //cut some Number
          Str_Array[i] = number[i];
      }
      else {
        for(i=0; i<=len+digit-arr[1]; i++) {
          if(i == len && arr[2] == 0)
            Str_Array[i] = '.';
          else if(i >= len)                           //add '0'
            Str_Array[i] = '0';
          else
            Str_Array[i] = number[i];
        }
      }
    }
    else
      for(i=0; i<=len-arr[1]-arr[2]; i++) {
        Str_Array[i] = number[i];
        }
  
    //show
    draw_string(x, y, (const char*)pStr, Font, Color_Background, Color_Foreground);
}
*/