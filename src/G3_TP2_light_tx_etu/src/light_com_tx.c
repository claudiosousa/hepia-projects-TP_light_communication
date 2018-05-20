/**
 * R/B transmitter.
 * Note: Mylab2 must be powered up too to get enough power !
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */
#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "LPC17xx.h"
#include "ledrgb_dma.h"

#define RED 0xFF0000
#define BLUE 0xFF
#define WHITE 0xFFFFFF
#define BLACK 0

#define TIMER_CLOCK_S 25000000
#define TIMER_CLOCK_MS (TIMER_CLOCK_S / 1000)
#define TIMER_WAIT_MS 8
#define ASCII_SIZE 8
#define START_OF_FRAME "\x95\x1B"
#define MESSAGE "PTR is funny"
#define SOF_SIZE 2
#define MSG_SIZE 15
#define CHECKSUM_SIZE 1
#define FRAME_SIZE (SOF_SIZE + MSG_SIZE + CHECKSUM_SIZE)
#define STR_SIZE (FRAME_SIZE + 1)

// Allow the main code to be notified of the timer due date
bool sleep_done = false;

/**
 * Create a XOR checksum of the given string
 * @param s String to checksum
 * @param len Size of the string
 * @return Checksum of the string
 */
uint8_t calc_checksum(uint8_t *s, int len)
{
	int i;
	uint8_t chksum=0;

	for (i=0; i<len; i++)
		chksum^=s[i];
	return chksum;
}

/**
 * Timer that handler timing of the sender
 */
void TIMER0_IRQHandler(void)
{
	LPC_TIM0->IR = 1;
	sleep_done = true;
}

/**
 * Main function
 */
int main (void)
{
	// Timer setup
	LPC_TIM0->MCR = 3;
	LPC_TIM0->MR0 = TIMER_WAIT_MS - 1;
	LPC_TIM0->PR = TIMER_CLOCK_MS - 1;
	LPC_TIM0->TC = 0;
	NVIC_EnableIRQ(TIMER0_IRQn);
	LPC_TIM0->TCR = 1;
	// Message setup
	// Concatenate the parts and be sure to have a valid string
	char str[STR_SIZE];
	char msg[MSG_SIZE];
	memset(str, '\0', STR_SIZE);
	memset(msg, '\0', MSG_SIZE);
	strncpy(msg, MESSAGE, MSG_SIZE);
	msg[MSG_SIZE - 1] = '\0';
	strncpy(str, START_OF_FRAME, SOF_SIZE);
	strncpy(str + 2, msg, MSG_SIZE);
	str[FRAME_SIZE - 2] = '\0';
	str[FRAME_SIZE - 1] = calc_checksum((uint8_t *)msg, MSG_SIZE);
	str[FRAME_SIZE] = '\0';
	// Counters, one for the string and one for the bits
	unsigned char c = 0;
	unsigned int i = 0;
	// Temporary variables
	uint32_t color = 0;

	while(1)
	{
		// Wait for the timer to tell us that the date has come
		while (sleep_done == false) {
			__WFI();
		}
		sleep_done = false;

		// Get current char bits, RED for odd, BLUE for even
		// From MSB to LSB
		color = (str[c] & (1 << (ASCII_SIZE - 1 - i)) ? RED : 0) |
				(str[c] & (1 << (ASCII_SIZE - 2 - i)) ? BLUE : 0);
		set_rgb_leds_color(0, color);

		// Increment bit counter, two by two
		i = (i + 2) % ASCII_SIZE;
		// Increment char counter only if bit counter has reset
		if (i == 0) {
			c = (c + 1) % FRAME_SIZE;
		}
	}
}

