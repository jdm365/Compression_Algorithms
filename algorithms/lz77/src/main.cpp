#include <iostream>
#include <fstream>
#include <string>

#include "lz77.h"
#include "utils.h"

int main() {
	// const char* FILENAME = "../../data/declaration_of_independence.txt";
	const char* FILENAME = "../../data/enwik9";

	const char* buffer = read_file(FILENAME);
	std::cout << "Size of buffer: " << get_buffer_size_bytes(buffer) << " bytes" << std::endl;

	int compressed_size = 0;
	// LZ77_t* compressed_buffer = _compress_lz77(buffer, compressed_size);
    // std::cout << "Size of compressed data: " << compressed_size * sizeof(LZ77_t) << " bytes" << std::endl;

	uint8_t* compressed_buffer = compress_lz77(buffer, compressed_size);
    std::cout << "Size of compressed data: " << compressed_size << " bytes" << std::endl;

	free(compressed_buffer);
	return 0;
}
