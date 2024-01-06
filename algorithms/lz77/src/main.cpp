#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <time.h>

#include "lz77.h"

int main() {
	u64 filesize;

	// const char* FILENAME = "../../data/declaration_of_independence.txt";
	const char* FILENAME = "../../data/enwik6";
	// const char* FILENAME = "../../data/enwik7";
	// const char* FILENAME = "../../data/enwik8";
	// const char* FILENAME = "../../data/enwik9";
	char* buffer = read_input_buffer(FILENAME, &filesize);

	const u64 NUM_PRINT = std::min(100, (int)filesize);

	const u8 LENGTH_BITS = 5;
	const u8 WINDOW_BITS = 12;

	// Time the compression
	clock_t start = clock();

	BitStream* compressed_buffer = lz77_compress(buffer, filesize, WINDOW_BITS, LENGTH_BITS);
	u64 compressed_bytes = compressed_buffer->bit_index / 8;

	printf("Compression MB/s: %f\n", (double)filesize / (double)(clock() - start) * CLOCKS_PER_SEC / (1024.0 * 1024.0));

	u64   decompressed_size = filesize;
	char* decompressed_buffer = lz77_decompress(
			compressed_buffer, 
			filesize, 
			&decompressed_size, 
			WINDOW_BITS,
			LENGTH_BITS
			);

	printf("Decompressed buffer: ");
	for (u64 idx = 0; idx < NUM_PRINT; ++idx) {
		printf("%c", decompressed_buffer[idx]);
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
	printf("Uncompressed size:  %llu\n", filesize);
	printf("Compressed size:    %llu\n", compressed_bytes);
	printf("Reconstructed size: %llu\n", decompressed_size);
	printf("Compression ratio:  %f\n",  (double)filesize / compressed_bytes);
	printf("Total time:         %fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("========================================================================\n");

	free(buffer);
	free(decompressed_buffer);
	free(compressed_buffer->data);
	free(compressed_buffer);
	return 0;
}
