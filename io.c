
#include "io.h"

void init_print_buffer(struct print_buffer * b){
	b->buffer_size = 1;
	b->pos = 0;
	b->characters = (char *)malloc(1);
	b->characters[0] = '\0';
}

unsigned int get_count_snprintf(const char * fmt, va_list v){
	/*  Figure out how big it is first. */
	return vsnprintf((char *)0, 0, fmt, v);
}

void printfbuff(struct print_buffer * b, const char * fmt, ...){
	va_list t1;
	va_list t2;
	va_start(t1, fmt);
	va_start(t2, fmt);
	
	/*  Make sure we have room. */
	unsigned int size = get_count_snprintf(fmt, t1);
	b->buffer_size += size;
	b->characters = realloc(b->characters, b->buffer_size);
	
	/*  Write the actual data. */
	vsnprintf(&b->characters[b->pos], size + 1, fmt, t2);
	b->pos += size;

	va_end(t1);
	va_end(t2);
}
