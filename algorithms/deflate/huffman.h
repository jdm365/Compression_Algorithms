#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NUM_CODES 286

typedef struct MinHeapNode MinHeapNode;
typedef struct MinHeapNode {
	uint8_t data;
	uint32_t frequency;
	MinHeapNode* left;
	MinHeapNode* right;
} MinHeapNode;

MinHeapNode* new_node(uint8_t data, uint32_t frequency);

inline bool compare_nodes(MinHeapNode* a, MinHeapNode* b) {
	return a->frequency > b->frequency;
}

void push_heap(
		MinHeapNode** heap,
		uint32_t* heap_size,
		uint8_t data,
		uint32_t frequency
		);

MinHeapNode* pop_heap(
		MinHeapNode** heap,
		uint32_t* heap_size
		);

static const uint32_t BIT_MASK[33] = {
	0x00000000,
	0x00000001,
	0x00000003,
	0x00000007,
	0x0000000F,
	0x0000001F,
	0x0000003F,
	0x0000007F,
	0x000000FF,
	0x000001FF,
	0x000003FF,
	0x000007FF,
	0x00000FFF,
	0x00001FFF,
	0x00003FFF,
	0x00007FFF,
	0x0000FFFF,
	0x0001FFFF,
	0x0003FFFF,
	0x0007FFFF,
	0x000FFFFF,
	0x001FFFFF,
	0x003FFFFF,
	0x007FFFFF,
	0x00FFFFFF,
	0x01FFFFFF,
	0x03FFFFFF,
	0x07FFFFFF,
	0x0FFFFFFF,
	0x1FFFFFFF,
	0x3FFFFFFF,
	0x7FFFFFFF,
	0xFFFFFFFF
};

typedef struct {
	uint32_t* buffer;
	uint64_t bit_idx;
	uint64_t word_idx;
	uint64_t buffer_size;
} BitWriter;

void init_bitwriter(BitWriter* writer, uint64_t buffer_size);
void write_bits(BitWriter* writer, uint32_t bits, uint8_t length);
void read_bits(BitWriter* writer, uint32_t* bits, uint8_t length);

void  append_huffman_tree_literal(uint32_t* frequencies, char literal);
void  append_huffman_tree_pair(uint32_t* frequencies, uint16_t offset);

void  build_huffman_tree(uint32_t* frequencies, uint64_t size, MinHeapNode** root);

void  gather_codes(
		MinHeapNode* root,
		uint16_t code,
		uint8_t length,
		uint16_t* codes,
		uint8_t*  code_lengths
		);
