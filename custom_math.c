#include "custom_math.h"

struct complex_number e_i_x(double d){
	struct complex_number c;
	c.r = cos(d);
	c.i = sin(d);
	return c;
}

struct complex_number make_complex_number(double r, double i){
	struct complex_number c;
	c.r = r;
	c.i = i;
	return c;
}

struct complex_number complex_add(struct complex_number a, struct complex_number b){
	struct complex_number c;
	c.r = a.r + b.r;
	c.i = a.i + b.i;
	return c;
}

struct complex_number complex_multiply(struct complex_number a, struct complex_number b){
	struct complex_number c;
	c.r = (a.r * b.r) - (a.i * b.i);
	c.i = (a.r * b.i) + (a.i * b.r);
	return c;
}

double magnitude(struct complex_number c) {
	return sqrt(pow(c.r,2) + pow(c.i,2));
}

int is_power_of_two(uint32_t i){
	while(!(i & 0x1) && i != 0){
		i >>= 1;
	}
	if(i == 1){
		return 1;
	}else{
		return 0;
	}
}
