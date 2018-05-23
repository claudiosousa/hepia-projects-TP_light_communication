/**
 * Command decoder module (Implementation)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include "command_decoder.h"
#include <stdlib.h>
#include <string.h>
#include "LPC17xx.h"
#include "uart.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LCD_MAGENTA	(LCD_RED | LCD_BLUE)
#define LCD_YELLOW	(LCD_RED | LCD_GREEN)

#define CMD_SEND_LENGTH 30

#define CMD_SCROLL_DELAY_SLOW 450
#define CMD_SCROLL_DELAY_FAST 220

#define CMD_COMMAND_COLOR '\x02'
#define CMD_COMMAND_SCROLL '\x03'
#define CMD_COMMAND_LOAD '\x04'
#define CMD_COMMAND_LEDS '\x05'
static const char * CMD_TO_STR[] = { "color", "scroll", "load", "leds" };

// Allow the UART callback to send the characters to the task
static xQueueHandle char_queue;

/**
 * Determine whether an element is available for reading from the buffer
 * @param buf Circular buffer for which we want to know
 * @return TRUE if at least one element is available, FALSE otherwise
 */
bool string_circular_buffer_has(string_circular_buffer * buf) {
	return buf->read_index != buf->write_index;
}

/**
 * Add a new string to the given buffer
 * @param buf Circular buffer where to add the string
 * @param str String to add
 */
void string_circular_buffer_add(string_circular_buffer * buf, char * str) {
	strncpy(buf->buffer[buf->write_index], str, CMD_STR_LENGTH);
	buf->buffer[buf->write_index][CMD_STR_LENGTH - 1] = '\0';
	buf->write_index = (buf->write_index + 1) % CMD_BUFFER_LENGTH;
}

/**
 * Pop the next string in the buffer
 * @param buf Circular buffer from where to get the string
 * @return The pointer to the string currently available
 */
char * string_circular_buffer_pop(string_circular_buffer * buf) {
	char * ret = buf->buffer[buf->read_index];
	buf->read_index = (buf->read_index + 1) % CMD_BUFFER_LENGTH;
	return ret;
}

/**
 * Callback function of the UART RX.
 * Receive character one by one and put them in the queue.
 */
void uart_rx_cmd_callback(int int_status) {
	uint8_t c = LPC_UART0->RBR;
	portBASE_TYPE task_woken = 0;
	xQueueSendToBackFromISR(char_queue, &c, &task_woken);
}

/**
 * Reset the character buffer that get the command from UART
 * @param cmd_decoder Decoder data to work on
 */
void cmd_command_buffer_reset(command_decoder_t * cmd_decoder) {
	memset(cmd_decoder->command_buffer, '\0', CMD_UART_LENGTH);
	cmd_decoder->command_buffer_idx = 0;
}

/**
 * Get the character from the queue and put it in the temporary buffer.
 * If a complet command is received, then the output will be put in 'cmd'
 * @param cmd_decoder Decoder data to work on
 * @param cmd Character buffer to put the result if complet
 * @return TRUE if a complet command is copied, FALSE otherwise
 */
bool cmd_get_next_command_in_buffer(command_decoder_t * cmd_decoder, char * cmd) {
	bool complet = false;
	uint8_t c = '\0';

	while ((complet == false) && (xQueueReceive(char_queue, &c, 0) == pdTRUE)) {
		cmd_decoder->command_buffer[cmd_decoder->command_buffer_idx] = c;
		cmd_decoder->command_buffer_idx++;

		if ((c == '\0') || (c == '\n') || (strlen(cmd_decoder->command_buffer) >= CMD_SEND_LENGTH)) {
			strncpy(cmd, cmd_decoder->command_buffer, CMD_STR_LENGTH);
			cmd_command_buffer_reset(cmd_decoder);
			complet = true;
		}
	}

	return complet;
}

void cmd_init(command_decoder_t * cmd_decoder) {
	init_lcd();
	clear_screen(LCD_BLACK);
	setup_scroll(0, 10, 0);
	uart0_init_ref(115200, NULL, uart_rx_cmd_callback);

	char_queue = xQueueCreate(CMD_UART_LENGTH, sizeof(uint8_t));

	cmd_command_buffer_reset(cmd_decoder);
	cmd_decoder->message_print_buffer.read_index = 0;
	cmd_decoder->message_print_buffer.write_index = 0;
	cmd_decoder->command_print_buffer.read_index = 0;
	cmd_decoder->command_print_buffer.write_index = 0;
	cmd_decoder->emitter_text_color = LCD_WHITE;
	cmd_decoder->scroll_delay = CMD_SCROLL_DELAY_SLOW;
	cmd_decoder->scroll_auto = true;
}

void cmd_send_message(command_decoder_t * cmd_decoder, char * msg) {
	string_circular_buffer_add(&cmd_decoder->message_print_buffer, msg);
}

void cmd_decode_next(command_decoder_t * cmd_decoder) {
	char cmd_mem[CMD_STR_LENGTH];
	memset(cmd_mem, '\0', CMD_STR_LENGTH);
	char * cmd = cmd_mem; // Allow to skip first char

	if (cmd_get_next_command_in_buffer(cmd_decoder, cmd)) {
		char * cmd_content = cmd + 1; // Skip first char to ease test

		if (cmd[0] == CMD_COMMAND_COLOR) {
			if (strncmp(cmd_content, "white", 6) == 0) {
				cmd_decoder->emitter_text_color = LCD_WHITE;
			}
			else if (strncmp(cmd_content, "yellow", 7) == 0) {
				cmd_decoder->emitter_text_color = LCD_YELLOW;
			}
			else if (strncmp(cmd_content, "red", 4) == 0) {
				cmd_decoder->emitter_text_color = LCD_RED;
			}
			else if (strncmp(cmd_content, "green", 6) == 0) {
				cmd_decoder->emitter_text_color = LCD_GREEN;
			}
			else if (strncmp(cmd_content, "blue", 5) == 0) {
				cmd_decoder->emitter_text_color = LCD_BLUE;
			}
		}
		else if (cmd[0] == CMD_COMMAND_SCROLL) {
			if (strncmp(cmd_content, "slow", 5) == 0) {
				cmd_decoder->scroll_delay = CMD_SCROLL_DELAY_SLOW;
				cmd_decoder->scroll_auto = true;
			}
			else if (strncmp(cmd_content, "fast", 5) == 0) {
				cmd_decoder->scroll_delay = CMD_SCROLL_DELAY_FAST;
				cmd_decoder->scroll_auto = true;
			}
			else if (strncmp(cmd_content, "stop", 5) == 0) {
				cmd_decoder->scroll_auto = false;
			}
		}
		else if (cmd[0] == CMD_COMMAND_LOAD) {

		}
		else if (cmd[0] == CMD_COMMAND_LEDS) {
			if (strncmp(cmd_content, "on", 3) == 0) {

			}
			else if (strncmp(cmd_content, "off", 4) == 0) {

			}
		}
		else { } // Send command, this is only text, just take it

		string_circular_buffer_add(&cmd_decoder->command_print_buffer, cmd);
	}
}

void cmd_print(command_decoder_t * cmd_decoder) {
	char str_to_print[CMD_STR_LENGTH];
	memset(str_to_print, '\0', CMD_STR_LENGTH);
	int color = LCD_WHITE;

	if (string_circular_buffer_has(&cmd_decoder->message_print_buffer)) {
		strncpy(str_to_print, string_circular_buffer_pop(&cmd_decoder->message_print_buffer), CMD_STR_LENGTH);
		color = cmd_decoder->emitter_text_color;
	}
	else if (string_circular_buffer_has(&cmd_decoder->command_print_buffer)) {
		color = LCD_WHITE;
		char * cmd = string_circular_buffer_pop(&cmd_decoder->command_print_buffer);
		uint8_t cmd_marker = cmd[0];

		if ((cmd_marker >= CMD_COMMAND_COLOR) && (cmd_marker <= CMD_COMMAND_LEDS)) {
			color = LCD_MAGENTA;
			strncat(str_to_print, CMD_TO_STR[cmd_marker - CMD_COMMAND_COLOR], CMD_STR_LENGTH);
			strncat(str_to_print, " ", CMD_STR_LENGTH);
			cmd++; // Don't print the command char
		}
		strncat(str_to_print, cmd, CMD_STR_LENGTH);
	}
	str_to_print[CMD_STR_LENGTH - 1] = '\0';

	if ((cmd_decoder->scroll_auto) || (str_to_print[0] != '\0')) {
		lcd_printf(color, LCD_BLACK, "%s\n", str_to_print);
	}
}

void cmd_task(void * param) {
	command_decoder_t * cmd_decoder = (command_decoder_t*)param;
	portTickType tick_start = xTaskGetTickCount();

	while (1) {
		cmd_decode_next(cmd_decoder);
		cmd_print(cmd_decoder);

		vTaskDelayUntil(&tick_start, cmd_decoder->scroll_delay / portTICK_RATE_MS);
	}
}
