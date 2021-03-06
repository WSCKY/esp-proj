#pragma once
#include <stdint.h>
#include "esp_err.h"
#include "ll_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Decode the jpeg ``image.jpg`` embedded into the program file into pixel data.
 *
 * @param pixels A pointer to a pointer for an array of rows, which themselves are an array of pixels.
 *        Effectively, you can get the pixel data by doing ``decode_image(&myPixels); pixelval=myPixels[ypos][xpos];``
 * @return - ESP_ERR_NOT_SUPPORTED if image is malformed or a progressive jpeg file
 *         - ESP_ERR_NO_MEM if out of memory
 *         - ESP_OK on succesful decode
 */
esp_err_t jpg_decode(const char *path, pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen, LcdSize_t size);

#ifdef __cplusplus
}
#endif
