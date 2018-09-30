#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "mpu6500.h"

static const char *TAG = "MPU6500";
static const float Gravity = 9.8f;

#define MPU_CHECK(a, str, ret_val) \
    if (!(a)) { \
        ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val); \
    }

MPU6500::MPU6500(mpu6500_conf_t *mpu_conf)
{
	mpu6500_mux = xSemaphoreCreateRecursiveMutex();
	//Check whether it is a valid GPIO number
	if(GPIO_IS_VALID_GPIO(mpu_conf->pin_num_int)) {
		//Initialize non-SPI GPIOs
		gpio_pad_select_gpio((gpio_num_t)mpu_conf->pin_num_int);
		gpio_set_direction((gpio_num_t)mpu_conf->pin_num_int, GPIO_MODE_INPUT_OUTPUT);
	}
	//Initialize SPI Bus for MPU6500
	spi_bus_config_t buscfg;
	memset(&buscfg, 0, sizeof(buscfg));
	buscfg.miso_io_num = (gpio_num_t)mpu_conf->pin_num_miso,
	buscfg.mosi_io_num = (gpio_num_t)mpu_conf->pin_num_mosi,
	buscfg.sclk_io_num = (gpio_num_t)mpu_conf->pin_num_clk,
	buscfg.quadwp_io_num = -1,
	buscfg.quadhd_io_num = -1,

	spi_bus_initialize(mpu_conf->spi_host, &buscfg, mpu_conf->dma_channel);
	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));
	devcfg.clock_speed_hz = mpu_conf->clk_freq,     //Clock out frequency
	devcfg.mode = 3,                                //SPI mode 3
	devcfg.spics_io_num = (gpio_num_t)mpu_conf->pin_num_cs,     //CS pin
	devcfg.queue_size = 7,                          //We want to be able to queue 7 transactions at a time
	devcfg.pre_cb = NULL,                           //Specify pre-transfer callback

	spi_bus_add_device(mpu_conf->spi_host, &devcfg, &spi_wr);

	_tx_buf = (uint8_t *)malloc(15);
	assert(_tx_buf != NULL);
	_rx_buf = (uint8_t *)malloc(15);
	assert(_rx_buf != NULL);
	memset(_tx_buf, 0, 15);
	memset(_rx_buf, 0, 15);

	_wr_reg(MPU_ERG_PWRMGT, 0x80);
	vTaskDelay(10 / portTICK_RATE_MS);
	_wr_reg(MPU_REG_SGNRST, 0x07);
	_wr_reg(MPU_ERG_PWRMGT, 0x00);
	vTaskDelay(10 / portTICK_RATE_MS);
	_wr_reg(MPU_REG_SMPDIV, 0x00);
	_wr_reg(MPU_REG_CONFIG, 0x03);
	_wr_reg(MPU_REG_ACFG_2, 0x06);
	_wr_reg(MPU_REG_ACCCFG, 0x10);
	_wr_reg(MPU_REG_GYRCFG, 0x18);
	vTaskDelay(10 / portTICK_RATE_MS);
	_acc_fs = acc_fs_8g;
	_gyr_fs = gyr_fs_2000dps;
	_update_factor();
	raw_data = (mpu6500_raw_t *)malloc(sizeof(mpu6500_raw_t));
	assert(raw_data != NULL);
	unit_data = (mpu6500_unit_t *)malloc(sizeof(mpu6500_unit_t));
	assert(unit_data != NULL);
}

esp_err_t MPU6500::_wr_reg(uint8_t reg, uint8_t val)
{
	xSemaphoreTakeRecursive(mpu6500_mux, portMAX_DELAY);
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = 8 * 2,                 // data length in bits
	t.tx_data[0] = reg,               // The first byte is reg
	t.tx_data[1] = val,               // The second byte is val
	t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
	esp_err_t ret = spi_device_transmit(spi_wr, &t); //Transmit!
	xSemaphoreGiveRecursive(mpu6500_mux);
	return ret;
}

esp_err_t MPU6500::_rd_reg(uint8_t reg, uint8_t num)
{
	MPU_CHECK(num <= 14, "Invalid param", ESP_ERR_INVALID_ARG);
	xSemaphoreTakeRecursive(mpu6500_mux, portMAX_DELAY);
	_tx_buf[0] = reg | 0x80;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = (num + 1) * 8,
	t.rxlength = t.length;
	t.tx_buffer = _tx_buf;
	t.rx_buffer = _rx_buf;
	esp_err_t ret = spi_device_transmit(spi_wr, &t);
	xSemaphoreGiveRecursive(mpu6500_mux);
	return ret;
}

void MPU6500::_update_acc_factor()
{
	switch(_acc_fs) {
	case acc_fs_2g:
		acc_fs_fact = (4.0f * Gravity) / 65536.0f;
		break;
	case acc_fs_4g:
		acc_fs_fact = (8.0f * Gravity) / 65536.0f;
		break;
	case acc_fs_8g:
		acc_fs_fact = (16.0f * Gravity) / 65536.0f;
		break;
	case acc_fs_16g:
		acc_fs_fact = (32.0f * Gravity) / 65536.0f;
		break;
	default:
		acc_fs_fact = (16.0f * Gravity) / 65536.0f;
		break;
	}
}

void MPU6500::_update_gyr_factor()
{
	switch(_gyr_fs) {
	case gyr_fs_250dps:
		gyr_fs_fact = 500.0f / 65536.0f;
		break;
	case gyr_fs_500dps:
		gyr_fs_fact = 1000.0f / 65536.0f;
		break;
	case gyr_fs_1000dps:
		gyr_fs_fact = 2000.0f / 65536.0f;
		break;
	case gyr_fs_2000dps:
		gyr_fs_fact = 4000.0f / 65536.0f;
		break;
	default:
		gyr_fs_fact = 4000.0f / 65536.0f;
		break;
	}
}

void MPU6500::_update_factor()
{
	_update_acc_factor();
	_update_gyr_factor();
}

esp_err_t MPU6500::update()
{
	esp_err_t ret = _rd_reg(0x3B, 14);
	MPU_CHECK(ret == ESP_OK, "read failed", ret);
	((int8_t *)&(raw_data->acc.x))[0] = _rx_buf[2];
	((int8_t *)&(raw_data->acc.x))[1] = _rx_buf[1];

	((int8_t *)&(raw_data->acc.y))[0] = _rx_buf[4];
	((int8_t *)&(raw_data->acc.y))[1] = _rx_buf[3];

	((int8_t *)&(raw_data->acc.z))[0] = _rx_buf[6];
	((int8_t *)&(raw_data->acc.z))[1] = _rx_buf[5];

	((int8_t *)&(raw_data->tmp))[0] = _rx_buf[8];
	((int8_t *)&(raw_data->tmp))[1] = _rx_buf[7];

	((int8_t *)&(raw_data->gyr.x))[0] = _rx_buf[10];
	((int8_t *)&(raw_data->gyr.x))[1] = _rx_buf[9];

	((int8_t *)&(raw_data->gyr.y))[0] = _rx_buf[12];
	((int8_t *)&(raw_data->gyr.y))[1] = _rx_buf[11];

	((int8_t *)&(raw_data->gyr.z))[0] = _rx_buf[14];
	((int8_t *)&(raw_data->gyr.z))[1] = _rx_buf[13];

	unit_data->acc.x = raw_data->acc.x * acc_fs_fact;
	unit_data->acc.y = raw_data->acc.y * acc_fs_fact;
	unit_data->acc.z = raw_data->acc.z * acc_fs_fact;
	unit_data->tmp = (raw_data->tmp / 333.87f) + 21.0f;
	unit_data->gyr.x = raw_data->gyr.x * gyr_fs_fact;
	unit_data->gyr.y = raw_data->gyr.y * gyr_fs_fact;
	unit_data->gyr.z = raw_data->gyr.z * gyr_fs_fact;
	return ret;
}

mpu6500_raw_t MPU6500::get_raw()
{
	return *raw_data;
}

mpu6500_unit_t MPU6500::get_unit()
{
	return *unit_data;
}

esp_err_t MPU6500::set_acc_scale(acc_fs_t scale)
{
	MPU_CHECK(scale <= acc_fs_16g, "Invalid param", ESP_ERR_INVALID_ARG);
	esp_err_t ret = _rd_reg(MPU_REG_ACCCFG, 1);
	if(ret == ESP_OK) {
		uint8_t tmp = _rx_buf[1];
		tmp &= ACC_FS_SEL_Msk;
		tmp |= (scale << ACC_FS_SEL_Pos);
		ret = _wr_reg(MPU_REG_ACCCFG, tmp);
		if(ret == ESP_OK) {
			_acc_fs = scale;
			_update_acc_factor();
		}
	}
	return ret;
}

esp_err_t MPU6500::set_gyr_scale(gyr_fs_t scale)
{
	MPU_CHECK(scale <= gyr_fs_2000dps, "Invalid param", ESP_ERR_INVALID_ARG);
	esp_err_t ret = _rd_reg(MPU_REG_GYRCFG, 1);
	if(ret == ESP_OK) {
		uint8_t tmp = _rx_buf[1];
		tmp &= ~GYR_FS_SEL_Msk;
		tmp |= (scale << GYR_FS_SEL_Pos);
		ret = _wr_reg(MPU_REG_GYRCFG, tmp);
		if(ret == ESP_OK) {
			_gyr_fs = scale;
			_update_gyr_factor();
		}
	}
	return ret;
}

esp_err_t MPU6500::set_gyr_offset(gyr_raw_t *off)
{
	MPU_CHECK(_wr_reg(MPU_REG_GYR_X_OFFS_H, (off->x) >> 8) == ESP_OK, "set offset failed", ESP_FAIL);
	MPU_CHECK(_wr_reg(MPU_REG_GYR_X_OFFS_L, (off->x) & 0xFF) == ESP_OK, "set offset failed", ESP_FAIL);
	MPU_CHECK(_wr_reg(MPU_REG_GYR_Y_OFFS_H, (off->y) >> 8) == ESP_OK, "set offset failed", ESP_FAIL);
	MPU_CHECK(_wr_reg(MPU_REG_GYR_Y_OFFS_L, (off->y) & 0xFF) == ESP_OK, "set offset failed", ESP_FAIL);
	MPU_CHECK(_wr_reg(MPU_REG_GYR_Z_OFFS_H, (off->z) >> 8) == ESP_OK, "set offset failed", ESP_FAIL);
	MPU_CHECK(_wr_reg(MPU_REG_GYR_Z_OFFS_L, (off->z) & 0xFF) == ESP_OK, "set offset failed", ESP_FAIL);
	return ESP_OK;
}

esp_err_t MPU6500::set_gyr_dlpf(gyr_dlpf_t dlpf)
{
	esp_err_t ret = ESP_OK;
	ret = _rd_reg(MPU_REG_GYRCFG, 1);
	if(ret == ESP_OK) {
		uint8_t tmp = _rx_buf[1];
		tmp &= ~DLPF_BYPASS_Msk;
		if(dlpf < GYR_DLPF_BYP_Pos) {
			// Use DLPF.
			tmp |= DLPF_BYPASS_Disable;
			ret = _wr_reg(MPU_REG_GYRCFG, tmp);
			if(ret == ESP_OK) {
				ret = _rd_reg(MPU_REG_CONFIG, 1);
				if(ret == ESP_OK) {
					tmp = _rx_buf[1];
					tmp &= ~DLPF_CONFIG_Msk;
					tmp |= dlpf;
					ret = _wr_reg(MPU_REG_CONFIG, tmp);
				}
			}
		} else {
			tmp |= (dlpf - GYR_DLPF_BYP_Pos + 1);
			ret = _wr_reg(MPU_REG_GYRCFG, tmp);
		}
	}
	return ret;
}

esp_err_t MPU6500::set_acc_dlpf(acc_dlpf_t dlpf)
{
	esp_err_t ret = ESP_OK;
	if(dlpf == acc_dlpf_byp_1130hz) {
		ret = _wr_reg(MPU_REG_ACFG_2, ACC_DLPF_BYP_Msk << ACC_DLPF_BYP_Pos);
	} else {
		ret = _wr_reg(MPU_REG_ACFG_2, dlpf << ACC_DLPF_CFG_Pos);
	}
	return ret;
}

esp_err_t MPU6500::get_id(uint8_t *id)
{
	esp_err_t ret = _rd_reg(MPU_REG_WHOAMI, 1);
	if(ret == ESP_OK)
		*id = _rx_buf[1];
	return ret;
}

MPU6500::~MPU6500()
{
	free(_tx_buf);
	free(_rx_buf);
	free(raw_data);
	free(unit_data);
	spi_bus_remove_device(spi_wr);
	vSemaphoreDelete(mpu6500_mux);
}
