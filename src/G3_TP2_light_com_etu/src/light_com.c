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
#define CS_BUF_LEN (4*72*SAMPLES_PER_BIT)		// 1440 samples (2 frames of 72 symbols per half of the buffer, 1 symbol=2 bits)

#define LCD_MAGENTA	(LCD_RED | LCD_BLUE)
#define LCD_YELLOW	(LCD_RED | LCD_GREEN)

static ext_cs_t buffer[CS_BUF_LEN];

// global variables used for demo (to be removed!)
int buf_index = -1;
bool is_rx_char = false;
char c;

// callback function of the external RGB sensor
void rgb_callback(int buf_idx) {
	buf_index = buf_idx;
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
int normalize_red(int* seq_ref_red, int* seq_ref_blue) {
	//todo:normalize
	int max_red;
	int max_blue;
	//for eahc red: red => red * max_blue/ max_red;
}

int corr_index(int* seq_ref_red, int* seq_ref_blue) {
	long long max_corr_value = 0;
	int max_corr_index;
	long long corr_value;
	int corr_length = 8 * SAMPLES_PER_BIT;
	for (int ibuffer = 0; ibuffer < CS_BUF_LEN - corr_length; ibuffer++) {
		corr_value = 0;
		for (int i = 0; i < corr_length; i++) {
			corr_value += buffer[i + ibuffer].red * seq_ref_red[i];
			corr_value += buffer[i + ibuffer].blue * seq_ref_blue[i];
		}
		if (corr_value > max_corr_value) {
			max_corr_value = corr_value;
			max_corr_index = ibuffer
		}
	}
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

	memset(buffer, 0, sizeof(buffer));
	if (ext_colorsensor_init_int(buffer, CS_BUF_LEN, 1000000 / 625, rgb_callback) != CS_NOERROR)
		exit(1);

	// wait call of RGB sensor callback:
	while (buf_index == -1) {
	};

	int seq_ref_red[8 * SAMPLES_PER_BIT];
	int seq_ref_blue[8 * SAMPLES_PER_BIT];
	build_seq_refs(seq_ref_red, seq_ref_blue);

	normalize_red(seq_ref_red, seq_ref_blue);

	lcd_printf(LCD_MAGENTA, LCD_BLACK, "%i", sizeof(long long));

//int2file("scripts/seq_ref_red.txt", seq_ref_red,sizeof(seq_ref_red) / sizeof(int), sizeof(int), false);
//int2file("scripts/seq_ref_blue.txt", seq_ref_blue, sizeof(seq_ref_blue) / sizeof(int), sizeof(int), false);

// save the RGB frame received
	int2file("scripts/rgb.txt", buffer,	CS_BUF_LEN * sizeof(ext_cs_t) / sizeof(int16_t) / 2, sizeof(int16_t),
	false);

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
