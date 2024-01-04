#pragma once

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
    u8* data;
    u64 bit_index;
} BitStream;

void init_bitstream(BitStream* stream, u8* buffer);

void write_bit_to_stream(BitStream* stream, bool bit);
bool read_bit_to_stream(BitStream* stream);
void write_bits_to_stream(BitStream* stream, u64 value, u64 num_bits);
u64  read_bits_to_stream(BitStream* stream, u64 num_bits);

bool check_buffer_equivalence(
		char* buffer1,
		char* buffer2,
		u64 size
		);
BitStream* lz77_compress(
		char* buffer,
		u64 size,
		u8 window_bits,
		u8 length_bits
		);
char* lz77_decompress(
		BitStream* compressed_stream,
		u64  size,
		u64* decompressed_size,
		u8 window_bits,
		u8 length_bits
		);
