// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <sys/param.h>
#include "st7735s.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/xtensa_api.h"
#include "freertos/task.h"

#define SPIFIFOSIZE 16

static const char *TAG = "ST7735S";

/*
 This struct stores a bunch of command values to be initialized for ILI9341
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static const lcd_init_cmd_t lcd_init_cmds[]={
	{0x01, {0x01}, 0x80},
	{0x11, {0x11}, 0x80},
	{0x20, {0x20}, 0x80},
	{0x38, {0x38}, 0x80},
	{0x13, {0x13}, 0x80},
	{0x3a, {0x05}, 1},
	{0xc0, {0x03}, 1},
	{0xc5, {0xc8}, 1},
	{0xc6, {0x1f}, 1},
	{0xfe, {0x00, 0x04}, 2},
	{0x26, {0x04}, 1},
	{0x2a, {0x00, 0x00, 0x00, 0x7f}, 4},
	{0x2b, {0x00, 0x00, 0x00, 0x9f}, 4},
	{0x36, {0xc0}, 1},
	{0x29, {0x00}, 0x80},
    {0, {0}, 0xff}
};

#define LCD_CMD_LEV   (0)
#define LCD_DATA_LEV  (1)

/*This function is called (in irq context!) just before a transmission starts.
It will set the D/C line to the value indicated in the user field */
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    lcd_dc_t *dc = (lcd_dc_t *) t->user;
    gpio_set_level((int)dc->dc_io, (int)dc->dc_level);
}

static esp_err_t _lcd_spi_send(spi_device_handle_t spi, spi_transaction_t* t)
{
    return spi_device_transmit(spi, t); //Transmit!
}

void lcd_cmd(spi_device_handle_t spi, uint16_t cmd, lcd_dc_t *dc)
{
    esp_err_t ret;
    dc->dc_level = LCD_CMD_LEV;
    spi_transaction_t t = {
        .length = 8,                    // Command is 8 bits
        .tx_buffer = &cmd,              // The data is the cmd itself
        .user = (void *) dc,            // D/C needs to be set to 0
    };
    ret = _lcd_spi_send(spi, &t);       // Transmit!
    assert(ret == ESP_OK);              // Should have had no issues.
}

void lcd_data(spi_device_handle_t spi, const void *data, int len, lcd_dc_t *dc)
{
    esp_err_t ret;
    if (len == 0) {
        return;    //no need to send anything
    }
    dc->dc_level = LCD_DATA_LEV;

    spi_transaction_t t = {
        .length = len * 8,              // Len is in bytes, transaction length is in bits.
        .tx_buffer = data,              // Data
        .user = (void *) dc,            // D/C needs to be set to 1
    };
    ret = _lcd_spi_send(spi, &t);       // Transmit!
    assert(ret == ESP_OK);              // Should have had no issues.
}

uint32_t lcd_init(lcd_conf_t* lcd_conf, spi_device_handle_t *spi_wr_dev, lcd_dc_t *dc, int dma_chan)
{
    //Initialize non-SPI GPIOs
    gpio_pad_select_gpio(lcd_conf->pin_num_dc);
    gpio_set_direction(lcd_conf->pin_num_dc, GPIO_MODE_OUTPUT);

    //Reset the display
    if (lcd_conf->pin_num_rst < GPIO_NUM_MAX) {
        gpio_pad_select_gpio(lcd_conf->pin_num_rst);
        gpio_set_direction(lcd_conf->pin_num_rst, GPIO_MODE_OUTPUT);
        gpio_set_level(lcd_conf->pin_num_rst, (lcd_conf->rst_active_level) & 0x1);
        vTaskDelay(100 / portTICK_RATE_MS);
        gpio_set_level(lcd_conf->pin_num_rst, (~(lcd_conf->rst_active_level)) & 0x1);
        vTaskDelay(100 / portTICK_RATE_MS);
    }

    if (lcd_conf->init_spi_bus) {
        //Initialize SPI Bus for LCD
        spi_bus_config_t buscfg = {
            .miso_io_num = lcd_conf->pin_num_miso,
            .mosi_io_num = lcd_conf->pin_num_mosi,
            .sclk_io_num = lcd_conf->pin_num_clk,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
        };
        spi_bus_initialize(lcd_conf->spi_host, &buscfg, dma_chan);
    }

    spi_device_interface_config_t devcfg = {
        // Use low speed to read ID.
        .clock_speed_hz = 1 * 1000 * 1000,        //Clock out frequency
        .mode = 0,                                //SPI mode 0
        .spics_io_num = lcd_conf->pin_num_cs,     //CS pin
        .queue_size = 7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb = lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
#if 0
    spi_device_handle_t rd_id_handle;
    spi_bus_add_device(lcd_conf->spi_host, &devcfg, &rd_id_handle);
    uint32_t lcd_id = lcd_get_id(rd_id_handle, dc);
    spi_bus_remove_device(rd_id_handle);
#else
    uint32_t lcd_id = lcd_get_id(NULL, dc);
#endif

    // Use high speed to write LCD
    devcfg.clock_speed_hz = lcd_conf->clk_freq;
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;
    spi_bus_add_device(lcd_conf->spi_host, &devcfg, spi_wr_dev);

    int cmd = 0;
    assert(lcd_init_cmds != NULL);
    //Send all the commands
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(*spi_wr_dev, lcd_init_cmds[cmd].cmd, dc);
        lcd_data(*spi_wr_dev, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F, dc);
        if (lcd_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    //Enable backlight
    if (lcd_conf->pin_num_bckl < GPIO_NUM_MAX) {
        gpio_pad_select_gpio(lcd_conf->pin_num_bckl);
        gpio_set_direction(lcd_conf->pin_num_bckl, GPIO_MODE_OUTPUT);
        gpio_set_level(lcd_conf->pin_num_bckl, (lcd_conf->bckl_active_level) & 0x1);
    }
    ESP_LOGI(TAG, "ST7735S Driver Initialized.");
    return lcd_id;
}

void lcd_send_uint16_r(spi_device_handle_t spi, const uint16_t data, int32_t repeats, lcd_dc_t *dc)
{
    uint32_t i;
    uint32_t word = data << 16 | data;
    uint32_t word_tmp[16];
    spi_transaction_t t;
    dc->dc_level = LCD_DATA_LEV;

    while (repeats > 0) {
        uint16_t bytes_to_transfer = MIN(repeats * sizeof(uint16_t), SPIFIFOSIZE * sizeof(uint32_t));
        for (i = 0; i < (bytes_to_transfer + 3) / 4; i++) {
            word_tmp[i] = word;
        }

        memset(&t, 0, sizeof(t));           //Zero out the transaction
        t.length = bytes_to_transfer * 8;   //Len is in bytes, transaction length is in bits.
        t.tx_buffer = word_tmp;             //Data
        t.user = (void *) dc;               //D/C needs to be set to 1
        _lcd_spi_send(spi, &t);             //Transmit!
        repeats -= bytes_to_transfer / 2;
    }
}

uint32_t lcd_get_id(spi_device_handle_t spi, lcd_dc_t *dc)
{
#if 0
    //get_id cmd
    lcd_cmd( spi, 0x04, dc);

    spi_transaction_t t;
    dc->dc_level = LCD_DATA_LEV;
    memset(&t, 0, sizeof(t));
    t.length = 8 * 4;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void *) dc;
    esp_err_t ret = _lcd_spi_send(spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*) t.rx_data;
#else
    ESP_LOGW(TAG, "internal id returned.");
    return 0x7735;
#endif
}

