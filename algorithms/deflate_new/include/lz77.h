#pragma once

#include <stdint.h>

#define LENGTH_BITS 4
#define WINDOW_BITS 14 
#define TABLE_SIZE (WINDOW_BITS + 6)


uint64_t min(uint64_t a, uint64_t b);
uint64_t max(uint64_t a, uint64_t b);

typedef struct {
    uint8_t* data;
    uint64_t bit_index;
} BitStream;

typedef struct ArrayNode {
	uint32_t pattern;
	uint64_t index;
	bool is_set;
} ArrayNode;

typedef struct {
	ArrayNode* buckets;
	uint32_t bucket_indices[1 << WINDOW_BITS];
	uint32_t current_idx;
	bool is_full;
} HashTableArray;

uint32_t hash(uint32_t pattern);
void init_hash_table(HashTableArray* table);
void insert_hash_table(HashTableArray* table, uint32_t pattern, uint64_t index);
uint64_t find(HashTableArray* table, uint32_t pattern);

void  init_bitstream(BitStream* stream, uint8_t* buffer);

void write_bit(BitStream* stream, bool bit);
bool read_bit(BitStream* stream);
void write_bits(BitStream* stream, uint64_t value, uint64_t num_bits);
uint64_t read_bits(BitStream* stream, uint64_t num_bits);

BitStream* lz77_compress_old(
		const char* buffer,
		uint64_t size
		);
BitStream* lz77_compress(
		const char* buffer,
		uint64_t size
		);
char* lz77_decompress(
		BitStream* compressed_stream,
		uint64_t  size,
		uint64_t* decompressed_size
		);

