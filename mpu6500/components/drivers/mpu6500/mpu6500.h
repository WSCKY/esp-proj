#ifndef __MPU6500_H
#define __MPU6500_H

#include "string.h"
#include "stdio.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_partition.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "mpu6500_reg.h"

/**
 * @brief struct to map GPIO to mpu6500 pins
 */
typedef struct {
    int8_t pin_num_miso;         /*!<MasterIn, SlaveOut pin*/
    int8_t pin_num_mosi;         /*!<MasterOut, SlaveIn pin*/
    int8_t pin_num_clk;          /*!<SPI Clock pin*/
    int8_t pin_num_cs;           /*!<SPI Chip Select Pin*/
    int8_t pin_num_int;          /*!<IMU data interrupt Pin*/
    spi_host_device_t spi_host;  /*!< spi host index*/
    int dma_channel;             /*!< DMA channel to be used by SPI driver (1 or 2)*/
    int clk_freq;                /*!< spi clock frequency */
} mpu6500_conf_t;

typedef struct {
	int16_t x, y, z;
} imu_raw_t;

typedef struct {
	float x, y, z;
} imu_unit_t;

typedef imu_raw_t acc_raw_t;
typedef int16_t tmp_raw_t;
typedef imu_raw_t gyr_raw_t;

typedef imu_unit_t acc_unit_t;
typedef float tmp_unit_t;
typedef imu_unit_t gyr_unit_t;

typedef struct {
	acc_raw_t acc;
	tmp_raw_t tmp;
	gyr_raw_t gyr;
} mpu6500_raw_t;

typedef struct {
	acc_unit_t acc;
	tmp_unit_t tmp;
	gyr_unit_t gyr;
} mpu6500_unit_t;

#ifdef __cplusplus

class MPU6500 {
private:
	spi_device_handle_t spi_wr = NULL;
	uint8_t *_tx_buf = NULL, *_rx_buf = NULL;
	gyr_fs_t _gyr_fs = gyr_fs_2000dps;
	acc_fs_t _acc_fs = acc_fs_8g;
	float gyr_fs_fact = 0.0f, acc_fs_fact = 0.0f;
	mpu6500_raw_t *raw_data = NULL;
	mpu6500_unit_t *unit_data = NULL;
	SemaphoreHandle_t mpu6500_mux;
	void _update_acc_factor();
	void _update_gyr_factor();
	void _update_factor();
	esp_err_t _wr_reg(uint8_t reg, uint8_t val);
	esp_err_t _rd_reg(uint8_t reg, uint8_t num);
public:
	MPU6500(mpu6500_conf_t *mpu_config);
	esp_err_t update();
	mpu6500_raw_t get_raw();
	mpu6500_unit_t get_unit();
	esp_err_t set_acc_scale(acc_fs_t scale);
	esp_err_t set_gyr_scale(gyr_fs_t scale);
	esp_err_t set_gyr_offset(gyr_raw_t *off);
	esp_err_t get_id(uint8_t *id);
	virtual ~MPU6500();
};

#endif /* __cplusplus */

#endif /* __MPU6500_H */
