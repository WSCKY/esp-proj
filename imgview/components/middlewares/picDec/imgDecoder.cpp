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

void imgDecoder::decodeBMP(const char *file)
{
	if(file != NULL) path = file;
	bmp_decode(path, pDrawInit, pFillData);
}
