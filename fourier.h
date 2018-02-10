#ifndef FOURIER_H_DEFINED
#define FOURIER_H_DEFINED

#include <assert.h>
#include "custom_math.h"

void generic_slow_discrete_fourier_transform(struct complex_number * in, struct complex_number * out, unsigned int series_length, double scale, double direction);
void fast_discrete_fourier_transform(struct complex_number * in, struct complex_number * out, unsigned int series_length, double scale, double direction);

#endif
