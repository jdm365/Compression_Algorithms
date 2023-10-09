#include <iostream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <string.h>

#include <stdint.h>
#include <stdlib.h>

#include "lz77.h"
#include "utils.h"

#define WINDOW_SIZE 8192 
#define LOOKAHEAD_BUFFER_SIZE 1024
// #define LOOKAHEAD_BUFFER_SIZE 65536 


/*
uint8_t* compress_lz77(const char* input, int& compressed_size) {
    int input_bytes = strlen(input);  // For simplicity, assume it's a C-string
    uint8_t* compressed_data = (uint8_t*) malloc(2 * input_bytes);
    int compressed_data_idx = 0;
    uint8_t flag_byte = 0;
    int flag_bit_pos = 0;

    std::unordered_map<std::string, int> prefix_map;

    const float one_tenthousandth = input_bytes / 10000.0f;
    float percent_boundary = 0.0f;
    std::cout << std::setprecision(2);

    int idx = 0;

    int iters_left = 10000;
    int time_remaining_seconds = 0;
    int time_remaining_minutes = 0;
    auto start = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::high_resolution_clock::now() - start;

    while (idx < input_bytes) {
        if (idx > percent_boundary) {
            time_elapsed = std::chrono::high_resolution_clock::now() - start;
            time_remaining_seconds = static_cast<int>(time_elapsed.count() * iters_left * 1e-9);
            time_remaining_minutes = time_remaining_seconds / 60;
            std::cout << "\rCompressing: " << (100.0f * idx / input_bytes) << "%       ETA: " << time_remaining_minutes << "m " << time_remaining_seconds % 60 << "s" << std::flush;
            percent_boundary += one_tenthousandth;
            iters_left--;
            start = std::chrono::high_resolution_clock::now();
        }

        int longest_match_length = 0;
        int longest_match_offset = 0;

        // Update prefix_map with new substrings
        int window_start = std::max(idx - WINDOW_SIZE, 0);
        int window_end = idx;
        int lookahead_start = idx;
        int lookahead_end = std::min(idx + LOOKAHEAD_BUFFER_SIZE, input_bytes);
        
        for (int len = 1; lookahead_start + len <= lookahead_end; ++len) {
            std::string prefix(&input[lookahead_start], len);
            prefix_map[prefix] = lookahead_start;
        }

        for (int len = 1; lookahead_start + len <= lookahead_end; ++len) {
            std::string prefix(&input[lookahead_start], len);
            auto it = prefix_map.find(prefix);
            if (it != prefix_map.end() && it->second < window_end && it->second >= window_start) {
                int match_length = len;
                longest_match_length = match_length;
                longest_match_offset = idx - it->second;
            }
        }

        if (flag_bit_pos == 0) {
            compressed_data[compressed_data_idx++] = flag_byte;
        }

        if (longest_match_length > 5) {
            flag_byte |= (1 << flag_bit_pos);
            compressed_data[compressed_data_idx++] = (uint8_t) longest_match_offset;
            compressed_data[compressed_data_idx++] = (uint8_t) longest_match_length;
            compressed_data[compressed_data_idx++] = input[idx + longest_match_length];
            idx += longest_match_length + 1;
        } else {
            flag_byte &= ~(1 << flag_bit_pos);
            compressed_data[compressed_data_idx++] = input[idx];
            idx += 1;
        }

        flag_bit_pos = (flag_bit_pos + 1) % 8;

        if (flag_bit_pos == 0) {
            compressed_data[compressed_data_idx - 1 - 8] = flag_byte;
            flag_byte = 0;
        }
    }

    if (flag_bit_pos != 0) {
        compressed_data[compressed_data_idx - 1 - flag_bit_pos] = flag_byte;
    }

    std::cout << "\rCompressing: " << "100.00%" << std::flush << std::endl;
    compressed_size = compressed_data_idx;
    return compressed_data;
}
*/

uint8_t* compress_lz77(const char* input, int& compressed_size) {
	int input_bytes = get_buffer_size_bytes(input);
    uint8_t* compressed_data = (uint8_t*) malloc(2 * input_bytes);
    int compressed_data_idx = 0;
    uint8_t flag_byte = 0;
    int flag_bit_pos = 0;

	const float one_tenthousandth = input_bytes / 10000;
	float percent_boundary = 0;

	std::cout << std::setprecision(2);

    int idx = 0;

	int iters_left = 10000;
	int time_remaining_seconds = 0;
	int time_remaining_minutes = 0;

	auto start = std::chrono::high_resolution_clock::now();
	auto time_elapsed = std::chrono::high_resolution_clock::now() - start;
    while (idx < input_bytes) {
		if (idx > percent_boundary) {
			time_elapsed = std::chrono::high_resolution_clock::now() - start;
			time_remaining_seconds = time_elapsed.count() * (iters_left / (10000 - iters_left + 1e-6)) * 1e-9;
			time_remaining_minutes = time_remaining_seconds / 60;
			std::cout << "\rCompressing: " << (100.0 * idx / input_bytes) << "%       ETA: " << time_remaining_minutes << "m " << time_remaining_seconds % 60 << "s" << std::flush;
			percent_boundary += one_tenthousandth;
			iters_left--;
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

	std::cout << "\rCompressing: " << "100.00%" << std::flush << std::endl;
    compressed_size = compressed_data_idx;

	time_elapsed = std::chrono::high_resolution_clock::now() - start;
	time_remaining_seconds = time_elapsed.count() * (iters_left / (10000 - iters_left + 1e-6)) * 1e-9;
	time_remaining_minutes = time_remaining_seconds / 60;
	std::cout << "Time taken: " << time_remaining_minutes << "m " << time_remaining_seconds % 60 << "s" << std::endl;
    return compressed_data;
}
