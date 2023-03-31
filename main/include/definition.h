#ifndef _DEFINITION_H_
#define _DEFINITION_H_
#pragma once

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define OLED_SPI_HOST   HSPI_HOST
#define PIN_OLED_MOSI   23
#define PIN_OLED_MISO   22
#define PIN_OLED_SCLK   18
#define PIN_OLED_CS     21
#define PIN_OLED_DC     25
#define PIN_OLED_RST    26

#define SSD1327_MAX_CONTRAST                        127
#define SSD1327_COLORMASK                           0x0f
#define SSD1327_COLORSHIFT                          4
#define SSD1327_PIXELSPERBYTE                       2
#define SSD1327_SETCOLUMNADDRESS                    0x15
#define SSD1327_SETROWADDRESS                       0x75
#define SSD1327_SETCONTRAST                         0x81
#define SSD1327_SETREMAP                            0xA0
#define SSD1327_SETSTARTLINE                        0xA1
#define SSD1327_SETOFFSET                           0xA2
#define SSD1327_NORMALDISPLAY                       0xA4
#define SSD1327_DISPLAYALLON                        0xA5
#define SSD1327_DISPLAYALLOFF                       0xA6
#define SSD1327_INVERTDISPLAY                       0xA7
#define SSD1327_SETMULTIPLEX                        0xA8
#define SSD1327_FUNCTIONSELECTIONA                  0xAB
#define SSD1327_DISPLAYOFF                          0xAE
#define SSD1327_DISPLAYON                           0xAF
#define SSD1327_SETPHASELENGTH                      0xB1
#define SSD1327_SETFRONTCLOCKDIVIDER                0xB3
#define SSD1327_SETGPIO                             0xB5
#define SSD1327_SETSECONDPRECHARGEPERIOD            0xB6
#define SSD1327_SETGRAYSCALETABLE                   0xB8
#define SSD1327_SELECTDEFAULTLINEARGRAYSCALETABLE   0xB9
#define SSD1327_SETPRECHARGEVOLTAGE                 0xBC
#define SSD1327_SETVCOMHVOLTAGE                     0xBE
#define SSD1327_FUNCTIONSELECTIONB                  0xD5
#define SSD1327_SETCOMMANDLOCK                      0xFD
#define SSD1327_HORIZONTALSCROLLRIGHTSETUP          0x26
#define SSD1327_HORIZONTALSCROLLLEFTSETUP           0x27
#define SSD1327_DEACTIVATESCROLL                    0x2E
#define SSD1327_ACTIVATESCROLL                      0x2F

#endif