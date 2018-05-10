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

#define CS_BUF_LEN (4*72*5)		// 1440 samples (2 frames of 72 symbols per half of the buffer, 1 symbol=2 bits)

#define LCD_MAGENTA	(LCD_RED | LCD_BLUE)
#define LCD_YELLOW	(LCD_RED | LCD_GREEN)


static ext_cs_t buffer[CS_BUF_LEN];

// global variables used for demo (to be removed!)
int buf_index=-1;
bool is_rx_char=false;
char c;

// callback function of the external RGB sensor
void rgb_callback(int buf_idx)
{
	buf_index=buf_idx;
	ext_colorsensor_stop_int();		// stop acquisition to allow the frame saving
}

// callback function of the UART RX
void uart_rx_cmd_callback(int int_status)
{
	c=LPC_UART0->RBR;				// UART RX character read
	is_rx_char=true;
}

int main(void)
{
	init_lcd();
	clear_screen(LCD_BLACK);

	memset(buffer, 0, sizeof(buffer));
	if (ext_colorsensor_init_int(buffer, CS_BUF_LEN, 1000000/625, rgb_callback)!=CS_NOERROR)
		exit(1);

	// wait call of RGB sensor callback:
	while(buf_index==-1);

	// save the RGB frame received
	int2file("scripts/rgb.txt", buffer, CS_BUF_LEN*sizeof(ext_cs_t)/sizeof(int16_t)/2, sizeof(int16_t), false);

	// trace initialisation (note: configHEPIA_TRACINGmust be set to MYLABLIB_TRACES in FreeRTOS Config.h)
	init_traces(115200, 2, true);

	// UART initialisation for RX and (bad!) example of use
	uart0_init_ref(115200, NULL, uart_rx_cmd_callback);

	// wait call of UART callback and print the first character received:
	while(!is_rx_char);
	lcd_printf(LCD_MAGENTA, LCD_BLACK, "0x%02x", c);

	return 1;
}
