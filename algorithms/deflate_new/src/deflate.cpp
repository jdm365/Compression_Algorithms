#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "lz77.h"
#include "huffman.h"
#include "deflate.h"

StateData deflate_compress(
		const char* data, 
		uint64_t size
		) {
	StateData state_data;

	clock_t start = clock();

	BitStream* bit_stream = lz77_compress(data, size);

	state_data.lz77_compressed_size = (bit_stream->bit_index / 8) + (bit_stream->bit_index % 8 != 0);
	printf("LZ77_compressed_size: %lu\n", state_data.lz77_compressed_size);
	printf("LZ77 compression time: %f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("LZ77 MB/s: %f\n", (double)size / (1024 * 1024) / ((double)(clock() - start) / CLOCKS_PER_SEC));
	start = clock();

	char* huffman_compressed_data = (char*)malloc(state_data.lz77_compressed_size);
	uint64_t output_size = state_data.lz77_compressed_size;

	BitWriter bit_writer;

	Node huffman_root = huffman_compress(
			// huffman_compressed_data,
			(char*)bit_stream->data,
			output_size,
			&bit_writer
			);
	state_data.huffman_writer = &bit_writer;

	output_size = bit_writer.buffer_size;

	printf("Huffman_compressed_size: %lu\n", output_size);
	printf("Huffman compression time: %f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("Huffman MB/s: %f\n", (double)output_size / (1024 * 1024) / ((double)(clock() - start) / CLOCKS_PER_SEC));
	fflush(stdout);
	exit(0);

	return state_data;
}


char* deflate_decompress(
		StateData state_data,
		uint64_t*  decompressed_size
		) {
	uint64_t huffman_decompressed_size = state_data.lz77_compressed_size;
	char* huffman_decompressed_data = (char*)malloc(huffman_decompressed_size);

	huffman_decompress(
			state_data.huffman_writer,
			*state_data.huffman_tree,
			huffman_decompressed_data,
			&huffman_decompressed_size
			);

	BitStream* bit_stream = (BitStream*)malloc(sizeof(BitStream));
	bit_stream->data = (uint8_t*)huffman_decompressed_data;
	bit_stream->bit_index = 0;

	char* lz77_decompressed_data = lz77_decompress(
			bit_stream,
			state_data.lz77_compressed_size,
			decompressed_size
			);

	free(bit_stream);

	return lz77_decompressed_data;
}


void print_bit_string(char* buffer, uint64_t size) {
	for (uint64_t idx = 0; idx < size; ++idx) {
		for (int bit = 7; bit >= 0; --bit) {
			bool is_set = buffer[idx] & (1 << bit);
			printf("%d", is_set ? 1 : 0);
		}
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
	size_t _ = fread(buffer, 1, *size, file);
	fclose(file);

	return buffer;
}

bool compare_buffers(char* buffer1, char* buffer2, uint64_t size) {
	uint64_t num_incorrect = 0;
	for (uint64_t idx = 0; idx < size; ++idx) {
		if (buffer1[idx] != buffer2[idx]) {
			++num_incorrect;
		}
	}
	printf("Number of incorrect bytes: %lu\n", num_incorrect);
	return (num_incorrect == 0);
}
