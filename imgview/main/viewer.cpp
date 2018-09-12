#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "lcd.h"

CMyLcd *lcd = NULL;

extern "C" void app_main()
{
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
	  .spi_host = HSPI_HOST,
	  .init_spi_bus = true,
  };

  /*Initialize SPI Handler*/
  if (lcd == NULL) {
	  lcd = new CMyLcd(&lcd_pins);
  }

  /*screen initialize*/
  lcd->invertDisplay(false);
  lcd->setRotation(1);             //Landscape mode
  lcd->fillScreen(COLOR_ESP_BKGD);
//  lcd->drawBitmap(0, 0, esp_logo, 137, 26);
//  lcd->drawBitmap(243, 0, aws_logo, 77, 31);
}
