#ifndef __SDCARD_H
#define __SDCARD_H

#include "string.h"
#include "stdio.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_partition.h"
#include "freertos/semphr.h"

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

/**
 * @brief struct to map GPIO to sdcard pins
 */
typedef struct {
    int8_t pin_num_miso;         /*!<MasterIn, SlaveOut pin*/
    int8_t pin_num_mosi;         /*!<MasterOut, SlaveIn pin*/
    int8_t pin_num_clk;          /*!<SPI Clock pin*/
    int8_t pin_num_cs;           /*!<SPI Chip Select Pin*/
    int8_t pin_num_cd;           /*!<Pin to card detect Pin*/
    int8_t pin_num_wp;           /*!<Pin to write protect Pin*/
    spi_host_device_t spi_host;  /*!< spi host index*/
    const char* base_path;
} sdcard_conf_t;

#define SDCARD_CONFIG_DEFAULT_VAL      (-1)

#ifdef __cplusplus

class SDCard {
private:
	sdmmc_host_t host;
	sdspi_slot_config_t slot_config;
	esp_vfs_fat_sdmmc_mount_config_t mount_config;
	sdmmc_card_t* card;
	SemaphoreHandle_t sdcard_mux;
public:
	SDCard(sdcard_conf_t *sd_conf);
	virtual ~SDCard();
};

#endif /* __cplusplus */

#endif /* __SDCARD_H */
