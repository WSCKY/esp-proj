#ifndef __OLEDFONT_H
#define __OLEDFONT_H

#include "esp_system.h"

typedef struct {
    uint8_t Height;
    uint8_t Width;
    const unsigned char* table;
} type_font_t;

extern const type_font_t Font12x6;
extern const type_font_t Font16x8;
extern const type_font_t Font24x12;

#endif
