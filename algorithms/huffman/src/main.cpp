#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "huffman.h"

bool compare_buffers(
		char* buffer1,
		char* buffer2,
		uint64_t size
		) {
	uint64_t num_mismatches = 0;
	for (uint64_t idx = 0; idx < size; ++idx) {
		if (buffer1[idx] != buffer2[idx]) {
			num_mismatches++;
		}
	}
	printf("Number of mismatches: %d\n", (int)num_mismatches);
	return (num_mismatches == 0);
}

char* get_test_buffer(uint64_t* size) {
	const char* test_string = "nine times";
	*size = strlen(test_string);
	char* buffer = (char*)malloc(*size);
	memcpy(buffer, test_string, *size);
	return buffer;
}


int main() {
	// const char* FILENAME = "../../data/declaration_of_independence.txt";
	// const char* FILENAME = "../../data/enwik6";
	// const char* FILENAME = "../../data/enwik7";
	const char* FILENAME = "../../data/enwik8";
	// const char* FILENAME = "../../data/enwik9";
	uint64_t filesize;

	char* buffer = read_input_buffer(FILENAME, &filesize);
	// char* buffer = get_test_buffer(&filesize);

	printf("File size uncompressed: %d\n\n", (int)filesize);

	clock_t start = clock();

	BitWriter bit_writer;
	Node root = huffman_compress(
			buffer, 
			filesize, 
			&bit_writer
			);
	printf("Compression MB/s: %f\n", (double)filesize / (double)(clock() - start) * CLOCKS_PER_SEC / (1024.0 * 1024.0));
	printf("Compressed size: %lu\n\n", bit_writer.buffer_size);

	// Print the two compressed buffers
	/*
	for (uint64_t idx = 0; idx < compressed_bytes; ++idx) {
		printf("v1: "); print_bit_string((uint8_t*)&compressed_buffer[idx], 1);
		printf("v2: "); print_bit_string((uint8_t*)&bit_writer.buffer[idx], 1);
		printf("\n");
	}
	exit(1);
	*/

	char* decompressed_buffer   = (char*)malloc(filesize);
	uint64_t decompressed_bytes = filesize;
	huffman_decompress(
	// huffman_decompress_lookup_table(
			&bit_writer,
			root,
			decompressed_buffer,
			&decompressed_bytes
			);
	decompressed_buffer = (char*)realloc(decompressed_buffer, decompressed_bytes);
	printf("Decompressed size: %lu\n", decompressed_bytes);

	printf("Decompression MB/s: %f\n\n", (double)filesize / (double)(clock() - start) * CLOCKS_PER_SEC / (1024.0 * 1024.0));

	if (compare_buffers(buffer, decompressed_buffer, filesize)) {
		printf("SUCCESS\n");
	} 
	else {
		printf("FAILURE\n");
	}

	printf("========================================================================\n");
	printf("============================ HUFFMAN ===================================\n");
	printf("========================================================================\n");
	printf("Uncompressed size:  %lu\n", filesize);
	printf("Compressed size:    %lu\n", bit_writer.buffer_size);
	printf("Reconstructed size: %lu\n", decompressed_bytes);
	printf("Compression ratio:  %f\n",  (double)filesize / bit_writer.buffer_size);
	printf("Total time:         %fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("========================================================================\n");
	exit(1);

	free(buffer);
	free(decompressed_buffer);
	return 0;
}
