/**
 * Name        : light_com.c
 * Date        : 1.2.2018
 * Description : template of TP2: RB light communication
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
#include "lcd.h"
#include "debug.h"
#include "traces_ref.h"
#include "ext_color_sensor.h"
#include "uart.h"
#include "LPC17xx.h"

#define SAMPLES_PER_BIT 5
#define DATA_LENGTH 15

#define BUF_LEN (2*72*SAMPLES_PER_BIT)		// 1440 samples (2 frames of 72 symbols per half of the buffer, 1 symbol=2 bits)

#define LCD_MAGENTA	(LCD_RED | LCD_BLUE)
#define LCD_YELLOW	(LCD_RED | LCD_GREEN)

static ext_cs_t double_buffer[BUF_LEN * 2];
static ext_cs_t * buffer = NULL;

// global variables used for demo (to be removed!)
int buf_index = -1;
bool is_rx_char = false;
char c;

// callback function of the external RGB sensor
void rgb_callback(int buf_idx) {
	buf_index = buf_idx;
	buffer = &double_buffer[buf_idx];
	ext_colorsensor_stop_int();	// stop acquisition to allow the frame saving
}

// callback function of the UART RX
void uart_rx_cmd_callback(int int_status) {
	c = LPC_UART0->RBR;				// UART RX character read
	is_rx_char = true;
}

void build_seq_refs(int* seq_ref_red, int* seq_ref_blue) {
	int bits_ref[16] = { 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1 }; //0x951B
	for (int ibit = 0; ibit < 16 / 2; ibit++) {
		int red = bits_ref[ibit * 2] ? 1 : -1;
		int blue = bits_ref[ibit * 2 + 1] ? 1 : -1;
		for (int i = 0; i < SAMPLES_PER_BIT; i++) {
			seq_ref_red[ibit * SAMPLES_PER_BIT + i] = red;
			seq_ref_blue[ibit * SAMPLES_PER_BIT + i] = blue;
		}
	}

}
int normalize_red() {
	//todo:normalize
	int max_red = 0, max_blue = 0;
	for (int i = 0; i < BUF_LEN; i++) {
		max_red = MAX(max_red, buffer[i].red);
		max_blue = MAX(max_blue, buffer[i].blue);
	}

	float ratio = (float) max_blue / max_red;
	for (int i = 0; i < BUF_LEN; i++)
		buffer[i].red *= ratio;
	return MAX(max_red, max_blue);
}

int corr_index(int* seq_ref_red, int* seq_ref_blue, int amplitude_offset) {
	long long max_corr_value = 0;
	int max_corr_index;
	long long corr_value;
	int corr_length = 8 * SAMPLES_PER_BIT;

	for (int ibuffer = 0; ibuffer < BUF_LEN / 2; ibuffer++) {
		corr_value = 0;
		for (int i = 0; i < corr_length; i++) {
			corr_value += (buffer[i + ibuffer].red - amplitude_offset)
					* seq_ref_red[i];
			corr_value += (buffer[i + ibuffer].blue - amplitude_offset)
					* seq_ref_blue[i];
		}
		if (corr_value > max_corr_value) {
			max_corr_value = corr_value;
			max_corr_index = ibuffer;
		}
	}

	return max_corr_index;
}

/*
 CS_BUF_LEN
 buffer

 function
 res = calc_corr(l, seq_ref, noisy_signal)
 res = seq_ref*noisy_signal(l:l+length(seq_ref)-1)';
 endfunction

 len_corr = length(noisy_signal)-length(seq_ref);
 corr_my = arrayfun(@(l)calc_corr(l, seq_ref, noisy_signal), 1:len_corr);
 */

int main(void) {
	init_lcd();
	clear_screen(LCD_BLACK);
	setup_scroll(0, 10, 0);

	memset(double_buffer, 0, sizeof(double_buffer));
	if (ext_colorsensor_init_int(double_buffer, BUF_LEN * 2, 1000000 / 625,
			rgb_callback) != CS_NOERROR)
		exit(1);

	// wait call of RGB sensor callback:
	while (buf_index == -1) {
	};

	//int2file("scripts/seq_ref_red.txt", seq_ref_red,sizeof(seq_ref_red) / sizeof(int), sizeof(int), false);
	//int2file("scripts/seq_ref_blue.txt", seq_ref_blue, sizeof(seq_ref_blue) / sizeof(int), sizeof(int), false);
	// save the RGB frame received
	//int2file("scripts/rgb.txt", double_buffer,	BUF_LEN * sizeof(ext_cs_t) / sizeof(int16_t), sizeof(int16_t),	false);

	int seq_ref_red[8 * SAMPLES_PER_BIT];
	int seq_ref_blue[8 * SAMPLES_PER_BIT];
	build_seq_refs(seq_ref_red, seq_ref_blue);

	int amplitude_threshold = normalize_red() /2;
	int sync = corr_index(seq_ref_red, seq_ref_blue, amplitude_threshold);

	//int2file("scripts/rgb_normalized.txt", double_buffer,	BUF_LEN * sizeof(ext_cs_t) / sizeof(int16_t), sizeof(int16_t),	false);

	char data[DATA_LENGTH + 1];
	data[15] = 0;
	ext_cs_t * data_buffer = &buffer[sync + SAMPLES_PER_BIT * 8 + 2];
	for (int i = 0; i < DATA_LENGTH; i++) {
		unsigned char c = 0;
		for (int ib =7;ib>=1;ib-=2){
			if (data_buffer->red > amplitude_threshold)
				c+= 1<<ib;
			if (data_buffer->blue > amplitude_threshold)
				c+= 1<<(ib-1);
			data_buffer += SAMPLES_PER_BIT;
		}
		data[i] = c;
	}
	lcd_printf(LCD_MAGENTA, LCD_BLACK, "%s", data);

// trace initialisation (note: configHEPIA_TRACINGmust be set to MYLABLIB_TRACES in FreeRTOS Config.h)
	init_traces(115200, 2, true);

// UART initialisation for RX and (bad!) example of use
	uart0_init_ref(115200, NULL, uart_rx_cmd_callback);

// wait call of UART callback and print the first character received:
	while (!is_rx_char)
		;
	lcd_printf(LCD_MAGENTA, LCD_BLACK, "0x%02x", c);

	return 1;
}
