
#include "fourier.h"
#include <stdlib.h>
#include <stdio.h>

int do_compression_tests(void);

int main(void){
	srand(50);
	unsigned int i;
	unsigned int tests_remaining = 200;
	while (tests_remaining--){
		//unsigned int num_input = (rand() % 100) + 1;
		unsigned int num_input = 8;
		printf("Running test with %u items.\n", num_input);
		struct complex_number * input = malloc(sizeof(struct complex_number) * num_input);
		struct complex_number * output = malloc(sizeof(struct complex_number) * num_input);
		for(i = 0; i < num_input; i++){
			int rand1 = rand();
			int rand2 = rand();
			int rand3 = rand();
			int rand4 = rand();
			int rand5 = rand();
			int rand6 = rand();
			if(rand1 % 2 == 0 && rand3 != 0){
				input[i].r = (double)rand2 / (double)rand3;
			}else{
				input[i].r = (double)rand2;
			}
			if(rand4 % 2 == 0 && rand6 != 0){
				input[i].i = (double)rand5 / (double)rand6;
			}else{
				input[i].i = (double)rand5;
			}
		}
		struct complex_number * original_input = malloc(sizeof(struct complex_number) * num_input);
		for(i = 0; i < num_input; i++){
			original_input[i] = input[i];
		}
		
		generic_slow_discrete_fourier_transform(input, output, num_input, 1.0, -1);

		generic_slow_discrete_fourier_transform(output, input, num_input, (1.0 / (double)num_input), 1);

		for(i = 0; i < num_input; i++){
			double diff_r = input[i].r - original_input[i].r;
			double diff_i = input[i].i - original_input[i].i;

			double diff_ratio = magnitude(input[i]) / magnitude(original_input[i]);
			printf("i=%u, diff ratio %.30f observed r: %f, orig r: %f\n", i, diff_ratio, input[i].r, original_input[i].r);
			(void)diff_r;
			(void)diff_i;
		}

		free(output);
		free(input);
		free(original_input);
	}
	
	do_compression_tests();
	return 0;
}
