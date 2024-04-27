#pragma once

#include <stdint.h>

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
inline void write_bits(BitWriter *writer, uint32_t bits, uint32_t length);
inline void read_bits(BitWriter *writer, uint32_t* bits, uint32_t length);

struct Node {
	uint8_t value;
	uint32_t frequency;
	Node* left;
	Node* right;

	Node(uint8_t value, uint32_t frequency) {
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
		bool operator()(const Node* left, const Node* right) {
			return left->frequency > right->frequency;
		}
	};
};

void  print_bit_string(uint8_t* buffer, uint64_t size);
char* read_input_buffer(const char* filename, uint64_t* size);
void  build_huffman_tree(char* buffer, uint64_t size, Node** root);
void  gather_codes(
		Node* root,
		uint32_t code,
		uint32_t length,
		uint32_t* codes,
		uint32_t* code_lengths
		);
void  gather_codes(
		Node* root,
		uint32_t code,
		uint32_t length,
		uint32_t* codes,
		uint8_t*  code_lengths
		);
void  print_codes(uint32_t* codes, uint32_t* code_lengths);
void  print_codes(uint32_t* codes, uint8_t* code_lengths);

void  _huffman_compress(
		char* buffer,
		uint64_t   size,
		uint32_t*  codes,
		uint32_t*  code_lengths,
		uint8_t* output,
		uint64_t*  output_size
		);
Node  huffman_compress(
		char* buffer,
		uint64_t   size,
		uint8_t* compressed_buffer,
		uint64_t*  compressed_bytes 
		);
void  huffman_decompress(
		uint8_t* compressed_buffer,
		uint64_t compressed_size,
		Node& root,
		char* output,
		uint64_t* output_size
		);

void  _huffman_compress_v2(
		char* buffer,
		uint64_t   size,
		uint32_t*  codes,
		uint8_t*   code_lengths,
		BitWriter* writer
		);
Node  huffman_compress_v2(
		char* buffer,
		uint64_t   size,
		BitWriter* writer
		);
void  huffman_decompress_v2(
		BitWriter* writer,
		Node& root,
		char* output,
		uint64_t* output_size
		);
