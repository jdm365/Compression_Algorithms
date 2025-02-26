#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

void print_bit_string(uint8_t* buffer, uint64_t size) {
	for (uint64_t idx = 0; idx < size; ++idx) {
		for (int bit = 7; bit >= 0; --bit) {
			bool is_set = buffer[idx] & (1 << bit);
			printf("%d", is_set ? 1 : 0);
		}
		printf(" ");
	}
	printf("\n");
}

char* read_input_buffer(
		const char* filename,
		uint64_t* size
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

PriorityQueue* init_priority_queue(
		uint64_t capacity
		) {
	PriorityQueue* queue = (PriorityQueue*)malloc(sizeof(PriorityQueue));
	queue->nodes = (Node**)malloc(capacity * sizeof(Node*));
	queue->size = 0;
	queue->capacity = capacity;

	return queue;
}

void swap_nodes(
		Node** a,
		Node** b
		) {
	Node* temp = *a;
	*a = *b;
	*b = temp;
}

void heapify_up(
		PriorityQueue* queue,
		uint64_t idx
		) {
	uint64_t parent = (idx - 1) / 2;
	while (idx > 0 && queue->nodes[idx]->frequency < queue->nodes[parent]->frequency) {
		swap_nodes(&queue->nodes[idx], &queue->nodes[parent]);
		idx = parent;
		parent = (idx - 1) / 2;
	}
}

void heapify_down(
		PriorityQueue* queue,
		uint64_t idx
		) {
	uint64_t left = 2 * idx + 1;
	uint64_t right = 2 * idx + 2;
	uint64_t smallest = idx;

	if (left < queue->size && queue->nodes[left]->frequency < queue->nodes[smallest]->frequency) {
		smallest = left;
	}
	if (right < queue->size && queue->nodes[right]->frequency < queue->nodes[smallest]->frequency) {
		smallest = right;
	}

	if (smallest != idx) {
		swap_nodes(&queue->nodes[idx], &queue->nodes[smallest]);
		heapify_down(queue, smallest);
	}
}

void enqueue(
		PriorityQueue* queue,
		Node* node
		) {
	if (queue->size == queue->capacity) {
		printf("ERROR: Queue is full\n");
		exit(1);
	}

	queue->nodes[queue->size++] = node;
	heapify_up(queue, queue->size - 1);
}

Node* dequeue(
		PriorityQueue* queue
		) {
	if (queue->size == 0) {
		printf("ERROR: Queue is empty\n");
		exit(1);
	}

	Node* node = queue->nodes[0];
	queue->nodes[0] = queue->nodes[--queue->size];
	heapify_down(queue, 0);

	return node;
}

bool is_empty(PriorityQueue* queue) {
	return queue->size == 0;
}

Node* init_node(
		uint8_t value,
		uint32_t frequency
		) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->value = value;
	node->frequency = frequency;
	node->left = NULL;
	node->right = NULL;

	return node;
}


void build_huffman_tree(
		char* buffer,
		uint64_t size,
		Node** root
		) {
	uint32_t frequencies[256] = {0};
	for (uint64_t idx = 0; idx < size; ++idx) {
		++frequencies[(uint8_t)buffer[idx]];
	}

	PriorityQueue* queue = init_priority_queue(256);

	for (uint8_t idx = 0; idx < 255; ++idx) {
		if (frequencies[idx] > 0) {
			enqueue(queue, init_node(idx, frequencies[idx]));
		}
	}
	if (frequencies[255] > 0) {
		enqueue(queue, init_node(255, frequencies[255]));
	}

	while (queue->size > 1) {
		Node* left  = dequeue(queue);
		Node* right = dequeue(queue);

		Node* parent = init_node(0, left->frequency + right->frequency);
		parent->left = left;
		parent->right = right;

		enqueue(queue, parent);
	}

	*root = dequeue(queue);

	free(queue->nodes);
	free(queue);
}

void gather_codes(
		Node* root,
		uint32_t code,
		uint32_t length,
		uint32_t* codes,
		uint8_t*  code_lengths
		) {
	if (root->left == NULL && root->right == NULL) {
		codes[root->value] 	   = code;
		code_lengths[root->value] = length;
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

void print_codes(
		uint32_t* codes,
		uint8_t* code_lengths
		) {
	for (uint32_t idx = 0; idx < 256; ++idx) {
		if (code_lengths[idx] > 0) {
			printf("%c: ", (char)idx);
			for (uint32_t bit_idx = 0; bit_idx < code_lengths[idx]; ++bit_idx) {
				printf("%d", (codes[idx] >> (code_lengths[idx] - bit_idx - 1)) & 1);
			}
			printf("\n");
		}
	}
}

void _huffman_compress(
		char* buffer,
		uint64_t   size,
		uint32_t*  codes,
		uint8_t*   code_lengths,
		BitWriter* bit_writer
		) {
	for (uint64_t idx = 0; idx < size; ++idx) {
		uint32_t code   = codes[(uint8_t)buffer[idx]];
		uint8_t  length = code_lengths[(uint8_t)buffer[idx]];

		if (length == 0) {
			printf("ERROR: No code for character %c\n", buffer[idx]);
			exit(1);
		}

		write_bits(bit_writer, code, length);
	}
}


Node huffman_compress(
		char* buffer,
		uint64_t   size,
		BitWriter* bit_writer
		) {
	init_bitwriter(bit_writer, size);

	Node* root = init_node(0, 0);
	build_huffman_tree(buffer, size, &root);

	uint32_t codes[256]        = {0};
	uint8_t  code_lengths[256] = {0};
	gather_codes(root, 0, 0, codes, code_lengths);

	// Print max length
	uint8_t max_length = 0;
	for (uint32_t idx = 0; idx < 256; ++idx) {
		if (code_lengths[idx] > max_length) {
			max_length = code_lengths[idx];
		}
	}
	// printf("Max length: %d\n", max_length);

	_huffman_compress(
			buffer,
			size,
			codes,
			code_lengths,
			bit_writer
			);
	bit_writer->buffer_size = bit_writer->word_idx * 4 
						    + (bit_writer->bit_idx / 8) 
						    + (bit_writer->bit_idx % 8 > 0);

	bit_writer->buffer = (uint32_t*)realloc(
			bit_writer->buffer, 
			bit_writer->buffer_size
			);

	return *root;
}

void huffman_decompress(
        BitWriter* writer,
        Node* root,
        char* output,
        uint64_t* output_size
        ) {
    uint64_t char_idx = 0;
    uint64_t word_idx = 0;
    uint64_t byte_idx = 0;
    uint8_t  bit_idx  = 0;

    memset(output, 0, *output_size);

	uint32_t current_word = writer->buffer[0];
    do {
        Node* node = root;
        while (node->left != NULL && node->right != NULL) {
            // Extract the correct bit from the current 32-bit word
            if (current_word & (1 << (31 - bit_idx))) {
                node = node->right;
            } else {
                node = node->left;
            }
            if (++bit_idx == 32) {
                bit_idx = 0;
				current_word = writer->buffer[++word_idx];
            }
			byte_idx += (bit_idx % 8) == 0;
        }

        output[char_idx++] = node->value;
    } while (byte_idx < writer->buffer_size);

    *output_size = char_idx;
}

void huffman_decompress_lookup_table(
        BitWriter* writer,
        Node* root,
        char* output,
        uint64_t* output_size
        ) {
	uint32_t lookup_table[256] = {0};
	uint8_t  lookup_table_lengths[256] = {0};
	gather_codes(root, 0, 0, lookup_table, lookup_table_lengths);

    uint64_t char_idx = 0;
    uint64_t word_idx = 0;

    memset(output, 0, *output_size);

    do {
		uint32_t current_word = writer->buffer[word_idx];
		uint32_t mask = 0x80000000;
		for (uint8_t idx = 0; idx < 32; ++idx) {
			uint32_t word = current_word & mask;
			uint32_t code = lookup_table[word];
			uint8_t  length = lookup_table_lengths[word];

			if (length == 0) {
				printf("ERROR: No code for word %d\n", word);
				exit(1);
			}

			write_bits(writer, code, length);
			mask >>= 1;
		}
		++word_idx;
	} while (word_idx < writer->buffer_size);
	
	*output_size = char_idx;
}
