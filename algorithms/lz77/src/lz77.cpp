#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <string.h>

#include <stdint.h>
#include <stdlib.h>

#include "lz77.h"
#include "utils.h"

#define WINDOW_SIZE 4096
#define LOOKAHEAD_BUFFER_SIZE 256
#define BASE 257
#define MOD 1000000007


uint8_t* compress_lz77(const char* input, int& compressed_size) {
    int input_bytes = strlen(input);  // Replace with your get_buffer_size_bytes function if needed
    std::vector<uint8_t> compressed_data(2 * input_bytes);
    int compressed_data_idx = 0;
    uint8_t flag_byte = 0;
    int flag_bit_pos = 0;

    std::unordered_map<long long, int> hashMap;
    long long rollingHash = 0, power = 1;

    // Initialize rolling hash and power for the first window
    for (int i = 0; i < WINDOW_SIZE && i < input_bytes; ++i) {
        rollingHash = (rollingHash * BASE + input[i]) % MOD;
        if (i != 0) power = (power * BASE) % MOD;
    }
    
    if (WINDOW_SIZE < input_bytes) {
        hashMap[rollingHash] = 0;
    }

    for (int idx = 0; idx < input_bytes; ++idx) {
        // Remove the old character from the rolling hash and update the hash map
        if (idx >= WINDOW_SIZE) {
            int old_start = idx - WINDOW_SIZE;
            rollingHash = ((rollingHash - input[old_start] * power) % MOD + MOD) % MOD;
            hashMap[rollingHash] = old_start + 1;
        }

		if (idx % 1000000 == 0) {
			std::cout << idx / 1000000 << " MB processed" << std::endl;
		}

        // Add the new character to the rolling hash
        rollingHash = (rollingHash * BASE + input[idx]) % MOD;

        // Try to find a match in the hash map
        int longest_match_length = 0, longest_match_offset = 0;
        if (hashMap.find(rollingHash) != hashMap.end()) {
            int offset = idx - hashMap[rollingHash];
            int len = 0;
            while (idx + len < input_bytes && input[hashMap[rollingHash] + len] == input[idx + len]) {
                ++len;
            }
            if (len > longest_match_length) {
                longest_match_length = len;
                longest_match_offset = offset;
            }
        }

        // Write the output
        if (flag_bit_pos == 0) {
            compressed_data[compressed_data_idx++] = flag_byte;
        }

        if (longest_match_length > 2) {
            flag_byte |= (1 << flag_bit_pos);
            compressed_data[compressed_data_idx++] = (longest_match_offset & 0xFF00) >> 8;
            compressed_data[compressed_data_idx++] = longest_match_offset & 0x00FF;
            compressed_data[compressed_data_idx++] = longest_match_length;
            idx += longest_match_length - 1;
        } else {
            flag_byte &= ~(1 << flag_bit_pos);
            compressed_data[compressed_data_idx++] = input[idx];
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

    compressed_size = compressed_data_idx;
    uint8_t* result = new uint8_t[compressed_size];
    memcpy(result, compressed_data.data(), compressed_size);
    return result;
}


/*
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

			uint16_t longest_match_offset_16 = (uint16_t) longest_match_offset;
			uint16_t longest_match_length_16 = (uint16_t) longest_match_length;

			memcpy(compressed_data + compressed_data_idx, &longest_match_offset_16, sizeof(uint16_t));
			compressed_data_idx += sizeof(uint16_t);
			memcpy(compressed_data + compressed_data_idx, &longest_match_length_16, sizeof(uint16_t));
			compressed_data_idx += sizeof(uint16_t);

			compressed_data[compressed_data_idx++] = input[idx + longest_match_length];

			idx += longest_match_length + 1;
            // flag_byte |= (1 << flag_bit_pos); // Set the bit to 1 to indicate a tuple.

            // compressed_data[compressed_data_idx++] = (uint8_t) longest_match_offset;
            // compressed_data[compressed_data_idx++] = (uint8_t) longest_match_length;
            // compressed_data[compressed_data_idx++] = input[idx + longest_match_length];

            // idx += longest_match_length + 1;
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
*/
