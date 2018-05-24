/**
 * CPU load module (Header)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#ifndef LOAD_H_
#define LOAD_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef struct load_t {
	// Allow the load module to wait passively when the load ms is 0
	xSemaphoreHandle sem;

	// Active load to give
	unsigned int load_ms;
} load_t;

/**
 * Initialise the load module
 * @param load load data memory to initialise
 */
void load_init(load_t * load);

/**
 * Load the CPU
 * @param load Load data to work on
 * @param Either to switch on (TRUE) or off (FALSE)
 */
void load_cpu(load_t * load, unsigned int load_ms);

/**
 * Task for RTOS
 * @param param Load data
 */
void load_task(void * param);

#endif /* LOAD_H_ */
