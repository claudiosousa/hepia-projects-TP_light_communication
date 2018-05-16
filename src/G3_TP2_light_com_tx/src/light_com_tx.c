/*
 * emetteur.c
 *
 * Date: 19.05.2015
 * Auteur: V. Pilloux
 * Description R/B transmitter. Warning: Mylab2 must be powered up too to get enough power !
 */
#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "ledrgb_dma.h"


#define RED 0xFF0000
#define BLUE 0xFF
#define WHITE 0xFFFFFF
#define BLACK 0



uint8_t calc_checksum(uint8_t *s, int len)
{
	int i;
	uint8_t chksum=0;

	for (i=0; i<len; i++)
		chksum^=s[i];
	return chksum;
}

// DO NOT FORGET TO PLUG A SECOND CABLE ON USB CONNECTOR ON MYLAB2 TO GET RIGHT COLORS!

int main (void)
{
	char *str="PTR is funny", cheksum;
	int i;

	// example of checksum calculation:
	cheksum=calc_checksum((uint8_t *)str, strlen(str));

	// example of RGB led using:
	while(1)
	{
		set_rgb_leds_color(0, RED);
		for (i=0; i<10000000; i++);
		set_rgb_leds_color(0, BLUE);
		for (i=0; i<10000000; i++);
	}
}

