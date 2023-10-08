#include <iostream>

#include <stdint.h>

#include "lz77.h"
#include "utils.h"

#define WINDOW_SIZE 8192
// #define LOOKAHEAD_BUFFER_SIZE 18
#define LOOKAHEAD_BUFFER_SIZE 65536 


/*
LZ77_t* compress_lz77(const char* input) {
	int input_bytes = get_buffer_size_bytes(input);
	int compressed_data_entries = 0;

	int window_start = 0;
	int window_end   = 0;

	int lookahead_start = 0;
	int lookahead_end   = 0;

	LZ77_t* compressed_data = (LZ77_t*)malloc(input_bytes * sizeof(LZ77_t));

	for (int idx = 0; idx < input_bytes; ++idx) {
		// Get the window and lookahead buffer
		window_start = std::max(idx - WINDOW_SIZE, 0);
		window_end   = idx;

		lookahead_start = idx;
		lookahead_end   = std::min(idx + LOOKAHEAD_BUFFER_SIZE, input_bytes);

		// Find the longest match
		int longest_match_length = 0;
		int longest_match_offset = 0;

		for (int window_idx = window_start; window_idx < window_end; ++window_idx) {
			int match_length = 0;

			// Search in the lookahead buffer for a match
			for (int lookahead_idx = lookahead_start; lookahead_idx < lookahead_end; ++lookahead_idx) {
				if (input[window_idx + match_length] == input[lookahead_idx]) {
					match_length++;
				}
				else {
					break;
				}
			}

			if (match_length > longest_match_length) {
				longest_match_length = match_length;
				longest_match_offset = idx - window_idx;
			}
		}

		// Write the longest match to the compressed data
		compressed_data[compressed_data_entries] = {
			(uint16_t)longest_match_offset,
			(uint16_t)longest_match_length,
			input[idx + longest_match_length]
		};
		compressed_data_entries++;
		idx += longest_match_length;
	}

	// Get first compressed_data_entries from compressed_data
	LZ77_t* compressed_data_final = (LZ77_t*)malloc(compressed_data_entries * sizeof(LZ77_t));
	memcpy(compressed_data_final, compressed_data, compressed_data_entries * sizeof(LZ77_t));

	free(compressed_data);

	std::cout << "Size of compressed data: " << compressed_data_entries * sizeof(LZ77_t) << " bytes" << std::endl;
	return compressed_data_final;
}
*/
LZ77_t* compress_lz77(const char* input, int& compressed_size) {
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
