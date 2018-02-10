
#include "fourier.h"
#include <stdio.h>

void generic_slow_discrete_fourier_transform(struct complex_number * in, struct complex_number * out, unsigned int series_length, double scale, double direction){
	/*
	    scale is usually 1 for forward transform, and 1/n for reverse transform.
	    direction is usually -1 for forward transform, and 1 for reverse transform.
        */
	unsigned int i;
	for(i = 0; i < series_length; i++){
		struct complex_number s = make_complex_number(0, 0);
		unsigned int j;
		for(j = 0; j < series_length; j++){
			s = complex_add(s, complex_multiply(in[j], e_i_x((direction * 2.0 * PI * (double)i * (double)j) / (double)series_length)));
		}
		out[i] = complex_multiply(make_complex_number(scale, 0), s);
	}
}

void bit_reverse_copy(struct complex_number * in, struct complex_number * out, unsigned int series_length, unsigned int log_2_len){
	unsigned int i;
	for(i = 0; i < series_length; i++){
		unsigned int NO_OF_BITS = log_2_len;
		unsigned int reverse_i = 0, j, temp;

		for (j = 0; j < NO_OF_BITS; j++) {
			temp = (i & (1 << j));
			if(temp){
				reverse_i |= (1 << ((NO_OF_BITS - 1) - j));
			}
		}

		out[reverse_i] = in[i];
	}
}

void fast_discrete_fourier_transform(struct complex_number * in, struct complex_number * out, unsigned int series_length, double scale, double direction){
	uint32_t n = series_length;
	uint32_t log_2_len = 0;
	uint32_t num_ones = 0;
	uint32_t s;

	/*  Determine the log base 2 value, and verify that it is a power of two. */
	do {
		n & 0x1 ? num_ones++ : 0;
		if((n >>= 1) > 0){
			log_2_len++;
		}
	} while (n > 0);

	assert(num_ones == 1 && "Must be a power of two!");
	bit_reverse_copy(in, out, series_length, log_2_len);
	for (s = 1; s <= log_2_len; s++){
		uint32_t m = 0x1 << s;
		struct complex_number omega_m = e_i_x((direction * (double)2 * PI) / (double)m);
		uint32_t k;
		for (k = 0; k < series_length; k += m){
			struct complex_number omega = make_complex_number(1, 0);
			uint32_t j;
			for (j = 0; j < (m / 2); j++){
				struct complex_number t = complex_multiply(omega,  out[k + j + m/2]);
				struct complex_number u = out[k + j];
				out[k + j] = complex_add(u, t);
				out[k + j + m / 2] = complex_add(u, complex_multiply(make_complex_number(-1,0), t));
				omega = complex_multiply(omega, omega_m);
			}
		}
	}

	for(s = 0; s < series_length; s++){
		out[s] = complex_multiply(make_complex_number(scale, 0), out[s]);
	}
}
