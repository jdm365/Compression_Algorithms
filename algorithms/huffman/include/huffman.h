#pragma once

#include <stdint.h>
#include <stdio.h>

#include <queue>

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

void  print_bit_string(u8* buffer, u64 size);
char* read_input_buffer(const char* filename, u64* size);
void  build_huffman_tree(char* buffer, u64 size, Node& root);
void  gather_codes(
		Node& root,
		u32 code,
		u32 length,
		u32* codes,
		u32* code_lengths
		);
void  print_codes(u32* codes, u32* code_lengths);
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
		char* compressed_buffer,
		u64*  compressed_bytes 
		);
void  huffman_decompress(
		char* compressed_buffer,
		u64 compressed_size,
		Node& root,
		char* output,
		u64* output_size
		);
