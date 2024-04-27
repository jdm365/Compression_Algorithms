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

	/*
	uint8_t* compressed_buffer = (uint8_t*)malloc(filesize);
	uint64_t compressed_bytes;
	Node root = huffman_compress(buffer, filesize, compressed_buffer, &compressed_bytes);
	printf("Compression MB/s: %f\n", (double)filesize / (double)(clock() - start) * CLOCKS_PER_SEC / (1024.0 * 1024.0));
	printf("Compressed size: %lu\n\n", compressed_bytes);
	*/

	clock_t start_v2 = clock();

	BitWriter bit_writer;
	Node root2 = huffman_compress_v2(
			buffer, 
			filesize, 
			&bit_writer
			);
	printf("Compression MB/s: %f\n", (double)filesize / (double)(clock() - start_v2) * CLOCKS_PER_SEC / (1024.0 * 1024.0));
	printf("Compressed size v2: %lu\n\n", bit_writer.buffer_size);

	// Print the two compressed buffers
	/*
	for (uint64_t idx = 0; idx < compressed_bytes; ++idx) {
		printf("v1: "); print_bit_string((uint8_t*)&compressed_buffer[idx], 1);
		printf("v2: "); print_bit_string((uint8_t*)&bit_writer.buffer[idx], 1);
		printf("\n");
	}
	exit(1);
	*/

	clock_t start2 = clock();

	/*
	char* decompressed_buffer = (char*)malloc(filesize);
	uint64_t decompressed_bytes  = filesize;
	huffman_decompress(
			compressed_buffer,
			compressed_bytes,
			root, 
			decompressed_buffer, 
			&decompressed_bytes
			);
	printf("Decompressed size: %lu\n", decompressed_bytes);

	printf("Decompression MB/s: %f\n\n", (double)filesize / (double)(clock() - start2) * CLOCKS_PER_SEC / (1024.0 * 1024.0));
	*/

	char* decompressed_buffer2 = (char*)malloc(filesize);
	uint64_t decompressed_bytes2  = filesize;
	huffman_decompress_v2(
			&bit_writer,
			root2,
			decompressed_buffer2,
			&decompressed_bytes2
			);
	decompressed_buffer2 = (char*)realloc(decompressed_buffer2, decompressed_bytes2);
	printf("Decompressed size v2: %lu\n", decompressed_bytes2);

	printf("Decompression MB/s: %f\n\n", (double)filesize / (double)(clock() - start2) * CLOCKS_PER_SEC / (1024.0 * 1024.0));

	// printf("Decompressed buffer 1: %s\n", decompressed_buffer);
	// printf("Decompressed buffer 2: %s\n", decompressed_buffer2);
	/*
	for (uint64_t idx = 0; idx < decompressed_bytes2; ++idx) {
		if (idx % 1048576 == 0) {
			sleep(8);
		}
		printf("%c", decompressed_buffer2[idx]);
	}
	*/

	if (compare_buffers(buffer, decompressed_buffer2, filesize)) {
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
	printf("Reconstructed size: %lu\n", decompressed_bytes2);
	printf("Compression ratio:  %f\n",  (double)filesize / bit_writer.buffer_size);
	printf("Total time:         %fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("========================================================================\n");
	exit(1);

	free(buffer);
	free(decompressed_buffer2);
	return 0;
}
