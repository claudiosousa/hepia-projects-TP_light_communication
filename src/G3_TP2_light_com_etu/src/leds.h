/**
 * LEDs module (Header)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct leds_t {
	// Allow the command decoder to send on/off command
	xQueueHandle switch_queue;

	// Whether the leds module is on
	bool on;
	// Bitmask of the leds to be on when the leds module is working
	uint8_t val;
} leds_t;

/**
 * Initialise the leds module
 * @param leds Leds data memory to initialise
 */
void leds_init(leds_t * leds);

/**
 * Switch on or off the leds module
 * @param leds Leds data to work on
 * @param Either to switch on (TRUE) or off (FALSE)
 */
void leds_switch_on_off(leds_t * leds, bool on_off);

/**
 * Task for RTOS
 * @param param LEDs data
 */
void leds_task(void * param);

#endif /* LEDS_H_ */
