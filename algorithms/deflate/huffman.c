#include <stdlib.h>
#include <string.h>

#include "huffman.h"


void init_bitwriter(BitWriter* writer, uint64_t buffer_size) {
	writer->buffer      = (uint32_t*)malloc(buffer_size);
	memset(writer->buffer, 0, buffer_size);
	writer->word_idx    = 0;
	writer->bit_idx     = 0;
	writer->buffer_size = buffer_size;
}


void write_bits(BitWriter* writer, uint32_t bits, uint8_t length) {
	int8_t nbits_rem_word_0 = 32 - writer->bit_idx;
	int8_t shift = nbits_rem_word_0 - length;

	if (shift >= 0) {
		if (shift == 0) {
			writer->buffer[writer->word_idx] |= bits;
			++(writer->word_idx);
			writer->bit_idx = 0;
			return;
		}

		uint32_t word_0_bits = bits << shift;
		word_0_bits &= BIT_MASK[nbits_rem_word_0];
		writer->buffer[writer->word_idx] |= word_0_bits;
		writer->bit_idx += length;
	}
	else {
		uint32_t word_0_bits = bits >> -shift;
		word_0_bits &= BIT_MASK[nbits_rem_word_0];
		writer->buffer[writer->word_idx] |= word_0_bits;

		++(writer->word_idx);
		writer->bit_idx += length;
		writer->bit_idx %= 32;

		shift = 32 + shift;
		uint32_t word_1_bits = (bits << shift);
		writer->buffer[writer->word_idx] |= word_1_bits;
	}
}


void append_huffman_tree_literal(
		uint32_t* frequencies,
		char literal
		) {
	++frequencies[(uint8_t)literal];
}

void append_huffman_tree_pair(
		uint32_t* frequencies,
		uint16_t offset
		) {
	uint8_t bits_needed = __builtin_clz(offset) - 16;
	++frequencies[256 + bits_needed];
}

void gather_codes(
		MinHeapNode* root,
		uint16_t code,
		uint8_t length,
		uint16_t* codes,
		uint8_t*  code_lengths
		) {
	if (root->left == NULL && root->right == NULL) {
		codes[root->data] 	     = code;
		code_lengths[root->data] = length;
		return;
	}
	code <<= 1;

	if (root->left != NULL) {
		gather_codes(
				root->left, 
				code,
				length + 1, 
				codes, 
				code_lengths
				);
	}

	if (root->right != NULL) {
		gather_codes(
				root->right, 
				code + 1,
				length + 1, 
				codes, 
				code_lengths
				);
	}
}
