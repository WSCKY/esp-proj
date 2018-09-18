#ifndef __IMG_DECODER_H
#define __IMG_DECODER_H

#include "string.h"
#include "stdio.h"
#include "esp_system.h"

#include "bmpDec.h"

#ifdef __cplusplus

class imgDecoder {
private:
	const char *path;
	pDrawBitmap_t pDrawBitmap;
public:
	imgDecoder(const char *file, pDrawBitmap_t pFunc);
	virtual ~imgDecoder();
	void decodeBMP();
};

#endif /* __cplusplus */

#endif /* __IMG_DECODER_H */
