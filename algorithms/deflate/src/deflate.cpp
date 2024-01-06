#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "lz77.h"
#include "huffman.h"
#include "deflate.h"

StateData deflate_compress(
		char* data, 
		u64   size, 
		u8    window_bits,
		u8    length_bits
		) {
	StateData state_data;

	clock_t start = clock();

	BitStream* bit_stream = lz77_compress(data, size, window_bits, length_bits);

	state_data.lz77_compressed_size = bit_stream->bit_index / 8;
	char* huffman_compressed_data = (char*)malloc(state_data.lz77_compressed_size);
	printf("LZ77_compressed_size: %lu\n", state_data.lz77_compressed_size);

	printf("LZ77 compression time: %f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("LZ77 MB/s: %f\n", (double)size / (1024 * 1024) / ((double)(clock() - start) / CLOCKS_PER_SEC));
	start = clock();

	char* output_buffer = (char*)malloc(state_data.lz77_compressed_size);
	u64   output_size   = state_data.lz77_compressed_size;

	LZSSTrees trees = huffman_compress_lzss_data(
			bit_stream,
			size,
			output_buffer,
			&output_size,
			window_bits,
			length_bits
			);
	printf("Huffman_compressed_size: %llu\n", output_size);
	printf("Huffman compression time: %f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("Huffman MB/s: %f\n", (double)output_size / (1024 * 1024) / ((double)(clock() - start) / CLOCKS_PER_SEC));

	state_data.huffman_compressed_data = output_buffer;
	state_data.huffman_compressed_size = output_size;
	state_data.huffman_trees 		   = &trees;

	free(bit_stream->data);
	free(bit_stream);
	free(output_buffer);
	free(data);

	return state_data;
}


char* deflate_decompress(
		StateData state_data,
		u64*  decompressed_size,
		u8    window_bits,
		u8    length_bits
		) {
	u64   huffman_decompressed_size = state_data.lz77_compressed_size;
	char* huffman_compressed_data = (char*)malloc(huffman_decompressed_size);

	huffman_decompress_lzss_data(
			state_data.huffman_compressed_data,
			state_data.huffman_compressed_size,
			*state_data.huffman_trees,
			huffman_compressed_data,
			&huffman_decompressed_size
			);

	BitStream* bit_stream = (BitStream*)malloc(sizeof(BitStream));
	bit_stream->data = (u8*)huffman_compressed_data;
	bit_stream->bit_index = 0;

	char* lz77_decompressed_data = lz77_decompress(
			bit_stream,
			state_data.lz77_compressed_size,
			decompressed_size,
			window_bits,
			length_bits
			);

	free(huffman_compressed_data);
	free(bit_stream);

	return lz77_decompressed_data;
}


void print_bit_string(char* buffer, u64 size) {
	for (u64 idx = 0; idx < size; ++idx) {
		for (int bit = 7; bit >= 0; --bit) {
			bool is_set = buffer[idx] & (1 << bit);
			printf("%d", is_set ? 1 : 0);
		}
	}
	printf("\n");
}

char* read_input_buffer(
		const char* filename,
		u64* size
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

bool compare_buffers(char* buffer1, char* buffer2, u64 size) {
	u64 num_incorrect = 0;
	for (u64 idx = 0; idx < size; ++idx) {
		if (buffer1[idx] != buffer2[idx]) {
			++num_incorrect;
		}
	}
	printf("Number of incorrect bytes: %lu\n", num_incorrect);
	return (num_incorrect == 0);
}
