#ifndef __IMG_DECODER_H
#define __IMG_DECODER_H

#include "string.h"
#include "stdio.h"
#include "esp_system.h"

#include "bmpDec.h"
#include "jpgDec.h"

#define LCD_WIDTH_DEFAULT      128
#define LCD_HEIGHT_DEFAULT     160

typedef enum {
	Img_BMP = 0,
	Img_JPG = 1,
	Img_GIF = 2,

	Img_Unknow = 0xF,
} ImgType_t;

typedef struct {
	const char *suffix;
	ImgType_t type;
} ImgSuffix_t;

#ifdef __cplusplus

class imgDecoder {
private:
	const char *path;
	LcdSize_t LcdSize;
	pDrawPrepare_t pDrawInit;
	pFillScreen_t pFillData;
	bool strcmp(const char *p1, const char *p2);
public:
	imgDecoder(pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen, LcdSize_t size = {LCD_WIDTH_DEFAULT, LCD_HEIGHT_DEFAULT});
	virtual ~imgDecoder();
	ImgType_t checkType(const char *file);
	const char *imgType2String(ImgType_t type);
	esp_err_t decode(const char *file);
	esp_err_t decodeBMP(const char *file);
	esp_err_t decodeJPG(const char *file);
};

#endif /* __cplusplus */

#endif /* __IMG_DECODER_H */
