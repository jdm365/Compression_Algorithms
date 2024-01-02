#include <stdio.h>
#include <stdlib.h>

#include <queue>

#include "huffman.h"


void print_bit_string(char* buffer, u64 size) {
	for (u64 idx = 0; idx < size; ++idx) {
		for (int bit = 7; bit >= 0; --bit) {
			bool is_set = buffer[idx] & (1 << bit);
			printf("%d", is_set ? 1 : 0);
		}
	}
	printf("\n");
}

char* read_input_buffer(
		const char* filename,
		u64* size
		) {
	char* buffer;

	FILE* file = fopen(filename, "rb");
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = (char*)malloc(*size + 1);
	fread(buffer, 1, *size, file);
	fclose(file);


	return buffer;
}


void build_huffman_tree(
		char* buffer,
		u64 size,
		Node& root
		) {
	u32 frequencies[256] = {0};
	for (u64 idx = 0; idx < size; ++idx) {
		frequencies[(u8)buffer[idx]]++;
	}

	std::priority_queue<Node*, std::vector<Node*>, Node::compare> queue;
	for (u32 idx = 0; idx < 256; ++idx) {
		if (frequencies[idx] > 0) {
			queue.push(new Node((char)idx, frequencies[idx]));
		}
	}

	while (queue.size() > 1) {
		Node* left = queue.top();
		queue.pop();

		Node* right = queue.top();
		queue.pop();

		Node* parent = new Node(0, left->frequency + right->frequency);
		parent->left = left;
		parent->right = right;
		queue.push(parent);
	}

	root = *queue.top();
}


void gather_codes(
		Node& root,
		u32 code,
		u32 length,
		u32* codes,
		u32* code_lengths
		) {
	if (root.left == nullptr && root.right == nullptr) {
		codes[(u8)root.value] 		 = code;
		code_lengths[(u8)root.value] = length;
		return;
	}

	if (root.left != nullptr) {
		gather_codes(
				*root.left, 
				code,
				length + 1, 
				codes, 
				code_lengths
				);
	}

	if (root.right != nullptr) {
		gather_codes(
				*root.right, 
				code | (1 << length),
				length + 1, 
				codes, 
				code_lengths
				);
	}
}

void print_codes(
		u32* codes,
		u32* code_lengths
		) {
	for (u32 idx = 0; idx < 256; ++idx) {
		if (code_lengths[idx] > 0) {
			printf("%c: ", (char)idx);
			for (u32 bit_idx = 0; bit_idx < code_lengths[idx]; ++bit_idx) {
				printf("%d", (codes[idx] >> (code_lengths[idx] - bit_idx - 1)) & 1);
			}
			printf("\n");
		}
	}
}

void huffman_compress(
		char* buffer,
		u64   size,
		u32*  codes,
		u32*  code_lengths,
		char* output,
		u64*  output_size
		) {
	u64 byte_idx = 0;
	u8  bit_idx  = 0;

	memset(output, 0, size);

	for (u64 idx = 0; idx < size; ++idx) {
		u32 code   = codes[(u8)buffer[idx]];
		u32 length = code_lengths[(u8)buffer[idx]];

		for (u32 idx = 0; idx < length; ++idx) {
			bool bit = (code >> idx) & 1;
			output[byte_idx] |= (bit << (bit_idx));
			bit_idx = (bit_idx + 1) % 8;
			byte_idx += (bit_idx == 0);
		}
	}

	*output_size = byte_idx + (bit_idx > 0);
}

void huffman_decompress(
		char* compressed_buffer,
		u64 compressed_size,
		Node& root,
		char* output,
		u64* output_size
		) {
	u64 char_idx = 0;
	u64 byte_idx = 0;
	u8  bit_idx  = 0;

	memset(output, 0, *output_size);

	do {
		Node* node = &root;
		while (node->left != nullptr && node->right != nullptr) {
			if (compressed_buffer[byte_idx] & (1 << bit_idx)) {
				node = node->right;
			} else {
				node = node->left;
			}
			bit_idx = (bit_idx + 1) % 8;
			byte_idx += (bit_idx == 0);
		}

		output[char_idx] = node->value;
		char_idx++;
	} while (byte_idx < compressed_size);

	*output_size = char_idx - (bit_idx > 0);
}
