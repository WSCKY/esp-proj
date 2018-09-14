/* Hello World Example

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
#include "esp_log.h"
#include "freertos/ringbuf.h"

static const char *TAG = "rb_test";

static char tx_item[] = "kychu test item.";

void app_main(void)
{
	ESP_LOGI(TAG, "start ringbuffer test.");
	//Create ring buffer;
	RingbufHandle_t buf_handle;
	buf_handle = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
	if(buf_handle == NULL) {
		ESP_LOGE(TAG, "Failed to create ring buffer.");
	}

	//Send an item
	UBaseType_t res = xRingbufferSend(buf_handle, tx_item, sizeof(tx_item), pdMS_TO_TICKS(1000));
	if(res != pdTRUE) {
		ESP_LOGE(TAG, "Failed to send item.");
	}

	//Receive on item from no-split ring buffer
	size_t item_size;
	char *item = (char *)xRingbufferReceive(buf_handle, &item_size, pdMS_TO_TICKS(1000));

	//Check received item
	if(item != NULL) {
		//Print item
		for(int i = 0; i < item_size; i ++) {
			printf("%c", item[i]);
		}
		printf("\n");
		//Return Item
		vRingbufferReturnItem(buf_handle, (void *)item);
	} else {
		//Failed to receive item
		ESP_LOGE(TAG, "Failed to receive item.");
	}
}
