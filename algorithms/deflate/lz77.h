#pragma once

#include <stdint.h>

#define MAX_WINDOW_BITS 15
#define WINDOW_SIZE (1 << MAX_WINDOW_BITS)
#define MAX_LENGTH_BITS 5
#define TABLE_SIZE (1 << (MAX_WINDOW_BITS + 5))

typedef struct ArrayNode {
	uint32_t pattern;
	uint64_t index;
	bool is_set;
} ArrayNode;

typedef struct {
	ArrayNode* buckets;
	uint32_t bucket_indices[1 << MAX_WINDOW_BITS];
	uint32_t current_idx;
	bool is_full;
} HashTableArray;

uint32_t hash(uint32_t pattern);
void init_hash_table(HashTableArray* table);
void insert_hash_table(HashTableArray* table, uint32_t pattern, uint64_t index);
uint64_t find(HashTableArray* table, uint32_t pattern);

void write_literal(
		char* buffer, 
		char c, 
		uint64_t* buffer_index
		);
void write_length_distance(
		char* buffer, 
		uint8_t length, 
		uint16_t distance, 
		uint64_t* buffer_index
		);

void lz77_compress(
		const char* input_buffer,
		uint64_t input_buffer_size,
		char* compressed_buffer,
		uint64_t* compressed_buffer_size,
		HashTableArray* table
		);
void lz77_decompress(
		const char* compressed_buffer,
		uint64_t compressed_buffer_size,
		char* decompressed_buffer,
		uint64_t* decompressed_buffer_size
		);

