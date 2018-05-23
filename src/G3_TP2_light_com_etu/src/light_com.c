/**
 * RB light communication
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include <cr_section_macros.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "debug.h"
#include "traces_ref.h"
#include "LPC17xx.h"
#include "light_decoder.h"
#include "command_decoder.h"

/**
 * Main function
 */
int main(void) {
	static light_decoder_t light_decoder;
	static command_decoder_t cmd_decoder;

	init_traces(115200, 2, true);
	cmd_init(&cmd_decoder);
	ld_init(&light_decoder, &cmd_decoder);

	xTaskCreate(
		ld_task,
		(signed portCHAR *)"light_decoder",
		configMINIMAL_STACK_SIZE,
		&light_decoder,
		tskIDLE_PRIORITY + 1,
		NULL
	);
	xTaskCreate(
		cmd_task,
		(signed portCHAR *)"command_decoder",
		configMINIMAL_STACK_SIZE,
		&cmd_decoder,
		tskIDLE_PRIORITY + 1,
		NULL
	);

	//int2file("scripts/seq_ref_red.txt", seq_ref_red,sizeof(seq_ref_red) / sizeof(int), sizeof(int), false);
	//int2file("scripts/seq_ref_blue.txt", seq_ref_blue, sizeof(seq_ref_blue) / sizeof(int), sizeof(int), false);
	// save the RGB frame received
	//int2file("scripts/rgb.txt", double_buffer,	BUF_LEN * sizeof(ext_cs_t) / sizeof(int16_t), sizeof(int16_t),	false);
	//int2file("scripts/rgb_normalized.txt", double_buffer,	BUF_LEN * sizeof(ext_cs_t) / sizeof(int16_t), sizeof(int16_t),	false);

	vTaskStartScheduler();

	return 1;
}
