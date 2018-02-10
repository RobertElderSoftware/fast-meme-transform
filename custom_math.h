#ifndef COMPLEX_H_DEFINED
#define COMPLEX_H_DEFINED

#include <stdint.h>
#include <math.h>

#define PI 3.141592653589
#define NUMBER_E 2.71828

struct complex_number{
	double r;
	double i;
};

struct complex_number e_i_x(double);
struct complex_number make_complex_number(double, double);
struct complex_number complex_add(struct complex_number, struct complex_number);
struct complex_number complex_multiply(struct complex_number, struct complex_number);
double magnitude(struct complex_number);
int is_power_of_two(uint32_t);

#endif
