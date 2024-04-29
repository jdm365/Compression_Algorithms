#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "lz77.h"


int main() {
	uint64_t filesize;

	// const char* FILENAME = "../../data/declaration_of_independence.txt";
	// const char* FILENAME = "../../data/enwik6";
	// const char* FILENAME = "../../data/enwik7";
	const char* FILENAME = "../../data/enwik8";
	// const char* FILENAME = "../../data/enwik9";
	char* buffer = read_input_buffer(FILENAME, &filesize);

	const uint64_t NUM_PRINT = min(100, (int)filesize);

	// Time the compression
	clock_t start = clock();

	// BitStream* compressed_buffer = lz77_compress(buffer, filesize);
	BitStream* compressed_buffer = lz77_compress_hash_array(buffer, filesize);
	uint64_t compressed_bytes = compressed_buffer->bit_index / 8;
	printf("Compression MB/s: %f\n", (double)filesize / (double)(clock() - start) * CLOCKS_PER_SEC / (1024.0 * 1024.0));

	start = clock();
	uint64_t   decompressed_size = filesize;
	char* decompressed_buffer = lz77_decompress(
			compressed_buffer, 
			filesize, 
			&decompressed_size
			);
	printf("Decompression MB/s: %f\n", (double)filesize / (double)(clock() - start) * CLOCKS_PER_SEC / (1024.0 * 1024.0));

	printf("Decompressed buffer: ");
	/*
	for (uint64_t idx = 0; idx < NUM_PRINT; ++idx) {
		printf("%c", decompressed_buffer[idx]);
	}
	*/
	for (uint64_t idx = 0; idx < decompressed_size; ++idx) {
		if (decompressed_buffer[idx] != buffer[idx]) {
			printf("DECOMPRESSED: %c\n", decompressed_buffer[idx]);
			printf("ORIG:         %c\n", buffer[idx]);
			printf("IDX:          %lu\n", idx);
		}
	}
	printf("\n\n");

	if (check_buffer_equivalence(buffer, decompressed_buffer, filesize)) {
		printf("SUCCESS\n");
	}
	else {
		printf("FAILURE\n");
	}

	printf("========================================================================\n");
	printf("============================== LZ77 ====================================\n");
	printf("========================================================================\n");
	printf("Uncompressed size:  %lu\n", filesize);
	printf("Compressed size:    %lu\n", compressed_bytes);
	printf("Reconstructed size: %lu\n", decompressed_size);
	printf("Compression ratio:  %f\n",  (double)filesize / compressed_bytes);
	printf("Total time:         %fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("========================================================================\n");

	free(buffer);
	free(decompressed_buffer);
	free(compressed_buffer->data);
	free(compressed_buffer);
	return 0;
}
