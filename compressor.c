
#include "compressor.h"

struct trie_node * make_trie_node(char c, unsigned int trie_depth){
	struct trie_node * t = malloc(sizeof(struct trie_node));
	t->c = c;
	t->num_children = 0;
	t->count = 0;
	t->last_index = -trie_depth;
	t->children = (struct trie_node_ref*)0;
	return t;
}

void add_trie_child(struct trie_node * t, char c, unsigned int trie_depth){
	t->num_children++;
	t->children = realloc(t->children, sizeof(struct trie_node_ref) * t->num_children);
	t->children[t->num_children-1].c = c;
	t->children[t->num_children-1].t = make_trie_node(c, trie_depth);
}

void add_trie_string(struct trie_node *, struct character_node *, unsigned int, unsigned int, unsigned int);

void traverse_trie_child(struct trie_node * t, struct character_node * str, unsigned int num_chars, unsigned int origin_index, unsigned int trie_depth){
	int i;
	assert(num_chars);
	for(i = 0; i < t->num_children; i++){
		if(t->children[i].c == str->c){
			add_trie_string(t->children[i].t, str->next, num_chars-1, origin_index, trie_depth);
			return;
		}
	}
	/*  There were no matching children above, so create a new one. */
	add_trie_child(t, str->c, trie_depth);
	/*  Just re-try again. */
	traverse_trie_child(t, str, num_chars, origin_index, trie_depth);
}

void add_trie_string(struct trie_node * t, struct character_node * str, unsigned int num_chars, unsigned int origin_index, unsigned int trie_depth){
	if(num_chars > 0){
		traverse_trie_child(t, str, num_chars, origin_index, trie_depth);
	}else{
		/*  Make sure that this match isn't going to overlap the previous one. */
		if(t->last_index + trie_depth <= origin_index){
			/*  The index of the character where this substring started. */
			t->last_index = origin_index;
			t->count++;
		}
	}
}

void print_trie_level(struct trie_node * t, unsigned int level){
	int i;
	for(i = 0; i < level; i++) printf(">>");
	printf("C is: '%c'\n", t->c);
	for(i = 0; i < level; i++) printf(">>");
	printf("count is: %u\n", t->count);

	for(i = 0; i < t->num_children; i++){
		print_trie_level(t->children[i].t, level+1);
	}
}


void trie_destroy(struct trie_node * t){
	unsigned int i;
	for(i = 0; i < t->num_children; i++){
		trie_destroy(t->children[i].t);
	}
	free(t->children);
	free(t);
}

void make_random_input(struct print_buffer * b){
	char samples[17] = {'0','1','2','3','4','5','6','7','8','9','e','-','=', '.', '+','n','x'};
	unsigned int size = rand() % 1000;
	int i;
	for(i = 0; i < size; i++){
		printfbuff(b, "%c", samples[rand() % 17]);
	}
}


void find_max_count_trie(struct trie_node * t, unsigned int * max){
	int i;
	if(t->count > *max){
		*max = t->count;
	}

	for(i = 0; i < t->num_children; i++){
		find_max_count_trie(t->children[i].t, max);
	}
}

unsigned int fill_max_count_trie(struct trie_node * t, unsigned int max, char * c){
	/*  Find the first one that had max count and use that one. */
	int i;
	*c = t->c;
	if(t->count == max){
		return 1;
	}

	for(i = 0; i < t->num_children; i++){
		if(fill_max_count_trie(t->children[i].t, max, c+1)){
			return 1;
		}
	}
	return 0;
}

void make_character_list(struct character_node ** n, char * c, unsigned int chars_left){
	struct character_node * prev = (struct character_node *)0;
	*n = (struct character_node *)0;
	while(chars_left > 0){
		*n = malloc(sizeof(struct character_node));
		(*n)->c = c[0];
		(*n)->prev = prev;
		
		/*  Advance to next char. */
		chars_left--;
		c += 1;
		n = &(*n)->next;
		if(chars_left == 0){
			*n = (struct character_node *)0;
		}
	}
}

void enumerate_character_list(struct character_node * n, unsigned int index){
	while(n){
		n->index = index;
		n = n->next;
		index += 1;
	}
}

void destroy_character_list(struct character_node * n){
	while(n){
		struct character_node * t = n;
		n = n->next;
		free(t);
	}
}

void print_character_list(struct character_node * n, struct print_buffer * b){
	while(n){
		printfbuff(b, "%c", n->c);
		n = n->next;
	}
}

int check_sequence_match(struct character_node * chrs, char * find, unsigned int chrs_left){
	if(chrs){
		if(chrs_left){
			if(chrs->c == *find){
				return check_sequence_match(chrs->next, find + 1, chrs_left -1);
			}else{
				return 0;
			}
		}else{
			return 1;
		}
	}else{
		return 0;
	}
}

struct character_node * get_following_character(struct character_node * chr,  unsigned int depth){
	if(depth){
		return get_following_character(chr->next,  depth -1);
	}else{
		return chr;
	}
}

void replace_sequences(struct character_node ** chrs, char * winning_trie, unsigned int trie_depth, char c){
	struct character_node * current = *chrs;
	struct character_node * previous = (struct character_node *)0;
	while(current){
		if(check_sequence_match(current, winning_trie, trie_depth)){
			struct character_node * n = malloc(sizeof(struct character_node));
			struct character_node * following = get_following_character(current, trie_depth);
			/*  Update new node links */
			n->c = c;
			n->prev = previous;
			n->next = following;
			/*  Update link from prev to new */
			if(previous){
				previous->next = n;
			}else{
				*chrs = n;
			}

			/*  Update link from following to n. */
			if(following){
				following->prev = n;
			}

			while(current != following){
				struct character_node * next = current->next;
				free(current);
				current = next;
			}

			/*  Update navigation pointers. */
			previous = n;
			current = following;
		}else{
			previous = current;
			current = current->next;
		}
	}
}

unsigned int count_characters(struct character_node * c){
	unsigned int i = 0;
	while(c){
		c = c->next;
		i++;
	}
	return i;
}

void set_next_available_character(struct compression_map * m){
	signed int i;
	for(i = m->next_available_character + 1; i < 256; i++){
		if(!m->decompression_pointers[i]){
			m->next_available_character = i;
			return;
		}
	}
	m->next_available_character = -1;
}

void initialize_decompression_map(struct compression_map * m){
	unsigned int i;
	m->next_available_character = 0;
	/*  Control characters, map to empty string. */
	for(i = 0; i < 32; i++){
		m->decompression_pointers[i] = malloc(1 * sizeof(char));
		m->decompression_pointers[i][0] = '\0';
	}

	/*  Most regular characters.  Initialize to null. */
	for(i = 32; i < 128; i++){
		m->decompression_pointers[i] = (char *)0;
	}

	/*  Extended ASCII characters. */
	for(i = 128; i < 256; i++){
		m->decompression_pointers[i] = malloc(1 * sizeof(char));
		m->decompression_pointers[i][0] = '\0';
	}

	/*  Digits map to themselves. */
	for(i = 48; i < 58; i++){
		m->decompression_pointers[i] = malloc(2 * sizeof(char));
		m->decompression_pointers[i][0] = i;
		m->decompression_pointers[i][1] = '\0';
	}

	/*  Other terminal characters that must map to themselves. */
	{
		char terminals[7] = {'e', '=', 'x', '-', '.', '+', ';'};
		for(i = 0; i < 7; i++){
			m->decompression_pointers[(int)terminals[i]] = malloc(2 * sizeof(char));
			m->decompression_pointers[(int)terminals[i]][0] = terminals[i];
			m->decompression_pointers[(int)terminals[i]][1] = '\0';
		}
	}

	/*  n is a special case because we want to translate it directly into a newline in the meme transform tool. */
	m->decompression_pointers['n'] = malloc(3 * sizeof(char));
	m->decompression_pointers['n'][0] = '\\';
	m->decompression_pointers['n'][1] = 'n';
	m->decompression_pointers['n'][2] = '\0';

	/*  Illegal characters that might cause problems in shells and should never show up in input. Map them to empty string. */
	{
		char illegals[9] = {'!', '`', '"', '\'', '$', '~', ',', '\\', 0x7F};
		for(i = 0; i < 9; i++){
			m->decompression_pointers[(int)illegals[i]] = malloc(1 * sizeof(char));
			m->decompression_pointers[(int)illegals[i]][0] = '\0';
		}
	}

	set_next_available_character(m);
}

void print_compression_map(struct compression_map * m){
	int i;
	for(i = 0; i < 256; i++){
		printf("0x%X ", i);
		if(i > 31 && i < 127){
			printf("'%c' ", i);
		}else{
			printf("--- ");
		}
		printf("--- => ");
		if(m->decompression_pointers[i]){
			printf("'%s'\n", m->decompression_pointers[i]);
		}else{
			printf("null\n");
		}
	}
}

void create_compressed_package(struct compression_map * m, struct character_node * chrs, struct print_buffer * b){
	unsigned int i;
	printfbuff(b, "echo -n '");
	print_character_list(chrs, b);
	printfbuff(b, "' | ");

	printfbuff(b, "hexdump -v -e '/1 \"%%u\\n\"' | awk '");
	printfbuff(b, "function d(f,k,a,b,o,w,i,q){");
		printfbuff(b, "for(k=32;k<127;k++)");
		printfbuff(b, "o[sprintf(\"%%c\", k)]=k;");
		printfbuff(b, "split(\"");
		for(i = 32; i < 127; i++){
			if(m->decompression_pointers[i]){
				printfbuff(b, "%s,", m->decompression_pointers[i]);
			}else{
				printfbuff(b, ",");
			}
		}
		printfbuff(b, "\",a,\",\");");
		/*  The list of terminal characters that will stop the recursion. */
		printfbuff(b, "split(\"0,1,2,3,4,5,6,7,8,9,=,e,x,+,-,.,n,;\",w,\",\");");
		printfbuff(b, "q=0;");
		printfbuff(b, "for(g in w)");
			printfbuff(b, "if(o[w[g]]==f)");
				printfbuff(b, "q++;");
		printfbuff(b, "i=f-32+1;");
		printfbuff(b, "if(q>0)");
			printfbuff(b, "printf(a[i]);");
		printfbuff(b, "else{");
			/*  Split up the characters and recursively run the expansion rules on each character. */
			printfbuff(b, "split(a[i],b,\"\");");
			printfbuff(b, "for(j in b)");
				printfbuff(b, "d(o[b[j]]);");
		printfbuff(b, "}");
	printfbuff(b, "}");
	printfbuff(b, "d($1)");
	printfbuff(b, "'");
}

void destroy_compression_map(struct compression_map * m){
	int i;
	for(i = 0; i < 256; i++){
		free(m->decompression_pointers[i]);
	}
}

void add_decompression_mapping(struct compression_map * m, char * winning_trie, unsigned int trie_depth, char c){
	char * new = malloc((trie_depth + 1) * sizeof(char));
	unsigned int i;
	for(i = 0; i < trie_depth; i++){
		new[i] = winning_trie[i];
	}
	new[i] = '\0';
	m->decompression_pointers[(int)c] = new;
}

void do_compression(struct print_buffer * in, struct print_buffer * out, unsigned int verbose){
	struct compression_map m;
	unsigned int max_trie_depth = MAX_TRIE_DEPTH; /*  The maximum length of substring that we may want to replace. */
	unsigned int occurrences[MAX_TRIE_DEPTH]; /*  For calculating the occurrences from the various sequence sizes. */
	int i;
	struct character_node * chrs;
	unsigned int done = 0;
	unsigned int current_size;
	initialize_decompression_map(&m);
	make_character_list(&chrs, in->characters, in->buffer_size-1);
	/*  While there are still free characters that can be used as a substitution rule. */
	while(!done && m.next_available_character != -1){
		unsigned int trie_depth;
		char winning_trie[MAX_TRIE_DEPTH][MAX_TRIE_DEPTH + 1];
		unsigned int highest_trie = 0;
		unsigned int highest_trie_benefit = 0;
		enumerate_character_list(chrs, 0);
		/*  Array to store the nth winning result, which is a string that takes up trie_depth + 1 characters. */
		current_size = count_characters(chrs);
		for(trie_depth = 2; trie_depth < max_trie_depth; trie_depth++){
			struct trie_node * root = make_trie_node('\0', trie_depth);
			struct character_node * current_character = chrs;
			for(i = 0; (current_size > trie_depth) && (i < (current_size-(trie_depth-1))); i++){
				add_trie_string(root, current_character, trie_depth, i, trie_depth);
				current_character = current_character->next;
			}

			occurrences[trie_depth] = 0;
			find_max_count_trie(root, &occurrences[trie_depth]);
			fill_max_count_trie(root, occurrences[trie_depth], &winning_trie[trie_depth][0]);
			trie_destroy(root);
		}

		for(trie_depth = 2; (trie_depth < max_trie_depth) && current_size > trie_depth; trie_depth++){
			/*  The approximate benefit in terms of characters removed considering the decompressor rule and replcement character. */
			unsigned int current_benefit = occurrences[trie_depth] > 0 ? (occurrences[trie_depth] -1) * (trie_depth-1) : 0;
			if(current_benefit > highest_trie_benefit){
				highest_trie_benefit = current_benefit;
				highest_trie = trie_depth;
			}
		}
		if(highest_trie == 0){
			done = 1; /*  Exit loop. */
		}else{
			if(verbose){
				unsigned int g;
				printf("#  Creating replacement rule for trie length %u '%c' -> '", highest_trie, m.next_available_character);
				for(g = 1; g < highest_trie + 1; g++){
					printf("%c", winning_trie[highest_trie][g]);
				}
				printf("' for a savings of %u characters.\n", highest_trie_benefit);
			}
			replace_sequences(&chrs, &winning_trie[highest_trie][1], highest_trie, m.next_available_character);
			add_decompression_mapping(&m, &winning_trie[highest_trie][1], highest_trie, m.next_available_character);
			set_next_available_character(&m);
		}
	}

	create_compressed_package(&m, chrs, out);

	destroy_compression_map(&m);
	destroy_character_list(chrs);
}

void do_compression_tests(void){
	unsigned int i;
	srand(3);
	for(i = 0; i < 100; i++){
		struct print_buffer in;
		struct print_buffer out;
		init_print_buffer(&in);
		init_print_buffer(&out);

		make_random_input(&in);
		do_compression(&in, &out, 1);
		free(in.characters);
		printf("Result is '%s'\n",out.characters);
		free(out.characters);
	}
}
