/*
   This code generates an effect that should pass the 'fancy graphics' qualification
   as set in the comment in the spi_master code.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <math.h>
#include <string.h>
#include "pretty_effect.h"
#include "decode_image.h"

#define LCD_X_SIZE      128
#define LCD_Y_SIZE      160

uint16_t **pixels;

//Calculate the pixel data for a set of lines (with implied line size of LCD_X_SIZE). Pixels go in dest, line is the Y-coordinate of the
//first line to be calculated, linect is the amount of lines to calculate. Frame increases by one every time the entire image
//is displayed; this is used to go to the next frame of animation.
void pretty_effect_calc_lines(uint16_t *dest, int line, int linect)
{
    for (int y=line; y<line+linect; y++) {
        for (int x=0; x<LCD_X_SIZE; x++) {
        	*dest++=pixels[y][x];
        }
    }
}


esp_err_t pretty_effect_init() 
{
    return decode_image(&pixels);
}
