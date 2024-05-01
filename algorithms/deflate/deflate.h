#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "lz77.h"

#define BUFFER_SIZE 65536

static const char* extension = ".deflate";

typedef struct HuffmanNode {
	struct HuffmanNode* left;
	struct HuffmanNode* right;
	uint16_t value;
	uint64_t frequency;
} HuffmanNode;

void init_huffman_node(HuffmanNode* node);
void destroy_huffman_node(HuffmanNode* node);
bool compare_huffman_node(const HuffmanNode* a, const HuffmanNode* b);

typedef struct StateData {
	HashTableArray* table;
	HuffmanNode* huffman_root;
	char* compressed_filename;
} StateData;

StateData compress(const char* input_filename);
void decompress(StateData* state_data, const char* input_filename);
