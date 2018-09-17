#ifndef __SDCARD_H
#define __SDCARD_H

#include "string.h"
#include "stdio.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_partition.h"
#include "freertos/semphr.h"

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

class SDCard {
private:
	sdmmc_host_t host = NULL;
	sdspi_slot_config_t slot_config = NULL;
	esp_vfs_fat_sdmmc_mount_config_t mount_config = NULL;
	sdmmc_card_t* card;
	SemaphoreHandle_t sdcard_mux;
public:
	SDCard(sdcard_conf_t *sd_conf);
	virtual ~SDCard();
	void print_card_info(FILE* stream);
}

#endif /* __SDCARD_H */
