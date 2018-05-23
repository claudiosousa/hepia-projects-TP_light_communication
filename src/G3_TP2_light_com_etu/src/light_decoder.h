/**
 * Light decoder module (Header)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include <stdlib.h>
#include <stdint.h>
#include "ext_color_sensor.h"
#include "command_decoder.h"

#ifndef LIGHT_DECODER_H_
#define LIGHT_DECODER_H_

#define LIGHT_SAMPLES_PER_BIT 5
// 1440 samples (2 frames of 72 symbols per half of the buffer, 1 symbol=2 bits)
#define LIGHT_BUF_LEN (2 * 72 * LIGHT_SAMPLES_PER_BIT)

typedef struct light_decoder_t {
	// Two buffers for two frame in one
	ext_cs_t double_buffer[LIGHT_BUF_LEN * 2];
	// The pointer to the actual buffer available for processing
	ext_cs_t * buffer;
	// Allow to send the message to print
	command_decoder_t * cmd_decoder;
} light_decoder_t;

/**
 * Initialize the light decoder
 * @param light_decoder Memory to initialise
 * @param cmd_decoder Pointer to the command decoder data, this allow to send message to print
 */
void ld_init(light_decoder_t * light_decoder, command_decoder_t * cmd_decoder);

/**
 * Task for RTOS
 * @param param Light decoder data
 */
void ld_task(void * param);

#endif /* LIGHT_DECODER_H_ */
