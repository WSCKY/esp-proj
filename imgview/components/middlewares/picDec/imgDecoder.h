#ifndef __IMG_DECODER_H
#define __IMG_DECODER_H

#include "string.h"
#include "stdio.h"
#include "esp_system.h"

#include "bmpDec.h"
#include "jpgDec.h"

#ifdef __cplusplus

class imgDecoder {
private:
	const char *path;
	pDrawPrepare_t pDrawInit;
	pFillScreen_t pFillData;
public:
	imgDecoder(const char *file, pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen);
	virtual ~imgDecoder();
	esp_err_t decodeBMP(const char *file);
	esp_err_t decodeJPG(const char *file);
};

#endif /* __cplusplus */

#endif /* __IMG_DECODER_H */
