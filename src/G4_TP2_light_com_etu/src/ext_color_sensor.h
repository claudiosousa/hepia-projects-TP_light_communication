/*
 * color_sensor.h
 *
 *  RGB sensor APDS 9960 driver.
 *
 *  Created on: 20.11.2017
 *  Author: VP
 */

#ifndef EXT_COLOR_SENSOR_H_
#define EXT_COLOR_SENSOR_H_

#include <stdint.h>
#include "LPC17xx.h"

#define EXT_COLORSENSOR_I2C_ADDR (0x39<<1)
#define EXT_CS_EN	0x80
#define EXT_CS_CTRL	0x8F
#define EXT_CS_FIRST_COLOR 0x94		// clear "color" data
#define EXT_CS_DEV_ID 0x92
#define EXT_CS_STATUS 0x93

#define APDS_PART_NUMBER 0xAB
#define APDS9960_EN 1
#define ALS_EN	(1<<1)				// ALS = Ambiant Light Sense
#define ALS_INT_EN	(1<<4)

#define CS_NOERROR	0
#define CS_INITERROR -31
#define CS_NOT_INITIALISED -32
#define CS_MEAS_DELAY_TOO_SHORT -33


typedef struct
{
	uint16_t clear;		// all color components (sort of sum, but frequency response is not exactly the smae)
	uint16_t red;
	uint16_t green;
	uint16_t blue;
} ext_cs_t;

/* Prototype of callback function called when one of both reception buffers
 * are filled. This function must be declared when color_sensor_init_int() is used.
 * Parameter: buffer_index: index of the first cs_t structure of the half buffer that has just been filled
 */
typedef void (*ext_cs_callbak_t)(int buffer_index);

/* Color sensor simple initialisation. Calling this function allows then to call colorsensor_read().
 * Parameter: i2c_freq: SLC frequency
 * Return: CS_NOERROR or CS_INITERROR if an error occurred
 */
int ext_colorsensor_init(uint32_t i2c_freq);

/* Color sensor initialisation to launch a continuous acquisition by interrupt of cs_t in a
 * circular buffer. A callback function is called when the first half of the buffer is filled and
 * also at the end of the buffer. The index given as a parameter by the callback function points on
 * first valid data that has been written by the interrupt. The value of the index is either 0 or buf_size/2.
 * The I2C clock frequency used is 400 kHz whatever meas_delay_us (which should be lower than 256 us).
 * Parameters: buffer: pointer on the cs_t buffer to be filled
 *             buf_size: buffer size (= number of cs_t structure inside)
 *             meas_delay_us: delay between each measure
 *             func: callback function called as described above
 * Return: CS_NOERROR or CS_INITERROR if an error occurred
 */
int ext_colorsensor_init_int(ext_cs_t *buffer, int buf_size, uint16_t meas_delay_us, ext_cs_callbak_t func);

/* Disable continuous acquisition (when color_sensor_init_int() has been called). */
void ext_colorsensor_stop_int();

/* Reads color sensor and fill cs_t structure.
 * Return: CS_NOERROR or CS_NOT_INITIALISED if colorsensor_init() was not called before
 */
int ext_colorsensor_read(ext_cs_t *data);

#endif /* EXT_COLOR_SENSOR_H_ */
