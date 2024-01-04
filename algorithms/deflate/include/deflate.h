#pragma once

#include "lz77.h"
#include "huffman.h"

typedef struct StateData {
	u64        lz77_compressed_size;
	char*      huffman_compressed_data;
	u64        huffman_compressed_size;
	Node*      huffman_root;
} StateData;

StateData deflate_compress(
		char* data, 
		u64   size, 
		u8    window_bits,
		u8    length_bits
		);

char* deflate_decompress(
		StateData state_data,
		u64*  decompressed_size,
		u8    window_bits,
		u8    length_bits
		);

void  print_bit_string(char* buffer, u64 size);
char* read_input_buffer(const char* filename, u64* size);
bool  compare_buffers(char* buffer1, char* buffer2, u64 size);
