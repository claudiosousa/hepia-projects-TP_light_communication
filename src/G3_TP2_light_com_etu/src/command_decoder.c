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

#define LCD_MAGENTA	(LCD_RED | LCD_BLUE)
#define LCD_YELLOW	(LCD_RED | LCD_GREEN)

#define CMD_SEND_LENGTH 30
#define CMD_UART_LENGTH 256
#define CMD_UART_MARKER_CHAR '\x01'

#define CMD_SCROLL_DELAY_SLOW 450
#define CMD_SCROLL_DELAY_FAST 220

#define CMD_COMMAND_COLOR '\x02'
#define CMD_COMMAND_SCROLL '\x03'
#define CMD_COMMAND_LEDS '\x04'
#define CMD_COMMAND_LOAD '\x05'

// Circular buffer for command from UART
static char cmd_recv[CMD_UART_LENGTH];
// Index from where to add character when received
static unsigned int cmd_recv_write_i = 0;
// Count of the number of character after the last marker has been placed
static unsigned int cmd_recv_marker_count = 0;

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
 * Receive character one by one.
 * To make it easier for the command decoder to find the command inside,
 * the callback insert an additional marker at the end of the command.
 */
void uart_rx_cmd_callback(int int_status) {
	uint8_t c = LPC_UART0->RBR;
	cmd_recv[cmd_recv_write_i] = c;
	cmd_recv_write_i = (cmd_recv_write_i + 1) % CMD_UART_LENGTH;
	cmd_recv_marker_count++;

	// Place marker for easy retrieval
	// Place on command end, text linefeed or text filled
	if ((c == '\0') || (c == '\n') || (cmd_recv_marker_count >= CMD_SEND_LENGTH)) {
		cmd_recv[cmd_recv_write_i] = CMD_UART_MARKER_CHAR;
		cmd_recv_write_i = (cmd_recv_write_i + 1) % CMD_UART_LENGTH;
		cmd_recv_marker_count = 0;
	}
}

bool cmd_get_next_command_in_buffer(command_decoder_t * cmd_decoder, char * cmd) {
	bool found = false;

	if (cmd_decoder->cmd_recv_read_i != cmd_recv_write_i) {
		char cmd_working[CMD_STR_LENGTH];
		memset(cmd_working, '\0', CMD_STR_LENGTH);

		// Real number of chars of the command if found
		unsigned int cmd_num_c = 0;
		// Compute number of chars to test
		unsigned int cmd_num_c_max = (cmd_decoder->cmd_recv_read_i <= cmd_recv_write_i) ?
			(cmd_recv_write_i - cmd_decoder->cmd_recv_read_i) : // Interval
			(cmd_recv_write_i + (CMD_UART_LENGTH - cmd_decoder->cmd_recv_read_i)); // Start + End
		// Copy until next marker or max char
		unsigned int i = cmd_decoder->cmd_recv_read_i;
		while ((cmd_num_c < cmd_num_c_max) && (cmd_recv[i % CMD_UART_LENGTH] != CMD_UART_MARKER_CHAR)) {
			cmd_working[cmd_num_c] = cmd_recv[i % CMD_UART_LENGTH];
			cmd_num_c++;
			i++;
		}
		// In case a marker is found, we can validate the command
		if (cmd_recv[i % CMD_UART_LENGTH] == CMD_UART_MARKER_CHAR) {
			strncat(cmd, cmd_working, CMD_STR_LENGTH);
			found = true;

			// Advance our reader index
			cmd_decoder->cmd_recv_read_i = (cmd_decoder->cmd_recv_read_i + cmd_num_c + 1) % CMD_UART_LENGTH; // Skip marker
		}
	}

	return found;
}

void cmd_init(command_decoder_t * cmd_decoder) {
	init_lcd();
	clear_screen(LCD_BLACK);
	setup_scroll(0, 10, 0);
	uart0_init_ref(115200, NULL, uart_rx_cmd_callback);

	memset(cmd_recv, '\0', CMD_UART_LENGTH);
	cmd_recv_write_i = 0;
	cmd_recv_marker_count = 0;

	cmd_decoder->message_print_buffer.read_index = 0;
	cmd_decoder->message_print_buffer.write_index = 0;
	cmd_decoder->command_print_buffer.read_index = 0;
	cmd_decoder->command_print_buffer.write_index = 0;
	cmd_decoder->emitter_text_color = LCD_WHITE;
	cmd_decoder->scroll_delay = CMD_SCROLL_DELAY_SLOW;
	cmd_decoder->scroll_auto = true;
	cmd_decoder->cmd_recv_read_i = 0;
}

void cmd_send_message(command_decoder_t * cmd_decoder, char * msg) {
	string_circular_buffer_add(&cmd_decoder->message_print_buffer, msg);
}

void cmd_decode_next(command_decoder_t * cmd_decoder) {
	char cmd_mem[CMD_STR_LENGTH];
	memset(cmd_mem, '\0', CMD_STR_LENGTH);
	char * cmd = cmd_mem; // Allow to skip first char

	if (cmd_get_next_command_in_buffer(cmd_decoder, cmd)) {
		char * cmd_content = cmd + 1;

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
		else if (cmd[0] == CMD_COMMAND_LEDS) {
			if (strncmp(cmd_content, "on", 3) == 0) {

			}
			else if (strncmp(cmd_content, "off", 4) == 0) {

			}
		}
		else if (cmd[0] == CMD_COMMAND_LOAD) {

		}
		else { } // Send commands, only text, just take it

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

		if (cmd[0] == CMD_COMMAND_COLOR) {
			color = LCD_MAGENTA;
			strncat(str_to_print, "color ", CMD_STR_LENGTH);
			cmd++;
		}
		else if (cmd[0] == CMD_COMMAND_SCROLL) {
			color = LCD_MAGENTA;
			strncat(str_to_print, "scroll ", CMD_STR_LENGTH);
			cmd++;
		}
		else if (cmd[0] == CMD_COMMAND_LEDS) {
			color = LCD_MAGENTA;
			strncat(str_to_print, "leds ", CMD_STR_LENGTH);
			cmd++;
		}
		else if (cmd[0] == CMD_COMMAND_LOAD) {
			color = LCD_MAGENTA;
			strncat(str_to_print, "load ", CMD_STR_LENGTH);
			cmd++;
		}
		strncat(str_to_print, cmd, CMD_STR_LENGTH);
	}
	str_to_print[CMD_STR_LENGTH - 1] = '\0';

	if ((cmd_decoder->scroll_auto) || ((!cmd_decoder->scroll_auto) && (str_to_print[0] != '\0'))) {
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
