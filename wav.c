
#include "wav.h"

void print_wav_header(struct wav_file_header * h){
	printf("#  Read .wav file header:\n");
	printf("#  RIFF: %c%c%c%c\n", h->RIFF[0], h->RIFF[1], h->RIFF[2], h->RIFF[3]);
	printf("#  file_size_bytes: %u\n", h->file_size_bytes);
	printf("#  WAVE: %c%c%c%c\n", h->WAVE[0], h->WAVE[1], h->WAVE[2], h->WAVE[3]);
	printf("#  fmt: %c%c%c%c\n", h->fmt[0], h->fmt[1], h->fmt[2], h->fmt[3]);
	printf("#  fmt_length: %u\n", h->fmt_length);
	printf("#  fmt_type: %u\n", h->fmt_type);
	printf("#  num_channels: %u\n", h->num_channels);
	printf("#  sample_rate: %u\n", h->sample_rate);
	printf("#  sample_rate_bps_channels_div8: %u\n", h->sample_rate_bps_channels_div8);
	printf("#  bps_channels: %u\n", h->bps_channels);
	printf("#  bits_per_sample: %u\n", h->bits_per_sample);
}

char * find_wav_data(char * p, char * end, uint32_t verbose){
	while(
		!(
			((char *)p)[0] == 'd' &&
			((char *)p)[1] == 'a' &&
			((char *)p)[2] == 't' &&
			((char *)p)[3] == 'a'
		) &&
		p < end
	){
		if(verbose){
			printf("#  Found %c%c%c%c\n", ((char*)p)[0], ((char*)p)[1], ((char*)p)[2], ((char*)p)[3]);
		}
		p += 1;
	}
	if(p < end){
		if(verbose){
			printf("#  Found data section at address %p\n", (void*)p);
		}
		return p;
	}else{
		if(verbose){
			printf("#  Did not find data section.\n");
		}
		return (char *)0;
	}
}

int load_wav_period(struct complex_number * time, char * wav_data, uint32_t sample_offset, uint32_t period_number, uint32_t samples_per_period, uint32_t total_samples_mono_channel, struct wav_file_header * header){
	/*
		time:  A pointer to the time domain data we want to populate.
		wav_data:  A pointer to the very first sample of the audio data not considering the sample offset starting point.
		sample_offset:  This is an integer representing the integer offset of the first sample we want to consider (important when start_time != 0).
		period_number:  An integer representing the current period number.
		samples_per_period:  An integer representing the number of samples in one period (considered as if there were only one channel).
		total_samples_mono_channel:  An integer representing the total number of samples in entire wav file (considered as if there were only one channel).
	*/
	uint32_t begin = sample_offset + samples_per_period * period_number;
	uint32_t end = sample_offset + samples_per_period * (period_number + 1);
	uint32_t i = begin;
	if(begin < total_samples_mono_channel){
		/*  Now consider padding. */
		for(i = begin; (i < end) && (i < total_samples_mono_channel); i++){
			/*  Only considers data from one channel.  TODO:  Consider the other channel if present. */
			unsigned int data_index = i * header->num_channels;
			double v;
			switch(header->bits_per_sample){
				case 8:{
 					/*  It turns out that 8-bit .wav formats are usually unsigned centered at 0x80 while higher bits are signed. */
 					v = (double)((int32_t)(((uint8_t*)wav_data)[data_index]) - 0x80);
					break;
				}case 16:{
 					v = (double)((int16_t*)wav_data)[data_index];
					break;
				}case 32:{
 					v = (double)((int32_t*)wav_data)[data_index];
					break;
				}default:{
					assert(0);
				}
			}
			time[i - begin].r = v;
			time[i - begin].i = 0;
		}
		/*  Possibly more data to process. */
		return 0;
	}else{
		return 1;
	}
}
