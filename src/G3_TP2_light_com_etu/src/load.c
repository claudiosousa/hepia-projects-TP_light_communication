/**
 * CPU load module (Implementation)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include "load.h"
#include "LPC17xx.h"

#define TIMER_CLOCK_S 25000000
#define TIMER_CLOCK_MS (TIMER_CLOCK_S / 1000)
#define LOAD_WAIT_MS 20
#define LOAD_MAX 15

void load_init(load_t * load) {
	LPC_TIM0->TC = 0;
	LPC_TIM0->TCR = 1;

	vSemaphoreCreateBinary(load->sem);
	load->load_ms = 0;
}

void load_cpu(load_t * load, unsigned int load_ms) {
	if (load_ms > LOAD_MAX) {
		load_ms = LOAD_MAX;
	}
	load->load_ms = load_ms;
	xSemaphoreGive(load->sem);
}

void load_task(void * param) {
	load_t * load = (load_t*)param;
	portTickType tick_start = xTaskGetTickCount();
	portTickType tick_wait = LOAD_WAIT_MS / portTICK_RATE_MS;
	unsigned int start = 0;

	while (1) {
		if (load->load_ms == 0) {
			while (load->load_ms == 0) {
				xSemaphoreTake(load->sem, portMAX_DELAY);
			}
			// Restart tick because we have waited an undefined quantity of time
			tick_start = xTaskGetTickCount();
		}

		start = LPC_TIM0->TC;
		while ((LPC_TIM0->TC - start) < (load->load_ms * TIMER_CLOCK_MS)) { }

		vTaskDelayUntil(&tick_start, tick_wait);
	}
}
