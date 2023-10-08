#include <iostream>

#include <stdint.h>

#include "lz77.h"
#include "utils.h"

#define WINDOW_SIZE 8192
// #define LOOKAHEAD_BUFFER_SIZE 18
#define LOOKAHEAD_BUFFER_SIZE 65536 

LZ77_t* _compress_lz77(const char* input, int& compressed_size) {
    int input_bytes = get_buffer_size_bytes(input);
    int compressed_data_entries = 0;
    LZ77_t* compressed_data = (LZ77_t*) malloc(input_bytes * sizeof(LZ77_t));

    for (int idx = 0; idx < input_bytes;) {
        int window_start = std::max(idx - WINDOW_SIZE, 0);
        int window_end = idx;

        int lookahead_start = idx;
        int lookahead_end = std::min(idx + LOOKAHEAD_BUFFER_SIZE, input_bytes);

        int longest_match_length = 0;
        int longest_match_offset = 0;

		if (idx == 100000) {
			break;
		}

        for (int window_idx = window_start; window_idx < window_end; ++window_idx) {
            int match_length = 0;

            for (int lookahead_idx = lookahead_start; lookahead_idx < lookahead_end; ++lookahead_idx) {
                if (window_idx + match_length >= idx) break;
                if (input[window_idx + match_length] == input[lookahead_idx]) {
                    ++match_length;
                } else {
                    break;
                }
            }

            if (match_length > longest_match_length) {
                longest_match_length = match_length;
                longest_match_offset = idx - window_idx;
            }
        }

        // Only make an entry if the match length is beneficial for compression
        if (longest_match_length > 6) {
            char next_char = input[idx + longest_match_length];

            compressed_data[compressed_data_entries++] = {
                (uint16_t)longest_match_offset,
                (uint16_t)longest_match_length,
                next_char
            };
            idx += longest_match_length + 1;  // Skip over the matched string
        } else {
			/*
            compressed_data[compressed_data_entries++] = {
                0,
                0,
                input[idx]
            };
			*/
            ++idx;
        }
    }

    compressed_size = compressed_data_entries;

    return compressed_data;
}


uint8_t* compress_lz77(const char* input, int& compressed_size) {
	int input_bytes = get_buffer_size_bytes(input);
    uint8_t* compressed_data = (uint8_t*) malloc(2 * input_bytes);
    int compressed_data_idx = 0;
    uint8_t flag_byte = 0;
    int flag_bit_pos = 0;

    int idx = 0;
    while (idx < input_bytes) {
		if (idx == 1000000) {
			break;
		}
        int longest_match_length = 0;
        int longest_match_offset = 0;

        int window_start = std::max(idx - WINDOW_SIZE, 0);
        int window_end = idx;
        int lookahead_start = idx;
        int lookahead_end = std::min(idx + LOOKAHEAD_BUFFER_SIZE, input_bytes);

        for (int window_idx = window_start; window_idx < window_end; ++window_idx) {
            int match_length = 0;

            for (int lookahead_idx = lookahead_start; lookahead_idx < lookahead_end; ++lookahead_idx) {
                if (input[window_idx + match_length] == input[lookahead_idx]) {
                    ++match_length;
                } else {
                    break;
                }
            }

            if (match_length > longest_match_length) {
                longest_match_length = match_length;
                longest_match_offset = idx - window_idx;
            }
        }

        // If this is the first bit in the flag byte, write the flag byte to the output.
        if (flag_bit_pos == 0) {
            compressed_data[compressed_data_idx++] = flag_byte;
        }

        // If the match length is more than 5, use compression
        if (longest_match_length > 5) {
            flag_byte |= (1 << flag_bit_pos); // Set the bit to 1 to indicate a tuple.

            compressed_data[compressed_data_idx++] = (uint8_t) longest_match_offset;
            compressed_data[compressed_data_idx++] = (uint8_t) longest_match_length;
            compressed_data[compressed_data_idx++] = input[idx + longest_match_length];

            idx += longest_match_length + 1;
        } else {
            flag_byte &= ~(1 << flag_bit_pos); // Set the bit to 0 to indicate a raw byte.

            compressed_data[compressed_data_idx++] = input[idx];

            idx += 1;
        }

        flag_bit_pos = (flag_bit_pos + 1) % 8;

        // If we have filled up a flag byte, write it back to its proper position.
        if (flag_bit_pos == 0) {
            compressed_data[compressed_data_idx - 1 - 8] = flag_byte;
            flag_byte = 0;
        }
    }

    // Update the last flag byte if it wasn't completely filled.
    if (flag_bit_pos != 0) {
        compressed_data[compressed_data_idx - 1 - flag_bit_pos] = flag_byte;
    }

    compressed_size = compressed_data_idx;
    return compressed_data;
}
