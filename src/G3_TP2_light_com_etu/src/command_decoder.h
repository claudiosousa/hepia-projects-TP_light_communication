/**
 * Command decoder module (Header)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#ifndef COMMAND_DECODER_H_
#define COMMAND_DECODER_H_

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define CMD_UART_LENGTH 256

typedef struct command_decoder_t {
	// Temporary buffer for current decoded command from the queue
	char command_buffer[CMD_UART_LENGTH];
	// Position to insert the next character from the queue
	unsigned int command_buffer_idx;

	// Buffers for strings to print
	xQueueHandle message_print_buffer;
	xQueueHandle command_print_buffer;

	// Color of the emitter text
	int emitter_text_color;
	// Scroll delay, see: CMD_SCROLL_DELAY_*
	int scroll_delay;
	// Either to print empty string for message
	bool scroll_auto;
} command_decoder_t;

/**
 * Initialise the command module
 * @param Decoder data memory to initialise
 */
void cmd_init(command_decoder_t * cmd_decoder);

/**
 * Send a message to the command module for printing
 * @param cmd_decoder Decoder data to work on
 * @param msg Message to print
 */
void cmd_send_message(command_decoder_t * cmd_decoder, char * msg);

/**
 * Decode and execute the next command in the buffer
 * @param cmd_decoder Decoder data to work on
 */
void cmd_decode_next(command_decoder_t * cmd_decoder);

/**
 * Print string from the buffer
 * @param cmd_decoder Decoder data to work on
 */
void cmd_print(command_decoder_t * cmd_decoder);

/**
 * Command decoder task
 * @param param Decoder data
 */
void cmd_task(void * param);

#endif /* COMMAND_DECODER_H_ */
