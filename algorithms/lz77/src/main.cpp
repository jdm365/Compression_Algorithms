#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <time.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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
	fread(buffer, 1, *size, file);
	fclose(file);


	return buffer;
}

typedef struct {
    u8* data;
    u64 bit_position;
} BitStream;

void init_bitstream(BitStream* stream, u8* buffer) {
    stream->data 		 = buffer;
    stream->bit_position = 0;
}

void write_bits(BitStream* stream, u64 value, int bit_count) {
    for (int i = bit_count - 1; i >= 0; --i) {
        u64 bit = (value >> i) & 1;
        stream->data[stream->bit_position / 8] |= bit << (7 - (stream->bit_position % 8));
        stream->bit_position++;
    }
}

u64 read_bits(BitStream* stream, int bit_count) {
	u64 value = 0;
	for (int i = bit_count - 1; i >= 0; --i) {
		u64 bit = (stream->data[stream->bit_position / 8] >> (7 - (stream->bit_position % 8))) & 1;
		value |= bit << i;
		stream->bit_position++;
	}
	return value;
}

void resize_bitstream(BitStream* stream, u64 new_size) {
	stream->data = (u8*)realloc(stream->data, new_size);
}


BitStream* compress(char* buffer, u64 size, u64 window_size) {
	u64 buffer_capacity = 1.25f * size;

	BitStream* compressed_bit_stream = (BitStream*)malloc(sizeof(BitStream));
	init_bitstream(compressed_bit_stream, (u8*)malloc(buffer_capacity));

    for (u64 idx = 0; idx < size; ++idx) {
        u64 max_length = 0;
        u64 max_offset = 0;
        char next_char = buffer[idx];

        // Calculate the start of the search window
        u64 window_start = idx > window_size ? idx - window_size : 0;

        for (u64 offset = window_start; offset < idx; ++offset) {
            u64 length = 0;
            while (idx + length < size && buffer[idx + length] == buffer[offset + length]) {
                ++length;
                if (length == window_size) {
                    break;
                }
            }

			u64 current_bit_position = compressed_bit_stream->bit_position;
			if (current_bit_position / 8 >= buffer_capacity - 24) {
				buffer_capacity *= 2;
				compressed_bit_stream->data = (u8*)realloc(compressed_bit_stream->data, buffer_capacity);
			}

            if (length > max_length) {
                max_length = length;
                max_offset = idx - offset;
                next_char  = idx + length < size ? buffer[idx + length] : '\0';
            }
        }

        if (max_length > 0) {
			write_bits(compressed_bit_stream, max_offset, 16);
			write_bits(compressed_bit_stream, max_length, 4);
			write_bits(compressed_bit_stream, next_char, 8);
            idx += max_length - 1;
        }
        else {
			write_bits(compressed_bit_stream, 0, 16);
			write_bits(compressed_bit_stream, 0, 4);
			write_bits(compressed_bit_stream, next_char, 8);
        }
    }

	u64 final_size = compressed_bit_stream->bit_position + 7 / 8;
	resize_bitstream(compressed_bit_stream, final_size);
	
	return compressed_bit_stream;
}


void decompress(BitStream* compressed_bit_stream, char* out_buffer, u64* out_size) {
    u64 buffer_idx = 0;
	u64 bit_capacity = compressed_bit_stream->bit_position;
	printf("bit_capacity: %lu\n", bit_capacity);
    compressed_bit_stream->bit_position = 0;

    while (compressed_bit_stream->bit_position < bit_capacity) {
        u64  offset    = read_bits(compressed_bit_stream, 16);
        u64  length    = read_bits(compressed_bit_stream, 4);
        char next_char = read_bits(compressed_bit_stream, 8);

        if (length > 0) {
            if (buffer_idx < offset) {
				print_bit_string((char*)&offset, 8);
				printf("buffer_idx: %lu, offset: %lu, length: %lu, next_char: %c\n", buffer_idx, offset, length, next_char);
                fprintf(stderr, "Error: Invalid offset in decompression\n");
				exit(1);
            }
            for (u64 idx = 0; idx < length; ++idx) {
                out_buffer[buffer_idx++] = out_buffer[buffer_idx - offset + idx];
            }
        }
        out_buffer[buffer_idx++] = next_char;
    }

    *out_size = buffer_idx;
}

bool check_buffer_equivalence(char* buffer1, char* buffer2, u64 size) {
	for (u64 idx = 0; idx < size; ++idx) {
		if (buffer1[idx] != buffer2[idx]) {
			return false;
		}
	}
	return true;
}


int main() {
	const char* FILENAME = "../../data/declaration_of_independence.txt";
	// const char* FILENAME = "../../data/enwik8";
	u64 filesize;
	
	char* buffer = read_input_buffer(FILENAME, &filesize);

	const int NUM_PRINT = std::min(100, (int)filesize);
	const int WINDOW_SIZE = 64;
	// const int WINDOW_SIZE = 256;
	// const int WINDOW_SIZE = 4096;
	// const int WINDOW_SIZE = 32768;
	// const int WINDOW_SIZE = 65535;
	
	printf("Contents of file:    ");
	for (u64 idx = 0; idx < NUM_PRINT; ++idx) {
		printf("%c", buffer[idx]);
	}
	printf("\n\n");

	// Time the compression
	clock_t start = clock();

	BitStream* compressed_buffer = compress(buffer, filesize, WINDOW_SIZE);

	// u64 compressed_bytes = compressed_buffer->bit_position / 8 + 1;
	u64 compressed_bytes = compressed_buffer->bit_position / 8;

	printf("Compressed buffer:   ");
	for (u64 idx = 0; idx < NUM_PRINT; ++idx) {
		printf("%c", compressed_buffer->data[idx]);
	}
	printf("\n\n");

	char* decompressed_buffer = (char*)malloc(filesize);
	u64   decompressed_size;

	decompress(compressed_buffer, decompressed_buffer, &decompressed_size);
	printf("Decompressed buffer: ");
	for (u64 idx = 0; idx < filesize; ++idx) {
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
	printf("============================ LZ77 ======================================\n");
	printf("========================================================================\n");
	printf("Uncompressed size:  %lu\n", filesize);
	printf("Compressed size:    %lu\n", compressed_bytes);
	printf("Reconstructed size: %lu\n", decompressed_size);
	printf("Compression ratio:  %f\n",  (double)compressed_bytes / (double)filesize);
	printf("Total time:         %fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("========================================================================\n");

	free(buffer);
	free(decompressed_buffer);
	free(compressed_buffer->data);
	free(compressed_buffer);
	return 0;
}
