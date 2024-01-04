#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

#include "lz77.h"

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

void init_bitstream(BitStream* stream, u8* buffer) {
    stream->data 	  = buffer;
    stream->bit_index = 0;
}

inline void write_bit(BitStream* stream, bool bit) {
	u64 byte_index = stream->bit_index / 8;
	u64 bit_offset = stream->bit_index % 8;

	if (bit) {
		stream->data[byte_index] |= (1 << bit_offset);
	}
	else {
		stream->data[byte_index] &= ~(1 << bit_offset);
	}

	stream->bit_index++;
}
inline bool read_bit(BitStream* stream) {
	u64 byte_index = stream->bit_index / 8;
	u64 bit_offset = stream->bit_index % 8;

	bool is_set = (stream->data[byte_index] >> bit_offset) & 1;
	stream->bit_index++;

	return is_set;
}

void write_bits(BitStream* stream, u64 value, u64 num_bits) {
	for (u64 bit = 0; bit < num_bits; ++bit) {
		bool is_set = value & (1 << bit);
		write_bit(stream, is_set);
	}
}

u64 read_bits(BitStream* stream, u64 num_bits) {
	u64 value = 0;
	for (u64 bit = 0; bit < num_bits; ++bit) {
		if (read_bit(stream)) {
			value |= (1 << bit);
		}
	}
	return value;
}


BitStream* lz77_compress(
		char* buffer,
		u64 size,
		u8 window_bits,
		u8 length_bits
		) {
	BitStream* stream = (BitStream*)malloc(sizeof(BitStream));
	u64 buffer_size = size * 2;
	u64 buffer_index = 0;

	u64 window_size = pow(2, window_bits) - 1;

	u64 window_start = 0;
	u64 window_end   = 0;

	init_bitstream(stream, (u8*)malloc(buffer_size));

	while (buffer_index < size) {
		u64 match_length = 0;
		u64 match_offset = 0;

		window_start = std::max((u64)0, buffer_index - window_size);
		window_end   = buffer_index;

		for (u64 window_idx = window_start; window_idx < window_end; ++window_idx) {
			u64 match_idx  = window_idx;
			u64 buffer_idx = buffer_index;

			while (buffer_idx < size && buffer[match_idx] == buffer[buffer_idx]) {
				match_idx++;
				buffer_idx++;

				if (match_idx - window_idx > pow(2, length_bits) - 1) {
					match_idx--;
					buffer_idx--;
					break;
				}
			}

			u64 length = buffer_idx - buffer_index;
			if (length > match_length) {
				match_length = length;
				match_offset = buffer_index - window_idx;
			}
		}

		if (match_length > 3) {
			write_bit(stream, 1);
			write_bits(stream, match_offset, window_bits);
			write_bits(stream, match_length, length_bits);
			buffer_index += match_length;
		}
		else {
			write_bit(stream, 0);
			write_bits(stream, buffer[buffer_index], 8);
			buffer_index++;
		}
	}

	// Shrinking the buffer to the actual size
	u64 stream_size = (stream->bit_index / 8) + 1;
	stream->data 	= (u8*)realloc(stream->data, stream_size);

	return stream;
}

char* lz77_decompress(
		BitStream* compressed_stream,
		u64  size,
		u64* decompressed_size,
		u8 window_bits,
		u8 length_bits
		) {
	char* buffer = (char*)malloc(size);
	u64 buffer_index = 0;

	u64 total_compressed_bits = compressed_stream->bit_index;
	compressed_stream->bit_index = 0;

	while (buffer_index < size) {
		bool is_match = read_bit(compressed_stream);
		if (is_match) {
			u64 match_offset = read_bits(compressed_stream, window_bits);
			u64 match_length = read_bits(compressed_stream, length_bits);

			for (u64 idx = 0; idx < match_length; ++idx) {
				buffer[buffer_index + idx] = buffer[buffer_index - match_offset + idx];
			}
			buffer_index += match_length;
		}
		else {
			buffer[buffer_index] = read_bits(compressed_stream, 8);
			buffer_index++;
		}
	}

	*decompressed_size = buffer_index;
	return buffer;
}

bool check_buffer_equivalence(
		char* buffer1,
		char* buffer2,
		u64 size
		) {
	u64 num_diff = 0;
	for (u64 idx = 0; idx < size; ++idx) {
		if (buffer1[idx] != buffer2[idx]) {
			num_diff++;
		}
	}
	printf("Number of differences: %lu\n", num_diff);
	return (num_diff == 0);
}
