#include "string.h"
#include "stdio.h"

#include "imgDecoder.h"

imgDecoder::imgDecoder(const char *file, pDrawBitmap_t pFunc)
{
	path = file;
	pDrawBitmap = pFunc;
}

imgDecoder::~imgDecoder()
{

}

void imgDecoder::decodeBMP()
{
	bmp_decode(path, pDrawBitmap);
}
