#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "sdcard.h"

static const char *TAG = "SDCARD";

SDCard::SDCard(sdcard_conf_t *sd_conf)
{
	sdcard_mux = xSemaphoreCreateRecursiveMutex();
    host = SDSPI_HOST_DEFAULT();
    if(sd_conf->spi_host != SDCARD_CONFIG_DEFAULT_VAL)
    	host.slot         = sd_conf->spi_host;
    slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = (gpio_num_t)sd_conf->pin_num_miso;
    slot_config.gpio_mosi = (gpio_num_t)sd_conf->pin_num_mosi;
    slot_config.gpio_sck  = (gpio_num_t)sd_conf->pin_num_clk;
    slot_config.gpio_cs   = (gpio_num_t)sd_conf->pin_num_cs;
    if(sd_conf->pin_num_cd != SDCARD_CONFIG_DEFAULT_VAL)
    	slot_config.gpio_cd   = (gpio_num_t)sd_conf->pin_num_cd;
    if(sd_conf->pin_num_wp != SDCARD_CONFIG_DEFAULT_VAL)
    	slot_config.gpio_wp   = (gpio_num_t)sd_conf->pin_num_wp;
    if(sd_conf->dma_channel >= 1 && sd_conf->dma_channel <= 2)
    	slot_config.dma_channel = sd_conf->dma_channel;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
	// Please check its source code and implement error recovery when developing
	// production applications.
	esp_err_t ret = esp_vfs_fat_sdmmc_mount(sd_conf->base_path, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem. "
				"If you want the card to be formatted, set format_if_mount_failed = true.");
		} else {
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
				"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return;
	}
}

SDCard::~SDCard()
{
	// unmount partition and disable SDMMC or SPI peripheral
	esp_vfs_fat_sdmmc_unmount();
	host.deinit();
	vSemaphoreDelete(sdcard_mux);
}
