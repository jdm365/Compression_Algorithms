#pragma once

#include <stdint.h>

typedef struct __attribute__((__packed__)) {
	uint16_t offset;
	uint16_t length;
	char next;
} LZ77_t;

uint8_t* compress_lz77(const char* input, int& compressed_size);
char* decompress_lz77(LZ77_t* input);
