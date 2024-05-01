#pragma once

#include <stdint.h>

#include "huffman.h"

typedef struct StateData {
	uint64_t   lz77_compressed_size;
	BitWriter* huffman_writer;
	Node* 	   huffman_tree;
} StateData;

StateData deflate_compress(
		const char* data, 
		uint64_t size
		);

char* deflate_decompress(
		StateData  state_data,
		uint64_t*  decompressed_size
		);

void  print_bit_string(char* buffer, uint64_t size);
char* read_input_buffer(const char* filename, uint64_t* size);
bool  compare_buffers(char* buffer1, char* buffer2, uint64_t size);
