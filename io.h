#ifndef MEME_IO_H
#define MEME_IO_H

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct print_buffer{
	/*  Bytes allocated in buffer. */
	unsigned int buffer_size;
	/*  Current position where data can be written to. */
	unsigned int pos;
	char * characters;
};

void init_print_buffer(struct print_buffer * b);
void printfbuff(struct print_buffer * b, const char * fmt, ...);

#endif
