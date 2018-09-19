#include "string.h"
#include "stdio.h"

#include "imgDecoder.h"

imgDecoder::imgDecoder(const char *file, pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen)
{
	path = file;
	pDrawInit = pDrawPrepare;
	pFillData = pFillScreen;
}

imgDecoder::~imgDecoder()
{

}

esp_err_t imgDecoder::decodeBMP(const char *file)
{
	if(file != NULL) path = file;
	return bmp_decode(path, pDrawInit, pFillData);
}
