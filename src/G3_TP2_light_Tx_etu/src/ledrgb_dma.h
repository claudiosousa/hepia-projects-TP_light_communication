/*
 * ledrgb_dma.h
 *
 *  Created on: Mar 26, 2014
 *      Author: Adrien, V. Pilloux
 */

#ifndef LEDRGB_DMA_H_
#define LEDRGB_DMA_H_

#include "LPC17xx.h"

typedef struct {
	uint8_t R;
	uint8_t G;
	uint8_t B;
} rgb_t;

void set_rgb_led_color(uint32_t RGB);

/*
 * Setup the RGB led1&2 with the desired RGB colors
 * Parameters: rgb1 for led1, rgb2 for led2
 */
void set_rgb_leds_color3(rgb_t rgb1, rgb_t rgb2);

/*
 * Setup the RGB led with the desired RGB color
 * Parameters: RGB color strength, on 24 bits (i.e. 0xFFFF00 = yellow)
 */
void set_rgb_leds_color(uint32_t rgb1, uint32_t rgb2);

#endif /* LEDRGB_DMA_H_ */
