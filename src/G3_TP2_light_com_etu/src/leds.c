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

	leds->switch_queue = xQueueCreate(10, sizeof(bool));
	leds->on = false;
	leds->val = 0;
}

void leds_switch_on_off(leds_t * leds, bool on_off) {
	xQueueSendToBack(leds->switch_queue, &on_off, portMAX_DELAY);
}

void leds_task(void * param) {
	leds_t * leds = (leds_t*)param;
	portTickType tick_start = xTaskGetTickCount();
	portTickType tick_wait = LEDS_WAIT_MS / portTICK_RATE_MS;
	bool on_off = false;

	while (1) {
		// When on, it just check if there is an 'off' command
		// If not, it continue working
		if (leds->on) {
			xQueueReceive(leds->switch_queue, &on_off, 0);
			if (!on_off) {
				leds->on = false;
			}
		}
		// When off, it waits a 'on' command passively
		else {
			while (!leds->on) {
				xQueueReceive(leds->switch_queue, &on_off, portMAX_DELAY);
				if (on_off) {
					leds->on = true;
				}
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
