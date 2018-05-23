/**
 * Light decoder module (Implementation)
 *
 * Auteur: Claudio Sousa, David Gonzalez
 */

#include "light_decoder.h"
#include <string.h>
#include "command_decoder.h"

// Macro that does a max (taken from lcd.h)
#define MAX(x,y) (((x)>(y))?(x):(y))

// Allow the receiver to wait for the buffer to be filled
static int buf_index;

/**
 * Callback function of the external RGB sensor
 */
void rgb_callback(int buf_idx) {
	buf_index = buf_idx;
	ext_colorsensor_stop_int();	// Stop acquisition to allow the frame saving
}

/**
 * Create a XOR checksum of the given string
 * @param s String to checksum
 * @param len Size of the string
 * @return Checksum of the string
 */
uint8_t calc_checksum(uint8_t * s, int len)
{
	int i;
	uint8_t chksum=0;

	for (i=0; i<len; i++)
		chksum^=s[i];
	return chksum;
}

/**
 *
 */
void build_seq_refs(int * seq_ref_red, int * seq_ref_blue) {
	int bits_ref[16] = { 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1 }; //0x951B
	for (int ibit = 0; ibit < 16 / 2; ibit++) {
		int red = bits_ref[ibit * 2] ? 1 : -1;
		int blue = bits_ref[ibit * 2 + 1] ? 1 : -1;
		for (int i = 0; i < LIGHT_SAMPLES_PER_BIT; i++) {
			seq_ref_red[ibit * LIGHT_SAMPLES_PER_BIT + i] = red;
			seq_ref_blue[ibit * LIGHT_SAMPLES_PER_BIT + i] = blue;
		}
	}
}

/**
 *
 */
int normalize_red(light_decoder_t * light_decoder) {
	//todo:normalize
	int max_red = 0, max_blue = 0;
	for (int i = 0; i < LIGHT_BUF_LEN; i++) {
		max_red = MAX(max_red, light_decoder->buffer[i].red);
		max_blue = MAX(max_blue, light_decoder->buffer[i].blue);
	}

	float ratio = (float) max_blue / max_red;
	for (int i = 0; i < LIGHT_BUF_LEN; i++)
		light_decoder->buffer[i].red *= ratio;
	return MAX(max_red, max_blue);
}

/**
 *
 */
int corr_index(light_decoder_t * light_decoder, int* seq_ref_red, int* seq_ref_blue, int amplitude_offset) {
	long long max_corr_value = 0;
	int max_corr_index;
	long long corr_value;
	int corr_length = 8 * LIGHT_SAMPLES_PER_BIT;

	for (int ibuffer = 0; ibuffer < LIGHT_BUF_LEN / 2; ibuffer++) {
		corr_value = 0;
		for (int i = 0; i < corr_length; i++) {
			corr_value += (light_decoder->buffer[i + ibuffer].red - amplitude_offset)
					* seq_ref_red[i];
			corr_value += (light_decoder->buffer[i + ibuffer].blue - amplitude_offset)
					* seq_ref_blue[i];
		}
		if (corr_value > max_corr_value) {
			max_corr_value = corr_value;
			max_corr_index = ibuffer;
		}
	}

	return max_corr_index;
}

void ld_init(light_decoder_t * light_decoder) {
	ld_start(light_decoder);
}

void ld_start(light_decoder_t * light_decoder) {
	buf_index = -1;
	memset(light_decoder->double_buffer, 0, sizeof(light_decoder->double_buffer));
	if (ext_colorsensor_init_int(light_decoder->double_buffer, LIGHT_BUF_LEN * 2, 1000000 / 625, rgb_callback) != CS_NOERROR) {
		exit(1);
	}
}

/*
 CS_BUF_LEN
 buffer

 function
 res = calc_corr(l, seq_ref, noisy_signal)
 res = seq_ref*noisy_signal(l:l+length(seq_ref)-1)';
 endfunction

 len_corr = length(noisy_signal)-length(seq_ref);
 corr_my = arrayfun(@(l)calc_corr(l, seq_ref, noisy_signal), 1:len_corr);
 */

void ld_process(light_decoder_t * light_decoder) {
	// Wait call of RGB sensor callback
	while (buf_index == -1) {};
	light_decoder->buffer = &light_decoder->double_buffer[buf_index];

	int seq_ref_red[8 * LIGHT_SAMPLES_PER_BIT];
	int seq_ref_blue[8 * LIGHT_SAMPLES_PER_BIT];
	build_seq_refs(seq_ref_red, seq_ref_blue);

	int amplitude_threshold = normalize_red(light_decoder) / 2;
	int sync = corr_index(light_decoder, seq_ref_red, seq_ref_blue, amplitude_threshold);

	char message[LIGHT_STR_LENGTH];
	memset(message, '\0', LIGHT_STR_LENGTH);
	ext_cs_t * data_buffer = &light_decoder->buffer[sync + LIGHT_SAMPLES_PER_BIT * 8 + 2];
	for (int i = 0; i < LIGHT_DATA_LENGTH; i++) {
		unsigned char c = 0;
		for (int ib =7;ib>=1;ib-=2){
			if (data_buffer->red > amplitude_threshold)
				c+= 1<<ib;
			if (data_buffer->blue > amplitude_threshold)
				c+= 1<<(ib-1);
			data_buffer += LIGHT_SAMPLES_PER_BIT;
		}
		message[i] = c;
	}

	if (strncmp(message, "", LIGHT_STR_LENGTH) == 0) {
		strncpy(message, "Nothing received", LIGHT_STR_LENGTH);
		message[LIGHT_STR_LENGTH - 1] = '\0';
	}
	/*else if (calc_checksum((uint8_t *)message, LIGHT_DATA_LENGTH) != 0)
	{
		strncpy(message, "Bad checksum", LIGHT_STR_LENGTH);
		message[LIGHT_STR_LENGTH - 1] = '\0';
	}*/

	cmd_send_message(message);

	// Restart acquisition
	ld_start(light_decoder);
}

void ld_task(void * param) {
	light_decoder_t * light_decoder = (light_decoder_t*)param;

	while (1) {
		ld_process(light_decoder);
	}
}

