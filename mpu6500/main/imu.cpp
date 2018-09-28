#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "mpu6500.h"

MPU6500 *imu = NULL;

static const char *TAG = "IMU_TEST";

extern "C" void app_main()
{
  mpu6500_conf_t imu_conf = {
	.pin_num_miso = GPIO_NUM_12,
	.pin_num_mosi = GPIO_NUM_15,
	.pin_num_clk = GPIO_NUM_14,
	.pin_num_cs = GPIO_NUM_13,
	.pin_num_int = (-1),
	.spi_host = HSPI_HOST,
	.dma_channel = 1,
	.clk_freq = 8 * 1000 * 1000,
  };

  /*Initialize SPI Handler*/
  if(imu == NULL) {
	  imu = new MPU6500(&imu_conf);
  }
  uint8_t imu_id = 0;
  if(imu->get_id(&imu_id) != ESP_OK) {
	  ESP_LOGE(TAG, "Failed to get ID.");
	  while(1) {
		  vTaskDelay(200 / portTICK_RATE_MS);
	  }
  }
  ESP_LOGI(TAG, "IMU ID is 0x%x.", imu_id);

  while(1) {
	  imu->update();
	  mpu6500_unit_t data = imu->get_unit();
	  printf("\033[0;34m[ax:%.2f, ay:%.2f, az:%.2f, tmp:%.2f, gx:%.2f, gy:%.2f, gz:%.2f]\r\033[0m"
			  , data.acc.x, data.acc.y, data.acc.z, data.tmp, data.gyr.x, data.gyr.y, data.gyr.z);
	  vTaskDelay(200 / portTICK_RATE_MS);
  }
}
