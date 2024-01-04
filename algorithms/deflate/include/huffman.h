#pragma once

#include <stdint.h>

#include "lz77.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct Node {
	char value;
	u32 frequency;
	Node* left;
	Node* right;

	Node(char value, u32 frequency) {
		this->value = value;
		this->frequency = frequency;
		this->left  = nullptr;
		this->right = nullptr;
	}

	~Node() {
		if (left != nullptr) {
			delete left;
		}

		if (right != nullptr) {
			delete right;
		}
	}

	struct compare {
		bool operator()(Node* left, Node* right) {
			return left->frequency > right->frequency;
		}
	};
};

void  build_huffman_tree(char* buffer, u64 size, Node& root);
void  gather_codes(
		Node& root,
		u32 code,
		u32 length,
		u32* codes,
		u32* code_lengths
		);
void  build_huffman_tree_u16(u16* buffer, u64 size, Node& root);
void  gather_codes_u16(
		Node& root,
		u32 code,
		u32 length,
		u32* codes,
		u32* code_lengths
		);
void  print_codes(u32* codes, u32* code_lengths, u32 size);
void  _huffman_compress(
		char* buffer,
		u64   size,
		u32*  codes,
		u32*  code_lengths,
		char* output,
		u64*  output_size
		);
Node  huffman_compress(
		char* buffer,
		u64   size,
		char* output,
		u64*  output_size
		);

void  huffman_decompress(
		char* compressed_buffer,
		u64 compressed_size,
		Node& root,
		char* output,
		u64* output_size
		);

typedef struct LZSSTrees {
	Node* root_literals;
	Node* root_offsets;
} LZSSTrees;

LZSSTrees huffman_compress_lzss_data(
		BitStream* stream,
		u64   size,
		char* output,
		u64*  output_size,
		u8    window_bits,
		u8    length_bits
		);

void  huffman_decompress_lzss_data(
		char* compressed_buffer,
		u64 compressed_size,
		LZSSTrees trees,
		char* output,
		u64* output_size
		);

void write_bit(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx,
		bool bit
		);
bool read_bit(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx
		);
void write_bits(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx,
		u64  value,
		u64  num_bits
		);
u64 read_bits(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx,
		u32 length
		);
