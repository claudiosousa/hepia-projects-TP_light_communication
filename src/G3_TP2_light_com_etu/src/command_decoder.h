/**
 * Command decoder module (Header)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#ifndef COMMAND_DECODER_H_
#define COMMAND_DECODER_H_

/**
 * Initialise the command module
 */
void cmd_init();

/**
 * Send a message to the command module for printing
 * @param msg Message to print
 */
void cmd_send_message(char * msg);

/**
 * Print string from the buffer
 */
void cmd_print();

#endif /* COMMAND_DECODER_H_ */
