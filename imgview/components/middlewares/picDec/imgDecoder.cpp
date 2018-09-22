#include "string.h"
#include "stdio.h"
#include "esp_log.h"

#include "imgDecoder.h"

ImgSuffix_t ImgTypes[] = {
		{".bmp", Img_BMP}, {".BMP", Img_BMP},
		{".jpg", Img_JPG}, {".JPG", Img_JPG},
		{".jpeg", Img_JPG}, {".JPEG", Img_JPG},
		{".gif", Img_GIF}, {".GIF", Img_GIF},
		{"Unknow", Img_Unknow},
};

imgDecoder::imgDecoder(pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen, LcdSize_t size)
{
	LcdSize = size;
	pDrawInit = pDrawPrepare;
	pFillData = pFillScreen;
}

imgDecoder::~imgDecoder()
{

}

bool imgDecoder::strcmp(const char *p1, const char *p2)
{
	while((*p1 != 0) && (*p2 != 0)) {
		if(*p1 != *p2) return false;
		p1 ++; p2 ++;
	}
	if((*p1 != 0) || (*p2 != 0)) return false;
	return true;
}

ImgType_t imgDecoder::checkType(const char *file)
{
	char *pointer = (char *)file + strlen(file);
	int len = 5;
	while(len --) {
		if(*(-- pointer) == '.') break;
	}
	if(*pointer == '.') {
		len = 0;
		while(ImgTypes[len].type != Img_Unknow) {
			if(strcmp(ImgTypes[len].suffix, pointer)) {
				return ImgTypes[len].type;
			}
			len ++;
		}
		return Img_Unknow;
	} else {
		return Img_Unknow;
	}
}

const char *imgDecoder::imgType2String(ImgType_t type)
{
	int i = 0;
	while(ImgTypes[i].type != type && ImgTypes[i].type != Img_Unknow) {
		i ++;
	}
	return ImgTypes[i].suffix;
}

esp_err_t imgDecoder::decodeBMP(const char *file)
{
	if(file == NULL) return ESP_ERR_INVALID_ARG;
	if(checkType(file) != Img_BMP) return ESP_ERR_INVALID_ARG;
	path = file;
	return bmp_decode(path, pDrawInit, pFillData);
}

esp_err_t imgDecoder::decodeJPG(const char *file)
{
	if(file == NULL) return ESP_ERR_INVALID_ARG;
	if(checkType(file) != Img_JPG) return ESP_ERR_INVALID_ARG;
	path = file;
	return jpg_decode(path, pDrawInit, pFillData, LcdSize);
}
