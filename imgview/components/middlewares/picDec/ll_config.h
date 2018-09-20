#ifndef __LL_CONFIG_H
#define __LL_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"

#include "esp_system.h"

typedef esp_err_t (*pDrawPrepare_t)(int16_t w, int16_t h);
typedef void (*pFillScreen_t)(const uint16_t *data, int16_t lines);

#endif /* __LL_CONFIG_H */
