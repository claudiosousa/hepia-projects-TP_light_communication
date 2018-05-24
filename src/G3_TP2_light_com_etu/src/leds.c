/**
 * LEDs module (Implementation)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include "leds.h"
#include "LPC17xx.h"

#define LEDS_GPIO_LED_ID 0xFF
#define LEDS_WAIT_MS 100

void leds_init(leds_t * leds) {
	LPC_GPIO2->FIODIR = LEDS_GPIO_LED_ID;
	LPC_GPIO2->FIOMASK = ~LEDS_GPIO_LED_ID;
	LPC_GPIO2->FIOPIN = 0;

	vSemaphoreCreateBinary(leds->sem);
	leds->on = false;
	leds->val = 0;
}

void leds_switch_on_off(leds_t * leds, bool on_off) {
	leds->on = on_off;
	xSemaphoreGive(leds->sem);
}

void leds_task(void * param) {
	leds_t * leds = (leds_t*)param;
	portTickType tick_start = xTaskGetTickCount();
	portTickType tick_wait = LEDS_WAIT_MS / portTICK_RATE_MS;

	while (1) {
		// When off, it waits a 'on' command passively
		if (!leds->on) {
			while (!leds->on) {
				xSemaphoreTake(leds->sem, portMAX_DELAY);
			}
			// Restart tick because we have waited an undefined quantity of time
			tick_start = xTaskGetTickCount();
		}

		leds->val <<= 1;
		if (leds->val == 0) {
			leds->val = 1;
		}
		LPC_GPIO2->FIOPIN = leds->val;

		vTaskDelayUntil(&tick_start, tick_wait);
	}
}
