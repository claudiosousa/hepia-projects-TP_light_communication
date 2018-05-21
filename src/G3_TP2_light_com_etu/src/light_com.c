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
	init_traces(115200, 2, true);
	command_decoder_t decoder_data = cmd_init();
	ld_init();

	//int2file("scripts/seq_ref_red.txt", seq_ref_red,sizeof(seq_ref_red) / sizeof(int), sizeof(int), false);
	//int2file("scripts/seq_ref_blue.txt", seq_ref_blue, sizeof(seq_ref_blue) / sizeof(int), sizeof(int), false);
	// save the RGB frame received
	//int2file("scripts/rgb.txt", double_buffer,	BUF_LEN * sizeof(ext_cs_t) / sizeof(int16_t), sizeof(int16_t),	false);
	//int2file("scripts/rgb_normalized.txt", double_buffer,	BUF_LEN * sizeof(ext_cs_t) / sizeof(int16_t), sizeof(int16_t),	false);

	while (1) {
		ld_process();
		cmd_decode_next(&decoder_data);
		cmd_print(&decoder_data);
		cmd_print(&decoder_data);
	}

	return 1;
}
