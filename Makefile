CUSTOM_FLAGS=-g -Wall -std=c99 -pedantic -Werror -Wno-overlength-strings
CUSTOM_COMPILER=clang

fmt: fmt.o custom_math.o fourier.o wav.o compressor.o io.o 
	@$(CUSTOM_COMPILER) compressor.o io.o fmt.o custom_math.o fourier.o wav.o -o fmt -lm $(CUSTOM_FLAGS)

io.o: io.c
	@$(CUSTOM_COMPILER) -c $< -o $@ $(CUSTOM_FLAGS)

compressor.o: compressor.c
	@$(CUSTOM_COMPILER) -c $< -o $@ $(CUSTOM_FLAGS)

test: tests
	@./tests

tests: test.o custom_math.o fourier.o wav.o compressor.o io.o 
	@$(CUSTOM_COMPILER) compressor.o io.o test.o custom_math.o fourier.o wav.o -o tests -lm $(CUSTOM_FLAGS)

fmt.o: fmt.c
	@$(CUSTOM_COMPILER) -c $< -o $@ $(CUSTOM_FLAGS)

fourier.o: fourier.c
	@$(CUSTOM_COMPILER) -c $< -o $@ $(CUSTOM_FLAGS)

custom_math.o: custom_math.c
	@$(CUSTOM_COMPILER) -c $< -o $@ $(CUSTOM_FLAGS)

test.o: test.c
	@$(CUSTOM_COMPILER) -c $< -o $@ $(CUSTOM_FLAGS)

wav.o: wav.c
	@$(CUSTOM_COMPILER) -c $< -o $@ $(CUSTOM_FLAGS)

clean:
	@rm -f fmt *.o compressor tests
