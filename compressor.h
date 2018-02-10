#ifndef MEME_COMPRESSOR_H
#define MEME_COMPRESSOR_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "io.h"

#define MAX_TRIE_DEPTH 4

struct trie_node{
	char c;
	struct trie_node_ref * children;
	unsigned int num_children;
	unsigned int count;
	int last_index;
};

struct trie_node_ref{
	char c;
	struct trie_node * t;
};

struct character_node{
	unsigned int index;
	char c;
	struct character_node * next;
	struct character_node * prev;
};

struct compression_map{
	/*  An array of pointers that describes what each character will decompress to. */
	char * decompression_pointers[256]; /*  Pointers to strings that describe what the character should map to.
		NULL pointer means that this character is free to represent something new.  Illegal characters should be
		represented as mapping to empty string.
	*/
	signed int next_available_character;
};


struct trie_node * make_trie_node(char c, unsigned int trie_depth);
void add_trie_child(struct trie_node * t, char c, unsigned int trie_depth);
void add_trie_string(struct trie_node *, struct character_node *, unsigned int, unsigned int, unsigned int);
void traverse_trie_child(struct trie_node * t, struct character_node * str, unsigned int num_chars, unsigned int origin_index, unsigned int trie_depth);
void add_trie_string(struct trie_node * t, struct character_node * str, unsigned int num_chars, unsigned int origin_index, unsigned int trie_depth);
void print_trie_level(struct trie_node * t, unsigned int level);
void trie_destroy(struct trie_node * t);
void make_random_input(struct print_buffer *);
void find_max_count_trie(struct trie_node * t, unsigned int * max);
unsigned int fill_max_count_trie(struct trie_node * t, unsigned int max, char * c);
void make_character_list(struct character_node ** n, char * c, unsigned int chars_left);
void enumerate_character_list(struct character_node * n, unsigned int index);
void destroy_character_list(struct character_node * n);
void print_character_list(struct character_node *, struct print_buffer *);
int check_sequence_match(struct character_node * chrs, char * find, unsigned int chrs_left);
struct character_node * get_following_character(struct character_node * chr,  unsigned int depth);
void replace_sequences(struct character_node ** chrs, char * winning_trie, unsigned int trie_depth, char c);
unsigned int count_characters(struct character_node * c);
void set_next_available_character(struct compression_map * m);
void initialize_decompression_map(struct compression_map * m);
void print_compression_map(struct compression_map * m);
void create_compressed_package(struct compression_map *, struct character_node *, struct print_buffer *);
void destroy_compression_map(struct compression_map * m);
void add_decompression_mapping(struct compression_map * m, char * winning_trie, unsigned int trie_depth, char c);
void do_compression(struct print_buffer *, struct print_buffer *, unsigned int);

#endif
