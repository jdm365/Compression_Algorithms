#include <stdio.h>
#include <stdlib.h>

#include <queue>

#include "lz77.h"
#include "huffman.h"

inline bool read_bit(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx
		) {
	bool bit = buffer[*byte_idx] & (1 << *bit_idx);
	*bit_idx = (*bit_idx + 1) % 8;
	*byte_idx += (*bit_idx == 0);
	return bit;
}

inline void write_bit(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx,
		bool bit
		) {
	buffer[*byte_idx] |= (bit << (*bit_idx));
	*bit_idx = (*bit_idx + 1) % 8;
	*byte_idx += (*bit_idx == 0);
}

void write_bits(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx,
		u32 code,
		u32 length
		) {
	for (u32 idx = 0; idx < length; ++idx) {
		bool bit = (code >> idx) & 1;
		write_bit(buffer, byte_idx, bit_idx, bit);
	}
}

u64 read_bits(
		char* buffer,
		u64* byte_idx,
		u8*  bit_idx,
		u32 length
		) {
	u64 code = 0;
	for (u32 idx = 0; idx < length; ++idx) {
		bool bit = read_bit(buffer, byte_idx, bit_idx);
		code |= (bit << idx);
	}
	return code;
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

void build_huffman_tree_u16(
		u16* buffer,
		u64 size,
		Node& root
		) {
	u32 frequencies[65536] = {0};
	for (u64 idx = 0; idx < size; ++idx) {
		frequencies[(u16)buffer[idx]]++;
	}

	std::priority_queue<Node*, std::vector<Node*>, Node::compare> queue;
	for (u32 idx = 0; idx < 65536; ++idx) {
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

void gather_codes_u16(
		Node& root,
		u32 code,
		u32 length,
		u32* codes,
		u32* code_lengths
		) {
	if (root.left == nullptr && root.right == nullptr) {
		codes[(u16)root.value] 		  = code;
		code_lengths[(u16)root.value] = length;
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
		u32* code_lengths,
		u32  size
		) {
	for (u32 idx = 0; idx < size; ++idx) {
		if (code_lengths[idx] > 0) {
			if (size == 256) {
				printf("%c: ", (char)idx);
			} else {
				printf("%d: ", idx);
			}
			for (u32 bit_idx = 0; bit_idx < code_lengths[idx]; ++bit_idx) {
				printf("%d", (codes[idx] >> (code_lengths[idx] - bit_idx - 1)) & 1);
			}
			printf("\n");
		}
	}
}

void _huffman_compress(
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

		write_bits(output, &byte_idx, &bit_idx, code, length);
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

Node huffman_compress(
		char* buffer,
		u64   size,
		char* output,
		u64*  output_size
		) {
	Node root(0, 0);
	build_huffman_tree(buffer, size, root);

	u32 codes[256] 		  = {0};
	u32 code_lengths[256] = {0};
	gather_codes(root, 0, 0, codes, code_lengths);

	_huffman_compress(
			buffer,
			size,
			codes,
			code_lengths,
			output,
			output_size
			);

	output = (char*)realloc(output, *output_size);

	return root;
}

LZSSTrees huffman_compress_lzss_data(
		BitStream* stream,
		u64   size,
		char* output,
		u64*  output_size,
		u8    window_bits,
		u8    length_bits
		) {
	LZSSTrees trees;
	char* buffer = (char*)malloc(size);

	// Gather literal and lzss data buffers.
	u64 total_compressed_bits = stream->bit_index;
	stream->bit_index = 0;

	char* literal_buffer = (char*)malloc(size);
	memset(literal_buffer, 0, size);
	u64   literal_buffer_size = 0;

	u16*  lzss_buffer = (u16*)malloc(size * sizeof(u16));
	memset(lzss_buffer, 0, size * sizeof(u16));
	u64   lzss_buffer_size = 0;

	u64   window_size = (1 << window_bits) - 1;

	u64   buffer_byte_index = 0;
	u8    buffer_bit_index  = 0;

	bool* is_match_buffer = (bool*)malloc(size * sizeof(bool));
	memset(is_match_buffer, 0, size * sizeof(bool));
	u64   is_match_buffer_size = 0;

	u64   buffer_index = 0;
	while (buffer_index < size) {
		bool is_match = read_bit(
				(char*)stream->data,
				&buffer_byte_index,
				&buffer_bit_index
				);
		is_match_buffer[is_match_buffer_size] = is_match;
		is_match_buffer_size++;

		if (is_match) {
			u64 match_offset = read_bits(
					(char*)stream->data,
					&buffer_byte_index,
					&buffer_bit_index,
					window_bits
					);
			u64 match_length = read_bits(
					(char*)stream->data,
					&buffer_byte_index,
					&buffer_bit_index,
					length_bits
					);

			for (u64 idx = 0; idx < match_length; ++idx) {
				buffer[buffer_index + idx] = buffer[buffer_index - match_offset + idx];
			}
			buffer_index += match_length;

			lzss_buffer[lzss_buffer_size]     = match_offset;
			lzss_buffer[lzss_buffer_size + 1] = match_length;
			lzss_buffer_size += 2;
		}
		else {
			buffer[buffer_index] = read_bits(
					(char*)stream->data,
					&buffer_byte_index,
					&buffer_bit_index,
					8
					);
			literal_buffer[literal_buffer_size] = buffer[buffer_index];

			literal_buffer_size++;
			buffer_index++;
		}
	}

	// Realloc
	literal_buffer = (char*)realloc(literal_buffer, literal_buffer_size);
	lzss_buffer    = (u16*)realloc(lzss_buffer, lzss_buffer_size * sizeof(u16));

	// Build huffman trees for literals and offsets.
	trees.root_literals = (Node*)malloc(sizeof(Node));
	trees.root_offsets  = (Node*)malloc(sizeof(Node));

	build_huffman_tree(literal_buffer, literal_buffer_size, *trees.root_literals);

	// Implement in u16
	build_huffman_tree_u16(lzss_buffer, lzss_buffer_size, *trees.root_offsets);

	u32 literal_codes[256] 		  = {0};
	u32 literal_code_lengths[256] = {0};
	gather_codes(*trees.root_literals, 0, 0, literal_codes, literal_code_lengths);

	u32 lzss_codes[65536] 		  = {0};
	u32 lzss_code_lengths[65536]  = {0};
	gather_codes(*trees.root_offsets, 0, 0, lzss_codes, lzss_code_lengths);

	/*
	printf("Literals:\n");
	print_codes(literal_codes, literal_code_lengths, 256);

	printf("\nOffsets:\n");
	print_codes(lzss_codes, lzss_code_lengths, 65536);
	*/

	// Reread the bitstream and compress the data using the huffman trees.
	stream->bit_index = 0;

	buffer_byte_index = 0;
	buffer_bit_index  = 0;

	u64 lzss_idx    = 0;
	u64 literal_idx = 0;

	u64 byte_index = 0;
	u8  bit_index  = 0;


	for (u64 idx = 0; idx < is_match_buffer_size; ++idx) {
		bool is_match = is_match_buffer[idx];

		write_bit(output, &byte_index, &bit_index, is_match);

		if (is_match) {
			if (lzss_idx + 1 >= lzss_buffer_size) {
				printf("LZSS index out of bounds!\n");
				exit(1);
			}
			// printf("LZSS Buffer value: %u;    LZSS index: %lu\n", lzss_buffer[lzss_idx], lzss_idx);
			write_bits(
					output,
					&byte_index,
					&bit_index,
					lzss_codes[lzss_buffer[lzss_idx]],
					lzss_code_lengths[lzss_buffer[lzss_idx]]
					);
			lzss_idx++;

			write_bits(
					output,
					&byte_index,
					&bit_index,
					lzss_codes[lzss_buffer[lzss_idx]],
					lzss_code_lengths[lzss_buffer[lzss_idx]]
					);
			lzss_idx++;
		}
		else {
			if (literal_idx >= literal_buffer_size) {
				printf("Literal index: %llu\n", literal_idx);
				printf("Literal buffer size: %llu\n", literal_buffer_size);
				printf("LZSS index: %llu\n", lzss_idx);
				printf("LZSS buffer size: %llu\n", lzss_buffer_size);
				exit(0);
			}
			write_bits(
					output, 
					&byte_index, 
					&bit_index, 
					literal_codes[(u8)literal_buffer[literal_idx]],
					literal_code_lengths[(u8)literal_buffer[literal_idx]]
					);
			literal_idx++;
		}
	}

	free(literal_buffer);
	free(lzss_buffer);
	free(is_match_buffer);

	*output_size = byte_index + (bit_index > 0);
	output = (char*)realloc(output, *output_size);

	return trees;
}

void  huffman_decompress_lzss_data(
		char* compressed_buffer,
		u64 compressed_size,
		LZSSTrees trees,
		char* output,
		u64* output_size
		) {

}
