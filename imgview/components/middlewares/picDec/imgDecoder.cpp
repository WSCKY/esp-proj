#include "string.h"
#include "stdio.h"

#include "imgDecoder.h"

imgDecoder::imgDecoder(pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen, LcdSize_t size)
{
	LcdSize = size;
	pDrawInit = pDrawPrepare;
	pFillData = pFillScreen;
}

imgDecoder::~imgDecoder()
{

}

esp_err_t imgDecoder::decodeBMP(const char *file)
{
	if(file == NULL) return ESP_ERR_INVALID_ARG;
	path = file;
	return bmp_decode(path, pDrawInit, pFillData);
}

esp_err_t imgDecoder::decodeJPG(const char *file)
{
	if(file == NULL) return ESP_ERR_INVALID_ARG;
	path = file;
	return jpg_decode(path, pDrawInit, pFillData, LcdSize);
}
