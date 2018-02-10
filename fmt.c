#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "fourier.h"
#include "wav.h"
#include "compressor.h"

enum command_types {
	ALL_SOX = 0,
	UBUNTU_APLAY = 1,
	COREAUDIO_SOX = 2,
	COREAUDIO_ALSA = 3
};

struct biquad_filter_details{
	char type;
	double Q;
	double F_c;
};

struct run_params{
	uint32_t display_help_and_exit;
	int32_t infile_argv_index;
	uint32_t command_type;
	uint32_t verbose;
	uint32_t include_imaginary;
	uint32_t enable_endpoint_discontinuities;
	uint32_t full_power_spectrum;
	uint32_t use_compression;
	uint32_t apply_hann_window;
	uint32_t float_precision;
	double sample_time_period;
	double volume;
	double threshold;
	double starttime;
	double endtime;
	double pitch;
	struct biquad_filter_details * bfds;
	uint32_t num_biquad_filters;
};

int load_input_file(char * argv[], signed int infile_argv_index, char ** buffer, uint32_t * bytes_used){
        FILE *f = NULL;
        int c = 0;
        uint32_t buffer_size = 1;
        *buffer = (char *)malloc(1);

	if(infile_argv_index > 0){
		if(!(f = fopen(argv[infile_argv_index], "rb"))){
			printf("Failed to open file %s for read.\n", argv[infile_argv_index]);
			return 1;
		}
        }else{
		f = stdin;
        }

        while (c != EOF) {
                c = getc(f);
                if(c == EOF)
                        break;
                if(!(*bytes_used < buffer_size)){
			buffer_size *= 2;
			*buffer = realloc(*buffer, buffer_size * sizeof(char));
                }
		(*buffer)[*bytes_used] = c;
                (*bytes_used)++;
        }
        fclose(f);
        return 0;
}

int is_bigger_than_threshold(double th, double r, double i, uint32_t include_imaginary){
	if(fabs(r) > th){
		return 1;
	}else if(include_imaginary && fabs(i) > th){
		return 1;
	}else{
		return 0;
	}
}

void apply_biquad_filter(struct complex_number * x, unsigned int len, struct biquad_filter_details * bfd, double f_s, unsigned int verbose){
	struct complex_number * y = malloc(sizeof(struct complex_number) * (len + 2));
	double K = tan(PI * (bfd->F_c / f_s));
	double Q = bfd->Q;

	/*  Coefficients from quadratic equations resulting from bi-linear transform. */
	double a1 = (2 * (K*K - 1)) / (K*K + (K/Q) + 1);
	double a2 = (K*K - (K/Q) + 1) / (K*K + (K/Q) + 1);
	double b0_lowpass = (K*K) / (K*K + K/Q + 1);
	double b0_highpass = 1 / (K*K + K/Q + 1);
	double b0 = bfd->type == 'l' ? b0_lowpass : b0_highpass;
	double b1_lowpass = 2 * b0;
	double b1_highpass = -2 * b0;
	double b1 = bfd->type == 'l' ? b1_lowpass : b1_highpass;
	double b2 = b0;
	double wn0 = 0;
	double wn1 = 0;
	double wn2 = 0;
	unsigned int i;
	if(verbose){
		printf("#  Applying a biquad filter with type=%c Q=%f, F_c=%f, F_s=%f\n", bfd->type, Q, bfd->F_c, f_s);
		printf("#  a1=%f\n", a1);
		printf("#  a2=%f\n", a2);
		printf("#  b0=%f\n", b0);
		printf("#  b1=%f\n", b1);
		printf("#  b2=%f\n", b2);
	}

	/*  Digital biquad filter, direct form 2. */
	for(i = 0; i < len + 2; i++){
		wn0 = x[i].r -(a1 * wn1) - (a2 * wn2);
		y[i].r = b0 * wn0 + b1 * wn1 + b2 * wn2;
		/*  Update registers. */
		wn2 = wn1;
		wn1 = wn0;
	}

	for(i = 0; i < len; i++){
		/*  Move filtered time domain signal back, and discard first 2 values. */
		x[i].r = y[i + 2].r;
	}
	free(y);
}

void apply_hann_window(struct complex_number * in, unsigned int len){
	unsigned int i;
	for(i = 0; i < len; i++){
		double hann = 0.5 * (1.0 - cos((2.0 * PI * (double)i) / (double)(len - 1.0)));
		in[i].r *= hann;
		in[i].i *= hann;
	}
}

void do_decoder_output(unsigned int samples_per_period_per_channel_pow_2, unsigned int samples_per_period_per_channel, unsigned int num_required_periods, unsigned int num_samples_last_period, unsigned int sample_rate, struct run_params * p){
	struct print_buffer fmt_string;
	init_print_buffer(&fmt_string);

	printfbuff(&fmt_string, " | awk '");
	printfbuff(&fmt_string, "BEGIN{");
		printfbuff(&fmt_string, "L=%u;", samples_per_period_per_channel);
		printfbuff(&fmt_string, "N=%u;", samples_per_period_per_channel_pow_2);
		printfbuff(&fmt_string, "for(n=0;n<L;n+=1)");
			printfbuff(&fmt_string, "o[n+1]=0;");
	printfbuff(&fmt_string, "}");
	printfbuff(&fmt_string, "{");
	printfbuff(&fmt_string, "l=split($1,t,\"x\");");
	printfbuff(&fmt_string, "for(j=0;j<l;j+=1){");
	printfbuff(&fmt_string, "split(t[j+1],p,\"=\");");
	if(p->include_imaginary){
		printfbuff(&fmt_string, "split(p[2],z,\";\");");
		printfbuff(&fmt_string, "x[j+1]=z[1];");
		printfbuff(&fmt_string, "y[j+1]=z[2];");
	}else{
		printfbuff(&fmt_string, "x[j+1]=p[2];");
	}
	printfbuff(&fmt_string, "f[j+1]=p[1];");
	printfbuff(&fmt_string, "}");
	printfbuff(&fmt_string, "M=(NR==%u?%u:2*L);", num_required_periods, num_samples_last_period);
	printfbuff(&fmt_string, "for(n=0;n<M;n+=1){");
		printfbuff(&fmt_string, "R=0;");
		if(p->include_imaginary){
			printfbuff(&fmt_string, "I=0;");
		}
		printfbuff(&fmt_string, "for(j=0;j<l;j+=1){");
			printfbuff(&fmt_string, "a=cos(2*3.141592*f[j+1]*(n/N)*%f);", p->pitch);
			if(p->include_imaginary){
				printfbuff(&fmt_string, "b=sin(2*3.141592*f[j+1]*(n/N)*%f);", p->pitch);
			}
			if(p->include_imaginary){
				printfbuff(&fmt_string, "R+=(x[j+1]*a)-(y[j+1]*b);");
				printfbuff(&fmt_string, "I+=(x[j+1]*b)+(y[j+1]*a);");
			}else{
				printfbuff(&fmt_string, "R+=(x[j+1]*a);");
			}
		printfbuff(&fmt_string, "}");
		printfbuff(&fmt_string, "if(n<L){");
			/*  Do a smoother transition when including the addition. */
			printfbuff(&fmt_string, "R=(R*(n/L))+(o[n+1]*((L-n)/L));");
			if(p->volume != 1){
				/*  Multiply by max 16 bit signed int to scale. */
				printfbuff(&fmt_string, "R*=%f;", p->volume);
			}
			printfbuff(&fmt_string, "R=int(R*32767);");
			/*  The integer comparisons are used to print negative signed 16 bit integers as though they were unsigned 16/32/64 bit integers. */
			printfbuff(&fmt_string, "printf(\"%%04X\\n\",R==0?0:(R>0?(R>32767?32767:R):(R<-32768?32768:65536+R)));");

		printfbuff(&fmt_string, "}else if(n<2*L){");
			if(!p->enable_endpoint_discontinuities){
				printfbuff(&fmt_string, "o[n-L+1]=R;");
			}
		printfbuff(&fmt_string, "}");
	printfbuff(&fmt_string, "}");
	printfbuff(&fmt_string, "}' | tac | tac | xxd -r -p | ");

	if(p->command_type == ALL_SOX){
		printfbuff(&fmt_string, "sox -q -r %u -b 16 -B -e signed -c 1 -t raw  - -t `sox -h|grep DRIVERS|sed 's/^.*: \\([^ ]\\+\\).*$/\\1/g'`", sample_rate);
	}else if(p->command_type == UBUNTU_APLAY){
		printfbuff(&fmt_string, "aplay -q -c 1 -f S16_BE -r %u ", sample_rate);
	}else if(p->command_type == COREAUDIO_SOX){
		printfbuff(&fmt_string, "sox -q -r %u -b 16 -B -e signed -c 1 -t raw  - -t coreaudio", sample_rate);
	}else if(p->command_type == COREAUDIO_ALSA){
		printfbuff(&fmt_string, "sox -q -r %u -b 16 -B -e signed -c 1 -t raw  - -t alsa", sample_rate);
	}else{
		assert("Unknown play value." && 0);
	}

	printfbuff(&fmt_string, "\n");

	printf("%s", fmt_string.characters);
	free(fmt_string.characters);
}


int parse_an_argument(int argc, char * argv[], struct run_params * p, unsigned int * current_arg){
	if(!strcmp(argv[*current_arg], "--help") || !strcmp(argv[*current_arg], "-h")){
		p->display_help_and_exit = 1;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--verbose") || !strcmp(argv[*current_arg], "-v")){
		p->verbose = 1;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--file") || !strcmp(argv[*current_arg], "-f")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->infile_argv_index = *current_arg;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--disable-compression") || !strcmp(argv[*current_arg], "-u")){
		p->use_compression = 0;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--enable-hann-window") || !strcmp(argv[*current_arg], "-w")){
		p->apply_hann_window = 1;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--enable-endpoint-discontinuities") || !strcmp(argv[*current_arg], "-b")){
		p->enable_endpoint_discontinuities = 1;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--include-imaginary") || !strcmp(argv[*current_arg], "-i")){
		/* When the imaginary parts aren't included the amplitude of the output is only half. */
		p->volume /= 2;
		p->include_imaginary = 1;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--command-type") || !strcmp(argv[*current_arg], "-k")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->command_type = strtoul(argv[*current_arg], (char **)0, 10);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--full-power-spectrum") || !strcmp(argv[*current_arg], "-a")){
		p->full_power_spectrum = 1;
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--volume") || !strcmp(argv[*current_arg], "-m")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->volume = strtod(argv[*current_arg], (char **)0);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--threshold") || !strcmp(argv[*current_arg], "-t")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->threshold = strtod(argv[*current_arg], (char **)0);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--start-time") || !strcmp(argv[*current_arg], "-s")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->starttime = strtod(argv[*current_arg], (char **)0);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--end-time") || !strcmp(argv[*current_arg], "-e")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->endtime = strtod(argv[*current_arg], (char **)0);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--sample-time-period") || !strcmp(argv[*current_arg], "-p")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->sample_time_period = strtod(argv[*current_arg], (char **)0);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--float-prevision") || !strcmp(argv[*current_arg], "-n")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->float_precision = strtoul(argv[*current_arg], (char **)0, 10);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--pitch") || !strcmp(argv[*current_arg], "-c")){
		(*current_arg) += 1;
		assert(*current_arg < argc);
		p->pitch = strtod(argv[*current_arg], (char **)0);
		(*current_arg) += 1;
	}else if(!strcmp(argv[*current_arg], "--filters") || !strcmp(argv[*current_arg], "-l")){
		(*current_arg) += 1;
		while(*current_arg < argc && argv[*current_arg][0] != '-'){
			char type = argv[*current_arg][0];
			assert(type == 'l' || type == 'h');
			(*current_arg) += 1;
			if(*current_arg < argc && argv[*current_arg][0] != '-'){
				double Q = strtod(argv[*current_arg], (char **)0);
				(*current_arg) += 1;
				if(*current_arg < argc && argv[*current_arg][0] != '-'){
					unsigned long F_c = strtoul(argv[*current_arg], (char **)0, 10);
					(*current_arg) += 1;
					p->num_biquad_filters++;
					p->bfds = realloc(p->bfds, sizeof(struct biquad_filter_details) * p->num_biquad_filters);
					p->bfds[p->num_biquad_filters-1].type = type;
					p->bfds[p->num_biquad_filters-1].Q = Q;
					p->bfds[p->num_biquad_filters-1].F_c = (double)F_c;
				}else{
					printf("TODO: Argument Error\n");
					return 1;
				}
			}else{
				printf("TODO: Argument Error\n");
				return 1;
			}
		}
	}else{
		printf("Don't know what to do with '%s'... ignoring this part of the parameters.  Specify '-f' for the file, or '-h' for help.\n", argv[*current_arg]);
		return 1;
		*current_arg = argc;
	}
	return 0;
}

int populate_run_params(int argc, char * argv[], struct run_params * p){
	unsigned int current_arg = 1;
	/*  Default params. */
	p->command_type = ALL_SOX;
	p->pitch = 1.0;
	p->display_help_and_exit = 0;
	p->float_precision = 0;
	p->verbose = 0;
	p->infile_argv_index = -1;
	p->full_power_spectrum = 0;
	p->include_imaginary = 0;
	p->use_compression = 1;
	p->apply_hann_window = 0;
	p->volume = 2;
	p->threshold = 300;
	p->sample_time_period = 0.05;
	p->starttime = 0.0;
	p->endtime = 0.0;
	p->bfds = (struct biquad_filter_details *)0;
	p->num_biquad_filters = 0;

	while(current_arg < argc){
		if(parse_an_argument(argc, argv, p, &current_arg)){
			return 1;
		}
	}

	if(p->verbose){
		unsigned int i;
		printf("#  Here are the run-time arguments that will be used:\n");
		printf("#  verbose: %s\n", p->verbose ? "Yes" : "No");
		printf("#  include_imaginary: %s\n", p->include_imaginary ? "Yes" : "No");
		printf("#  use_compression: %s\n", p->use_compression ? "Yes" : "No");
		printf("#  full_power_spectrum: %s\n", p->full_power_spectrum ? "Yes" : "No");
		printf("#  apply_hann_window: %s\n", p->apply_hann_window ? "Yes" : "No");
		printf("#  float_precision: %u\n", p->float_precision);
		printf("#  volume: %f\n", p->volume);
		printf("#  threshold: %f\n", p->threshold);
		printf("#  sample_time_period: %f\n", p->sample_time_period);
		printf("#  starttime: %f\n", p->starttime);
		printf("#  endtime: %f (0 means end of clip)\n", p->endtime);
		printf("#  pitch: %f\n", p->pitch);
		printf("#  Found %d biquad filter details\n", p->num_biquad_filters);
		for(i = 0; i < p->num_biquad_filters; i++){
			printf("#    Biquad filter #%d)  Q = %f, F_c = %f\n", i, p->bfds[i].Q, p->bfds[i].F_c);
		}
	}
	return 0;
}


int analyze_audio_and_build_command(struct run_params *, struct wav_file_header *, char *);

int analyze_audio_and_build_command(struct run_params * p, struct wav_file_header * header, char * data){
	uint32_t done = 0;
	uint32_t current_period = 0;
	struct complex_number * time;
	struct complex_number * freq;
	struct print_buffer uncompressed;
	struct print_buffer compressed;
	double samples_per_second_per_channel = header->sample_rate;
	/*  Note that this will have a loss due to conversion to integer so
		sample_time_period * samples_per_second_per_channel >= samples_per_period_per_channel / samples_per_second_per_channel
	*/
	uint32_t samples_per_period_per_channel = p->sample_time_period * samples_per_second_per_channel;

	uint32_t * samples_size_loc = (uint32_t*)(data + 4);
	char * actual_data_loc = data + 8;
	uint32_t total_samples_all_channels = *samples_size_loc / (header->bits_per_sample/8);
	uint32_t total_samples_mono_channel = total_samples_all_channels / header->num_channels;
	double clip_end_time = (double)total_samples_mono_channel / (double)header->sample_rate;
	double duration = (p->endtime == 0.0 ? clip_end_time : p->endtime) - p->starttime;

	uint32_t clip_total_samples_mono_channel = duration * header->sample_rate;
	uint32_t sample_offset = p->starttime * header->sample_rate;

	uint32_t num_required_periods = (clip_total_samples_mono_channel / samples_per_period_per_channel) + ((clip_total_samples_mono_channel % samples_per_period_per_channel) > 0 ? 1 : 0);
	uint32_t samples_per_period_per_channel_pow_2 = samples_per_period_per_channel;
	uint32_t num_samples_last_period = (clip_total_samples_mono_channel - ((clip_total_samples_mono_channel / samples_per_period_per_channel) * samples_per_period_per_channel));
	/*  This assertion should never fire: */
	assert(total_samples_all_channels % header->num_channels == 0);
	/*  Adjust endtime. */
	p->endtime = p->endtime == 0.0 ? clip_end_time : p->endtime;
	if(!(p->endtime <= clip_end_time)){
		printf("The end time can't be after the end of the entire file.\n");
	}
	if(!(p->starttime <= p->endtime)){
		printf("The start time should be before the end time.\n");
	}
	if(p->verbose){
		printf("#  Output clip will start at %f, end at %f. Duration is %f\n", p->starttime, p->endtime, duration);
	}

	while(!is_power_of_two(samples_per_period_per_channel_pow_2)){
		samples_per_period_per_channel_pow_2++;
	}
	/*  Prevent aliasing due to circular convolution in overlap add when the default number of samples is already (or is close to) a power of two. */
	samples_per_period_per_channel_pow_2 *= 2;

	if(p->verbose){
		printf("#  num_required_periods: %u\n", num_required_periods);
		printf("#  num_samples_last_period: %u\n", num_samples_last_period);
		printf("#  Sample size in bytes: %u\n", *samples_size_loc);
		printf("#  Samples per period per channel : %u\n", samples_per_period_per_channel);
		printf("#  Samples per period per channel power 2 adjusted: %u\n", samples_per_period_per_channel_pow_2);
		printf("#  Total input samples mono channel: %u\n", total_samples_mono_channel);
		printf("#  Total input samples all channels: %u\n", total_samples_all_channels);
		printf("#  Num Required sample periods: %u\n", num_required_periods);
		printf("#  Samples Per Period: %u\n", samples_per_period_per_channel);
	}

	time = malloc(sizeof(struct complex_number) * samples_per_period_per_channel_pow_2);
	freq = malloc(sizeof(struct complex_number) * samples_per_period_per_channel_pow_2);

	init_print_buffer(&uncompressed);
	init_print_buffer(&compressed);

	while(!done){
		unsigned int i;
		unsigned int num_matches = 0;
		int last_one = -1;
		/*  Initialize everything to zero. */
		for(i = 0; i < samples_per_period_per_channel_pow_2; i++){
			time[i].r = 0.0;
			time[i].i = 0.0;
			freq[i].r = 0.0;
			freq[i].i = 0.0;
		}

		done = load_wav_period(&time[0], actual_data_loc, sample_offset, current_period, samples_per_period_per_channel, total_samples_mono_channel, header);

		/*  Apply biquad filters to attenuate unwanted frequencies. */
		for(i = 0; i < p->num_biquad_filters; i++){
			apply_biquad_filter(&time[0], samples_per_period_per_channel, &p->bfds[i], (double)header->sample_rate, p->verbose);
		}
		/*  Apply a window function to the data to address issues with endpoint discontinuity.  */
		if(p->apply_hann_window){
			apply_hann_window(&time[0], samples_per_period_per_channel);
		}

		fast_discrete_fourier_transform(
			&time[0],
			&freq[0],
			samples_per_period_per_channel_pow_2,
			(1.0 / (double)samples_per_period_per_channel_pow_2),
			-1
		);

		for(i = 0; i < samples_per_period_per_channel_pow_2; i++){
			struct complex_number c = freq[i];
			if(is_bigger_than_threshold(p->threshold, c.r, c.i, p->include_imaginary)){
				num_matches++;
				last_one = i;
			}
		}

		if(num_matches == 0){
			printfbuff(&uncompressed, "0=%s", (p->include_imaginary ? "0;0" : "0"));
		}else{
			unsigned int upper_bound = p->full_power_spectrum ? samples_per_period_per_channel_pow_2 : (samples_per_period_per_channel_pow_2 / 2);
			for(i = 0; i < upper_bound; i++){
				struct complex_number c = freq[i];
				if(is_bigger_than_threshold(p->threshold, c.r, c.i, p->include_imaginary)){
					double denominator = (double)(1 << (header->bits_per_sample-1));
					if(p->include_imaginary){
						printfbuff(&uncompressed, "%u=%.*e;%.*e", i, p->float_precision, c.r / denominator, p->float_precision, c.i / denominator);
					}else{
						printfbuff(&uncompressed, "%u=%.*e", i, p->float_precision, c.r / denominator);
					}
					if(i != last_one){
						printfbuff(&uncompressed, "x");
					}
				}
			}
		}
		if(!done){
			if(p->use_compression){
				printfbuff(&uncompressed, "n");
			}else{
				printfbuff(&uncompressed, "\\n");
			}
		}
		current_period++;
		if(current_period >= num_required_periods){
			done = 1;
		}
	}
	current_period++;

	if(p->use_compression){
		do_compression(&uncompressed, &compressed, p->verbose);
		printf("%s", compressed.characters);
	}else{
		printf("echo -en \"%s\"", uncompressed.characters);
	}

	free(uncompressed.characters);
	free(compressed.characters);

	do_decoder_output(samples_per_period_per_channel_pow_2, samples_per_period_per_channel, num_required_periods, num_samples_last_period, header->sample_rate, p);

	free(time);
	free(freq);
	free(p->bfds);
	return 0;
}

int main(int argc, char * argv[]){
	struct run_params p;
	char * infile_bytes = (char *)0;
	uint32_t infile_bytes_size = 0;
	struct wav_file_header * header = (struct wav_file_header *)0;
	char * end;
	char * data;
	if(populate_run_params(argc, argv, &p)){
		return 1;
	}

	if(p.display_help_and_exit){
		printf("The Fast Meme Transform Tool\n");
		printf("\n");
		printf("Note that if you run the tool without any -f parameter, it will wait for input to come from standard in.\n");
		printf("\n");
		printf("Displaying command flags:\n");
		printf("\t-k, --command-type\n\t\tA single number that will determine which type of command will be generated to play the final audio.  You may need to use this option to generate a command that will play in your operating system without installing additional tools.  0 = use sox to automatically detect audio driver (Tested to work on Ubuntu 16), 1 = use aplay (tested on Ubuntu 16) 2 = sox with coreaudio (Probably works on a Mac, but I don't own one so I can't test this), 3 = sox with alsa.\n");
		printf("\t-h, --help\n\t\tDisplay this help menu.\n");
		printf("\t-t, --threshold\n\t\tA floating point number that describes the cutoff power level, below which any frequency with less than this quantity of power will not be included in the output.  If you make this number high, the output will be small, but the audio will sound terrible.  If you make this value low, the output will be huge and slow to decode, but the result will sound better. If you make this value large, the output will be small but the sound quality will decrease.\n");
		printf("\t-s, --start-time\n\t\tThe start time in seconds where the output audio clip will begin.\n");
		printf("\t-e, --end-time\n\t\tThe end time in seconds where the output audio clip will end.  An end time of zero indicates end of the clip.\n");
		printf("\t-v, --verbose\n\t\tBe verbose.\n");
		printf("\t-f, --file\n\t\tThe file name of the input PCM .wav file to be processed.\n");
		printf("\t-u, --disable-compression\n\t\tDisable compression of the output Fourier coefficients.\n");
		printf("\t-w, --enable-hann-window\n\t\tEnable the application of a Hann window to each audio sample period.\n");
		printf("\t-i, --include-imaginary\n\t\tInclude the imaginary parts of the coefficients of the Fourier transform output.\n");
		printf("\t-a, --full-power-spectrum\n\t\tInclude the entire range of frequencies from the Fourier transform output instead of just including up to N/2.\n");
		printf("\t-m, --volume\n\t\tA floating point number describing the output volume.\n");
		printf("\t-b, --enable-endpoint-discontinuities\n\t\tEnable endpoint discontinuities that occur at the ends of sampling intervals (see -p flag). Enable this option if you want the presence of annoying high-frequency popping noises in your audio.\n");
		printf("\t-p, --sample-time-period\n\t\tA floating point number that describes how much long each sub-sample will be when the audio is broken up into small sections to have a Fourier transform applied to it.\n");
		printf("\t-n, --float-precision\n\t\tAn integer describing how many digits should be printed after the decimal after for each Fourier coefficient that appears in the output.\n");
		printf("\t-l, --filters\n\t\tA list of length 3*n that one or more sequence of a single character followed by two numbers that describe the parameters of either a low pass or high pass biquad filter.  Each biquad filter will be applied one after another on the waveform before the Fourier transform is calculated.  For example, using '-l l 5 500' will apply a lowpass filter with a Q value of 5, and a cutoff frequency of 500.  The extra character 'l' or 'h' is for low pass and high pass respectively.\n");
		printf("\t-c, --pitch\n\t\tA positive number that can be used to either shift the frequency up or down.  Set this value greater than one to increase the pitch, and set it between 0 and 1 to decrease the pitch.\n");
		printf("\n");
		printf("Copyright 2018 Robert Elder Software Inc. available freely under MIT license.\n");
		return 0;
	}


	if(load_input_file(argv, p.infile_argv_index, &infile_bytes, &infile_bytes_size)){
		return 1;
	}

	header = (struct wav_file_header *)infile_bytes;

	if(p.verbose){
		print_wav_header(header);
	}

	if(header->fmt_type != 1){
		printf("Only PCM wav format is supported.\n");
		return 1;
	}

	if(p.verbose){
		printf("#  file size is %u in buffer\n", infile_bytes_size);
	}

	end = infile_bytes + infile_bytes_size;
	data = find_wav_data(((char*)header) + 36, end, p.verbose);
	if(data == (char*)0){
		printf("#  Unable to find data section.");
		return 1;
	}
	return analyze_audio_and_build_command(&p, header, data);
}
