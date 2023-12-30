#pragma once

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

u32 u32_hash(const u8* data, int size);

struct u32_HashMap {
	u8** keys;
	u32* values;
	u32* key_sizes;
};

void u32_add_item(u32_HashMap* map, u8* key, u32 value, int key_size);
u32  u32_get_item(u32_HashMap* map, u8* key, int key_size);

void lz77_compress(
		const u8* input_data,
		u32 	  input_size,
		u8* 	  output_data,
		u32* 	  output_size
		);

void lz77_decompress(
		const u8* input_data,
		u32 	  input_size,
		u8* 	  output_data,
		u32* 	  output_size
		);

struct MinHeapNode {
	u8 			 data;
	u32 		 freq;
	MinHeapNode* left;
	MinHeapNode* right;
};

struct MinHeap {
	u32 		  size;
	u32 		  capacity;
	MinHeapNode** array;
};

void build_huffman_tree(
		const u32* 	  weights,
		u32		   	  num_weights,
		MinHeapNode** root
		);

void huffman_compress(
		const u8* input_data,
		u32 	  input_size,
		u8* 	  output_data,
		u32* 	  output_size
		);

void huffman_decompress(
		const u8* input_data,
		u32 	  input_size,
		u8* 	  output_data,
		u32* 	  output_size
		);

void deflate_compress(
		const u8* input_data,
		u32 	  input_size,
		u8* 	  output_data,
		u32* 	  output_size
		);

void deflate_decompress(
		const u8* input_data,
		u32 	  input_size,
		u8* 	  output_data,
		u32* 	  output_size
		);

void write_gzip_header(
		u8*  output_data,
		u32* output_size
		);
