#include <stdio.h>

#include "huffman.h"

bool compare_buffers(
		char* buffer1,
		char* buffer2,
		u64 size
		) {
	u64 num_mismatches = 0;
	for (u64 idx = 0; idx < size; ++idx) {
		if (buffer1[idx] != buffer2[idx]) {
			num_mismatches++;
		}
	}
	printf("Number of mismatches: %d\n", (int)num_mismatches);
	return (num_mismatches == 0);
}


int main() {
	// const char* FILENAME = "../../data/declaration_of_independence.txt";
	const char* FILENAME = "../../data/enwik6";
	// const char* FILENAME = "../../data/enwik7";
	// const char* FILENAME = "../../data/enwik8";
	// const char* FILENAME = "../../data/enwik9";
	u64 filesize;

	char* buffer = read_input_buffer(FILENAME, &filesize);

	printf("File size uncompressed: %d\n", (int)filesize);

	clock_t start = clock();

	Node root(0, 0);
	build_huffman_tree(buffer, filesize, root);

	u32 codes[256] = {0};
	u32 code_lengths[256] = {0};
	gather_codes(root, 0, 0, codes, code_lengths);

	char* compressed_buffer = (char*)malloc(filesize);
	u64   compressed_bytes = 0;
	huffman_compress(
			buffer, 
			filesize, 
			codes,
			code_lengths,
			compressed_buffer,
			&compressed_bytes
			);

	// Resize output buffer
	compressed_buffer = (char*)realloc(compressed_buffer, compressed_bytes);

	char* decompressed_buffer = (char*)malloc(filesize);
	u64   decompressed_bytes  = filesize;
	huffman_decompress(
			compressed_buffer,
			compressed_bytes,
			root, 
			decompressed_buffer, 
			&decompressed_bytes
			);
	decompressed_buffer = (char*)realloc(decompressed_buffer, decompressed_bytes);

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
	printf("Compressed size:    %lu\n", compressed_bytes);
	printf("Reconstructed size: %lu\n", decompressed_bytes);
	printf("Compression ratio:  %f\n",  (double)filesize / compressed_bytes);
	printf("Total time:         %fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("========================================================================\n");

	free(buffer);
	free(compressed_buffer);
	free(decompressed_buffer);
	return 0;
}
