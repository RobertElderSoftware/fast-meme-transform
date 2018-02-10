#ifndef WAVE_H_DEFINED
#define WAVE_H_DEFINED

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "custom_math.h"

struct wav_file_header{
	char RIFF[4];
	uint32_t file_size_bytes;
	char WAVE[4];
	char fmt[4];
	uint32_t fmt_length;
	uint16_t fmt_type;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t sample_rate_bps_channels_div8;
	uint16_t bps_channels;
	uint16_t bits_per_sample;
};

struct run_params;

void print_wav_header(struct wav_file_header *);
char * find_wav_data(char * p, char * end, uint32_t);
int load_wav_period(struct complex_number *, char *, uint32_t, uint32_t, uint32_t, uint32_t, struct wav_file_header *);

#endif
