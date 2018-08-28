/* kyChu test project

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define LED_GPIO_PIN     CONFIG_USR_LED_GPIO

static const char *TAG = "self-proj";

void app_main()
{
    printf("Hi, kyChu!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    gpio_pad_select_gpio(LED_GPIO_PIN);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);

	ESP_LOGE(TAG, "This is Error Information");
	ESP_LOGW(TAG, "This is Warning Information");
	ESP_LOGI(TAG, "Normal Information");
	ESP_LOGD(TAG, "Debug Information"); /* use 'make menuconfig' to show it. */
	ESP_LOGV(TAG, "Verbose Information"); /* use 'make menuconfig' to show it. */

	printf("flash led ...\n");
	while(1) {
		gpio_set_level(LED_GPIO_PIN, 0);
		vTaskDelay(200 / portTICK_PERIOD_MS);
		gpio_set_level(LED_GPIO_PIN, 1);
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}

/* Test code -2 */
//    int count = 0;
//    while(1) {
//    	printf("count %d seconds ...\n", count ++);
//    	vTaskDelay(1000 / portTICK_PERIOD_MS);
//    	if(count >= 60)
//    		break;
//    }
//    printf("Exit, Entry Deep sleep ...\n");
//    fflush(stdout);
//
//    esp_deep_sleep_start();
//    printf("test output...\n");
//
//    while(1) {}

/* Test code -1 */
//    for (int i = 10; i >= 0; i--) {
//        printf("Restarting in %d seconds...\n", i);
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
//    }
//    printf("Restarting now.\n");
//    fflush(stdout);
//    esp_restart();
}
