#ifndef __BMPDEC_H
#define __BMPDEC_H

#include "string.h"
#include "stdio.h"
#include "esp_err.h"
#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef esp_err_t (*pDrawPrepare_t)(int16_t w, int16_t h);
typedef void (*pFillScreen_t)(const uint16_t *data, int16_t lines);

//#define BMP_USE_MALLOC		1
//#define BMP_DBUF_SIZE		2048

typedef struct {
    uint32_t biSize;
    long biWidth;
    long biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER;

typedef struct {
    uint16_t  bfType;
    uint32_t  bfSize;
    uint16_t  bfReserved1;
    uint16_t  bfReserved2;
    uint32_t  bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} __attribute__((packed)) RGBQUAD;

typedef struct {
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;  
	uint32_t RGB_MASK[3];
} __attribute__((packed)) BITMAPINFO;

typedef RGBQUAD * LPRGBQUAD; // color table

#define BI_RGB	 		0
#define BI_RLE8 		1
#define BI_RLE4 		2
#define BI_BITFIELDS 	3

// Export functions.
esp_err_t bmp_decode(const char *path, pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen);
//uint8_t minibmp_decode(uint8_t *filename,uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t acolor,uint8_t mode);
//uint8_t bmp_encode(uint8_t *filename,uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __BMPDEC_H */
