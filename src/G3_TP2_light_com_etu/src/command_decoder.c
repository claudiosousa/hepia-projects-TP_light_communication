/**
 * Command decoder module (Implementation)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include "command_decoder.h"
#include <stdbool.h>
#include <string.h>
#include "LPC17xx.h"
#include "uart.h"
#include "lcd.h"

#define LCD_MAGENTA	(LCD_RED | LCD_BLUE)
#define LCD_YELLOW	(LCD_RED | LCD_GREEN)

#define CMD_STR_LENGTH 31
#define CMD_BUFFER_LENGTH 10

typedef struct circular_buffer {
	char buffer[CMD_BUFFER_LENGTH][CMD_STR_LENGTH];
	unsigned int read_index;
	unsigned int write_index;
} circular_buffer;

// Building command from UART
static char cmd_recv[CMD_STR_LENGTH];
static unsigned int cmd_recv_i = 0;

// Buffer for strings to print
static circular_buffer message_print_buffer;
static circular_buffer command_print_buffer;
// Command received from the UART to decode
static circular_buffer command_buffer;

/**
 * Determine whether an element is available for reading from the buffer
 * @param buf Circular buffer for which we want to know
 * @return TRUE if at least one element is available, FALSE otherwise
 */
bool circular_buffer_has(circular_buffer * buf) {
	return buf->read_index != buf->write_index;
}

/**
 * Add a new string to the given buffer
 * @param buf Circular buffer where to add the string
 * @param str String to add
 */
void circular_buffer_add(circular_buffer * buf, char * str) {
	strncpy(buf->buffer[buf->write_index], str, CMD_STR_LENGTH);
	buf->buffer[buf->write_index][CMD_STR_LENGTH - 1] = '\0';
	buf->write_index = (buf->write_index + 1) % CMD_BUFFER_LENGTH;
}

/**
 * Pop the next element in the buffer
 * @param buf Circular buffer from where to get the string
 * @return The pointer to the string currently available
 */
char * circular_buffer_pop(circular_buffer * buf) {
	char * ret = buf->buffer[buf->read_index];
	buf->read_index = (buf->read_index + 1) % CMD_BUFFER_LENGTH;
	return ret;
}

/**
 * Reset the buffer used by the UART for getting command
 */
void cmd_recv_reset() {
	memset(cmd_recv, '\0', CMD_STR_LENGTH);
	cmd_recv_i = 0;
}

/**
 * Callback function of the UART RX.
 * Receive character one by one.
 */
void uart_rx_cmd_callback(int int_status) {
	cmd_recv[cmd_recv_i] = LPC_UART0->RBR;
	// Copy command into command buffer after it is either finished or filled
	if ((cmd_recv[cmd_recv_i] == '\0') || (cmd_recv_i >= (CMD_STR_LENGTH + 1))) {
		circular_buffer_add(&command_buffer, cmd_recv);
		cmd_recv_reset();
	}
	else {
		cmd_recv_i++;
	}
}

void cmd_init() {
	init_lcd();
	clear_screen(LCD_BLACK);
	setup_scroll(0, 10, 0);
	uart0_init_ref(115200, NULL, uart_rx_cmd_callback);

	cmd_recv_reset();
	message_print_buffer.read_index = 0;
	message_print_buffer.write_index = 0;
	command_print_buffer.read_index = 0;
	command_print_buffer.write_index = 0;
	command_buffer.read_index = 0;
	command_buffer.write_index = 0;
}

void cmd_send_message(char * msg) {
	circular_buffer_add(&message_print_buffer, msg);
}

void cmd_print() {
	char * str_to_print = NULL;
	int color = LCD_WHITE;

	if (circular_buffer_has(&message_print_buffer)) {
		str_to_print = circular_buffer_pop(&message_print_buffer);
		color = LCD_WHITE;
	}
	else if (circular_buffer_has(&command_print_buffer)) {
		str_to_print = circular_buffer_pop(&command_print_buffer);
		color = LCD_MAGENTA;
	}
	else {
		str_to_print = "";
	}

	lcd_printf(color, LCD_BLACK, "%s\n", str_to_print);
}
