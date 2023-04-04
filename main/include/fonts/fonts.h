#ifndef _FONTS_H_
#define _FONTS_H_

#define MAX_HEIGHT_FONT 41
#define MAX_WIDTH_FONT  32
#define OFFSET_BITMAP           

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct _tFont
{    
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
} sFONT;

extern sFONT Font8;
extern sFONT Font12;
extern sFONT Font16;
extern sFONT Font20;
extern sFONT Font24;

#ifdef __cplusplus
}
#endif
#endif
