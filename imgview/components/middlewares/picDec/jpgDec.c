/* jpeg decoder.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


/*
The image used for the effect on the LCD in the SPI master example is stored in flash 
as a jpeg file. This file contains the decode_image routine, which uses the tiny JPEG 
decoder library in ROM to decode this JPEG into a format that can be sent to the display.

Keep in mind that the decoder library cannot handle progressive files (will give 
``Image decoder: jd_prepare failed (8)`` as an error) so make sure to save in the correct
format if you want to use a different image file.
*/

#include "rom/tjpgd.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include "jpgDec.h"
#include "esp_heap_caps.h"

const char *TAG = "JPEG_DEC";

#define PIXEL_FIFO_SIZE        320

//Data that is passed from the decoder function to the infunc/outfunc functions.
typedef struct {
    FILE *f;                        //Pointer to jpeg file stream.
    int inPos;						//Current position in jpeg data
    pDrawPrepare_t DrawPrepare;     //Initialize the displayer to draw pixel
    pFillScreen_t FillPixel;        //pointer to a function to fill pixel data to displayer.
    uint16_t *outFIFO;              //fifo to store rgb data.
    int outPos;                     //Current position of rgb data;
} JpegDev;

//Input function for jpeg decoder. Just returns bytes from the inData field of the JpegDev structure.
static UINT infunc(JDEC *decoder, BYTE *buf, UINT len) 
{
    //Read bytes from input file
    JpegDev *jd = (JpegDev*)decoder->device;
    fseek(jd->f, jd->inPos, SEEK_SET);
    if (buf != NULL)
    	len = fread(buf, 1, len, jd->f);
    jd->inPos += len;
    return len;
}

//Output function. Re-encodes the RGB888 data from the decoder as big-endian RGB565 and
//stores it in the outData array of the JpegDev structure.
static UINT outfunc(JDEC *decoder, void *bitmap, JRECT *rect) 
{
    JpegDev *jd = (JpegDev *)decoder->device;
    uint8_t *in = (uint8_t *)bitmap;
    ImgArea_t area = {.left = rect->left, .right = rect->right, .top = rect->top, .bottom = rect->bottom};
    jd->DrawPrepare(&area);
    for (int y = rect->top; y <= rect->bottom; y ++) {
        for (int x = rect->left; x <= rect->right; x ++) {
            //We need to convert the 3 bytes in `in` to a rgb565 value.
            uint16_t v = 0;
            v|=((in[0]>>3)<<11);
            v|=((in[1]>>2)<<5);
            v|=((in[2]>>3)<<0);
            jd->outFIFO[jd->outPos ++] = v;
            if(jd->outPos >= PIXEL_FIFO_SIZE) {
            	jd->FillPixel(jd->outFIFO, jd->outPos);
            	jd->outPos = 0;
            }
            in += 3;
        }
    }
    if(jd->outPos > 0) {
    	jd->FillPixel(jd->outFIFO, jd->outPos);
    	jd->outPos = 0;
    }
    return 1;
}

//Size of the work space for the jpeg decoder.
#define WORKSZ 3100

//Decode the embedded image into pixel lines that can be used with the rest of the logic.
esp_err_t jpg_decode(const char *path, pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen)
{
    char *work = NULL;
    int r;
    JDEC decoder;
    JpegDev jd;
    esp_err_t ret = ESP_OK;
    FILE *f = fopen(path, "r"); // read only.
    if(f == NULL) {
    	ESP_LOGE(TAG, "can't open file %s", path);
    	return ESP_FAIL;
    }

    //Allocate the work space for the jpeg decoder.
    work = calloc(WORKSZ, 1);
    if (work == NULL) {
        ESP_LOGE(TAG, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    //Populate fields of the JpegDev struct.
    jd.f = f;
    jd.DrawPrepare = pDrawPrepare;
    jd.FillPixel = pFillScreen;
    jd.inPos = 0;
    jd.outFIFO = NULL;
    jd.outPos = 0;

    //Alocate pixel memory.
	jd.outFIFO = (uint16_t *)heap_caps_malloc(PIXEL_FIFO_SIZE * sizeof(uint16_t), MALLOC_CAP_DMA);
	if(jd.outFIFO == NULL) {
		ESP_LOGE(TAG, "Cannot allocate rgb fifo");
		ret = ESP_ERR_NO_MEM;
		goto err;
	}

    //Prepare and decode the jpeg.
    r = jd_prepare(&decoder, infunc, work, WORKSZ, (void*)&jd);
    if (r != JDR_OK) {
        ESP_LOGE(TAG, "Image decoder: jd_prepare failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }
    ESP_LOGI(TAG, "JPG, size:%dx%d", decoder.width, decoder.height);

    r = jd_decomp(&decoder, outfunc, 0);
    if (r!=JDR_OK) {
        ESP_LOGE(TAG, "Image decoder: jd_decode failed (%d)", r);
        ret=ESP_ERR_NOT_SUPPORTED;
        goto err;
    }

    //All done! Free the work area (as we don't need it anymore) and return victoriously.
err:
    //Something went wrong! Exit cleanly, de-allocating everything we allocated.
	fclose(f);
    free(work);
    if(jd.outFIFO != NULL)
    	heap_caps_free(jd.outFIFO);
    return ret;
}
