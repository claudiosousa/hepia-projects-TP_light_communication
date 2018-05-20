/**
 * Light decoder module (Header)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include <stdlib.h>
#include <stdint.h>

#ifndef LIGHT_DECODER_H_
#define LIGHT_DECODER_H_

// String length for received message
#define LIGHT_STR_LENGTH 31
#define LIGHT_DATA_LENGTH 15
#define LIGHT_SAMPLES_PER_BIT 5
// 1440 samples (2 frames of 72 symbols per half of the buffer, 1 symbol=2 bits)
#define LIGHT_BUF_LEN (2 * 72 * LIGHT_SAMPLES_PER_BIT)

/**
 * Initialize the light decoder
 */
void ld_init();

/**
 * Start or restart acquisition
 */
void ld_start();

/**
 * Process the available buffer
 */
void ld_process();

#endif /* LIGHT_DECODER_H_ */
