#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "sdcard.h"

static const char *TAG = "SDCARD";

SDCard::SDCard(sdcard_conf_t *sd_conf)
{
	sdcard_mux = xSemaphoreCreateRecursiveMutex();
    host = SDSPI_HOST_DEFAULT();
    if(sd_conf->spi_host != SDCARD_CONFIG_DEFAULT_VAL)
    	host.slot         = sd_conf->spi_host;
    slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = sd_conf->pin_num_miso;
    slot_config.gpio_mosi = sd_conf->pin_num_mosi;
    slot_config.gpio_sck  = sd_conf->pin_num_clk;
    slot_config.gpio_cs   = sd_conf->pin_num_cs;
    if(sd_conf->pin_num_cd != SDCARD_CONFIG_DEFAULT_VAL)
    	slot_config.gpio_cd   = sd_conf->pin_num_cd;
    if(sd_conf->pin_num_wp != SDCARD_CONFIG_DEFAULT_VAL)
    	slot_config.gpio_wp   = sd_conf->pin_num_wp;

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
	host.sdspi_host_deinit();
	vSemaphoreDelete(sdcard_mux);
}

SDCard::print_card_info(FILE* stream)
{
	xSemaphoreTakeRecursive(sdcard_mux, portMAX_DELAY);
    bool print_scr = false;
    bool print_csd = false;
    const char* type;
    fprintf(stream, "Name: %s\n", card->cid.name);
    if (card->is_sdio) {
        type = "SDIO";
        print_scr = true;
        print_csd = true;
    } else if (card->is_mmc) {
        type = "MMC";
        print_csd = true;
    } else {
        type = (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC";
    }
    fprintf(stream, "Type: %s\n", type);
    if (card->max_freq_khz < 1000) {
        fprintf(stream, "Speed: %d kHz\n", card->max_freq_khz);
    } else {
        fprintf(stream, "Speed: %d MHz%s\n", card->max_freq_khz / 1000,
                card->is_ddr ? ", DDR" : "");
    }
    fprintf(stream, "Size: %lluMB\n", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));

    if (print_csd) {
        fprintf(stream, "CSD: ver=%d, sector_size=%d, capacity=%d read_bl_len=%d\n",
                card->csd.csd_ver,
                card->csd.sector_size, card->csd.capacity, card->csd.read_block_len);
    }
    if (print_scr) {
        fprintf(stream, "SCR: sd_spec=%d, bus_width=%d\n", card->scr.sd_spec, card->scr.bus_width);
    }
    xSemaphoreGiveRecursive(sdcard_mux);
}
