#ifndef __LL_CONFIG_H
#define __LL_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"

#include "esp_system.h"

typedef struct {
	uint16_t left, right, top, bottom;
} ImgArea_t;

typedef struct {
	uint16_t width, height;
} LcdSize_t;

typedef esp_err_t (*pDrawPrepare_t)(ImgArea_t *);
typedef void (*pFillScreen_t)(const uint16_t *, uint16_t);

#endif /* __LL_CONFIG_H */
