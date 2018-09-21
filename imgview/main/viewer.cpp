#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "lcd.h"
#include "sdcard.h"
#include "imgDecoder.h"

CMyLcd *lcd = NULL;
SDCard *card = NULL;
imgDecoder *decoder = NULL;

static const char *TAG = "Viewer";

#define SDCARD_PATH      "/sdcard"
#define BMP_PATH         "/bmp"
#define JPG_PATH         "/jpg"

/*

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V

*/

uint16_t tColor = COLOR_WHITE;
uint16_t bColor = COLOR_WHITE;

static uint8_t x_cnt = 0, y_cnt = 0;

void setBold(int c)
{
	if(c == '1') {
		//bold font.
	} else {
		//normal font.
	}
}

uint16_t getColor(int code)
{
	switch(code) {
		case 0:  return COLOR_WHITE;
		case 30: return COLOR_BLACK;
		case 31: return COLOR_RED;
		case 32: return COLOR_GREEN;
		case 33: return COLOR_YELLOW;
		case 34: return COLOR_BLUE;
		case 35: return COLOR_PURPLE;
		case 36: return COLOR_CYAN;
		default: return COLOR_WHITE;
	}
}

int getCode(const char *p)
{
	int ret = 0;
	uint8_t vals = 0;
	uint8_t param[5] = {0}; // can not more than 5.
	while(vals < 5) {
		if(*p == ';') {
			ret ++; p ++; vals ++; continue;
		}
		else if(*p == 'm') {
			ret ++; p ++; vals ++; break;
		}
		else if((*p < '0') || (*p - '0' > 9)) { // unsupport command.
			vals = 0; ret = 0; break;
		}

		param[vals] *= 10;
		param[vals] += (*p - '0');

		ret ++; p ++;
	}
	if(vals > 0) {
		if(vals > 1) {
			setBold(param[0]);
		}
		tColor = getColor(param[vals - 1]);
		bColor = tColor;
		lcd->setTextColor(tColor);
	} else {
		setBold(0);
		tColor = COLOR_WHITE;
		bColor = tColor;
		lcd->setTextColor(tColor);
	}
	return ret;
}

int parseTerminal(const char *p)
{
	int ret = 0;
redo:
	if(*p == '\e') {
		p ++; ret ++;
		if(*p == '[') {
			p ++; ret ++;
			int tmp = getCode(p);
			ret += tmp;
			p += tmp;
		}
	} else if(*p == '\n') {
		p ++; ret ++;
		x_cnt = 0;
		y_cnt += 8;
		if(y_cnt + 8 > lcd->height()) {
			y_cnt = 0;
		}
	} else {
		return ret;
	}
goto redo;
}

void lcd_output(const char *pString)
{
	while(*pString != 0) {
		pString += parseTerminal(pString);
		if(*pString != 0) {
			if((x_cnt + 6) > lcd->width()) {
				x_cnt = 0;
				y_cnt += 8;
				if(y_cnt + 8 > lcd->height()) {
					y_cnt = 0;
				}
			}
			lcd->drawChar(x_cnt, y_cnt, *pString, tColor, bColor, 1);
			x_cnt += 6;
			pString ++;
		}
	}
}
#define LOG_CACHE_SIZE     80
int lcd_log_vprintf(const char *fmt, va_list ap)
{
	int ret = 0;
	char buf[LOG_CACHE_SIZE];
	ret = vsprintf(buf, fmt, ap);
	if(ret >= LOG_CACHE_SIZE) buf[LOG_CACHE_SIZE - 1] = 0;
	else buf[ret] = 0;
	lcd_output(buf);
	return ret;
}

esp_err_t setDrawAddr(ImgArea_t *pRect)
{
	lcd->setRotation(2);
	if((pRect->right + 1) > lcd->width()) return ESP_FAIL;
	if((pRect->bottom + 1) > lcd->height()) return ESP_FAIL;
	lcd->setAddrWindow(pRect->left, pRect->top, pRect->right, pRect->bottom);
	return ESP_OK;
}

void fillData(const uint16_t *data, uint16_t size, bool swap)
{
	lcd->fillDataFast(data, size, swap);
}

char *fullname = NULL;
char *getname(const char *a, const char *b)
{
	int i = 0;
	if(fullname == NULL)
		fullname = (char *)calloc(80, sizeof(char));
	while(*a != 0)
		fullname[i ++] = *a ++;
	while(*b != 0)
		fullname[i ++] = *b ++;
	fullname[i] = 0;
	return fullname;
}

bool checkBMP(const char *name)
{
	while(*name != '.' && *name != '\0')
		name ++;
	if(*name == '.') {
		name ++;
		if((*name == 'B' && *(name + 1) == 'M' && *(name + 2) == 'P') ||
		   (*name == 'b' && *(name + 1) == 'm' && *(name + 2) == 'p'))
			return true;
	}
	return false;
}

extern "C" void app_main()
{
  LcdSize_t lcd_size = {128, 160};
  printf("Image Viewer.\n");
  lcd_conf_t lcd_pins = {
	  .pin_num_miso = GPIO_NUM_25,
	  .pin_num_mosi = GPIO_NUM_18,
	  .pin_num_clk = GPIO_NUM_19,
	  .pin_num_cs = GPIO_NUM_22,
	  .pin_num_dc = GPIO_NUM_4,
	  .pin_num_rst = GPIO_NUM_5,
	  .pin_num_bckl = GPIO_NUM_2,
	  .clk_freq = 26 * 1000 * 1000,
	  .rst_active_level = 0,
	  .bckl_active_level = 0,
	  .spi_host = VSPI_HOST,
	  .init_spi_bus = true,
  };

  sdcard_conf_t sd_conf = {
	.pin_num_miso = GPIO_NUM_12,
    .pin_num_mosi = GPIO_NUM_15,
    .pin_num_clk = GPIO_NUM_14,
    .pin_num_cs = GPIO_NUM_13,
    .pin_num_cd = SDCARD_CONFIG_DEFAULT_VAL,
    .pin_num_wp = SDCARD_CONFIG_DEFAULT_VAL,
    .spi_host = HSPI_HOST,
	.dma_channel = 2,
    .base_path = SDCARD_PATH,
  };

  /*Initialize SPI Handler*/
  if (lcd == NULL) {
	  lcd = new CMyLcd(&lcd_pins);
  }

  if(card == NULL) {
	  card = new SDCard(&sd_conf);
  }

  /*screen initialize*/
  lcd->setRotation(3);             //Landscape mode
  lcd->fillScreen(lcd->color565(0x80, 0x80, 0x80));
  esp_log_set_vprintf(lcd_log_vprintf);
  ESP_LOGI(TAG, "Image Viewer Start.");

  esp_log_set_vprintf(vprintf);
  ESP_LOGI(TAG, "Image Viewer Start.");

  DIR *dir;
  struct dirent *dc;
  esp_err_t ret = ESP_OK;

  if(decoder == NULL) {
    decoder = new imgDecoder(setDrawAddr, fillData, lcd_size);
  }
  while(1) {
  	  dir = opendir(SDCARD_PATH JPG_PATH);
  	  while((dc = readdir(dir)) != NULL) {
  		  if(dc->d_type == 1) {// && checkJPG(dc->d_name)
  			  lcd->fillScreen(lcd->color565(0x80, 0x80, 0x80));
  			  ret = decoder->decodeJPG((const char *)getname(SDCARD_PATH JPG_PATH "/", dc->d_name));
  			  lcd->setRotation(2);
  			  lcd->setTextColor(COLOR_WHITE);
  			  lcd->drawString(dc->d_name, 0, 0);
  			  if(ret != ESP_OK) {
  				  lcd->setTextColor(COLOR_RED);
  				  lcd->drawString("Image decode failed!", 4, 76);
  			  }
  			  vTaskDelay(2000 / portTICK_RATE_MS);
  		  }
  	  }
  	  closedir(dir);
  	  vTaskDelay(2000 / portTICK_RATE_MS);
    }
//  while(1) {
//	  dir = opendir(SDCARD_PATH BMP_PATH);
//	  while((dc = readdir(dir)) != NULL) {
//		  if(dc->d_type == 1 && checkBMP(dc->d_name)) {
//			  lcd->fillScreen(lcd->color565(0x80, 0x80, 0x80));
//			  ret = decoder->decodeBMP((const char *)getname(SDCARD_PATH BMP_PATH "/", dc->d_name));
//			  lcd->setRotation(2);
//			  lcd->setTextColor(COLOR_WHITE);
//			  lcd->drawString(dc->d_name, 0, 0);
//			  if(ret != ESP_OK) {
//				  lcd->setTextColor(COLOR_RED);
//				  lcd->drawString("Image decode failed!", 4, 76);
//			  }
//			  vTaskDelay(2000 / portTICK_RATE_MS);
//		  }
//	  }
//	  closedir(dir);
//	  vTaskDelay(2000 / portTICK_RATE_MS);
//  }
}
