#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "lz77.h"
#include "deflate.h"

StateData compress(const char* input_filename) {
	const char* filename = strrchr(input_filename, '/'); ++filename;

	HashTableArray table;
	init_hash_table(&table);

	StateData state_data = {
		.table = &table,
		.huffman_root = NULL,
		.compressed_filename = (char*)malloc(strlen(filename) + strlen(extension) + 1)
	};
	strcpy(state_data.compressed_filename, filename);
	strcat(state_data.compressed_filename, extension);

	FILE* input_file  = fopen(input_filename, "rb");
	if (input_file == NULL) {
		fprintf(stderr, "Error: could not open file %s\n", input_filename);
		exit(1);
	}

	FILE* output_file = fopen(state_data.compressed_filename, "wb");
	if (output_file == NULL) {
		fprintf(stderr, "Error: could not open file %s\n", state_data.compressed_filename);
		exit(1);
	}

	// dump the input file into the output file
	char* buffer = (char*)malloc(BUFFER_SIZE);
	char* compressed_buffer = (char*)malloc(2 * BUFFER_SIZE);
	uint64_t compressed_buffer_size = 0;
	size_t read_bytes;

	size_t total_read_bytes = 0;
	// size_t total_written_bytes = 0;

	clock_t start = clock();

	while ((read_bytes = fread(buffer, 1, BUFFER_SIZE, input_file)) > 0) {
		lz77_compress(
				buffer, 
				read_bytes, 
				compressed_buffer, 
				&compressed_buffer_size,
				state_data.table
				);

		fwrite(compressed_buffer, 1, compressed_buffer_size, output_file);

		total_read_bytes    += read_bytes;
		// total_written_bytes += compressed_buffer_size;

		// printf("Total Read bytes:    %lu\n", total_read_bytes);
		// printf("Total Written bytes: %lu\n", total_written_bytes);
	}

	printf("MB/s: %f\n", (double)total_read_bytes / (1024 * 1024) / ((double)(clock() - start) / CLOCKS_PER_SEC));

	fclose(input_file);
	fclose(output_file);

	free(buffer);
	free(compressed_buffer);
	free(table.buckets);
	return state_data;
}

void decompress(StateData* state_data, const char* input_filename) {
}

void init_huffman_node(HuffmanNode* node) {
	node->left  = NULL;
	node->right = NULL;
	node->value = 0;
	node->frequency = 0;
}

void destroy_huffman_node(HuffmanNode* node) {
	if (node->left != NULL) {
		destroy_huffman_node(node->left);
	}
	if (node->right != NULL) {
		destroy_huffman_node(node->right);
	}

	free(node->left);
	free(node->right);
}

bool compare_huffman_node(const HuffmanNode* a, const HuffmanNode* b) {
	return a->frequency < b->frequency;
}
