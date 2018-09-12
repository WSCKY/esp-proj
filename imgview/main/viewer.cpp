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
//  lcd->invertDisplay(false);
//  lcd->setRotation(1);             //Landscape mode
  lcd->fillScreen(COLOR_ESP_BKGD);
  lcd->drawString("Hello World!", 0, 0);
  lcd->fillRect(0, 10, 60, 40, COLOR_BLUE);
  lcd->fillRect(64, 10, 60, 40, COLOR_GREEN);
  lcd->fillRect(0, 50, 60, 40, COLOR_RED);
//  lcd->drawBitmap(0, 0, esp_logo, 137, 26);
//  lcd->drawBitmap(243, 0, aws_logo, 77, 31);

  lcd->setRotation(0);
  lcd->drawString("Rotation-0", 0, 10);
  lcd->setRotation(1);
  lcd->drawString("Rotation-1", 0, 10);
  lcd->setRotation(2);
  lcd->drawString("Rotation-2", 0, 10);
  lcd->setRotation(3);
  lcd->drawString("Rotation-3", 0, 10);
  lcd->setRotation(4);
  lcd->drawString("Rotation-4", 0, 10);
  lcd->setRotation(5);
  lcd->drawString("Rotation-5", 0, 10);
  lcd->setRotation(6);
  lcd->drawString("Rotation-6", 0, 10);
}
