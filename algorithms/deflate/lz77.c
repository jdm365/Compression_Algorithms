#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "lz77.h"

uint64_t min(uint64_t a, uint64_t b) { return a < b ? a : b; }
uint64_t max(uint64_t a, uint64_t b) { return a > b ? a : b; }


inline uint32_t hash(uint32_t pattern) {
	uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;

    uint32_t hash = 0;
    uint32_t k   = pattern;

    // Prepare the key
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;

    // Mix into hash
    hash ^= k;
    hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;

    // Finalize hash
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

	return hash % TABLE_SIZE;
}

void init_hash_table(HashTableArray* table) {
	table->buckets = (ArrayNode*)malloc(sizeof(ArrayNode) * TABLE_SIZE);
	if (table->buckets == NULL) {
		fprintf(stderr, "Error: could not allocate memory for hash table\n");
		exit(1);
	}

	for (uint64_t idx = 0; idx < TABLE_SIZE; ++idx) {
		table->buckets[idx].pattern = 0;
		table->buckets[idx].index   = 0;
		table->buckets[idx].is_set  = false;
	}
	memset(table->bucket_indices, 0, sizeof(uint32_t) * (1 << MAX_WINDOW_BITS));
	table->current_idx = 0;
	table->is_full = false;
}

static void print_node(ArrayNode* node) {
	printf(
			"Pattern: %u Index: %lu Is Set: %d\n", 
			node->pattern, 
			node->index, 
			node->is_set
			);
}

void insert_hash_table(HashTableArray* table, uint32_t pattern, uint64_t index) {

	uint32_t bucket_idx = hash(pattern);

	/*
	uint32_t scan_direction = 1;
	if (pattern % 2 == 0) {
		scan_direction = -1;
	}

	// Search hash table index and linear probe.
	while (table->buckets[bucket_idx].is_set) {
		bucket_idx += scan_direction;
		bucket_idx %= TABLE_SIZE;
	}
	*/
	while (table->buckets[bucket_idx].is_set) {
		bucket_idx = (bucket_idx + 1) % TABLE_SIZE;
	}

	/*
	if (bucket_idx - hash(pattern) > 0) {
		printf("Collisions: %u\n", bucket_idx - hash(pattern));
	}
	else {
		printf("No collisions\n");
	}
	*/

	ArrayNode new_node = {
		pattern,
		index,
		true
	};
	table->buckets[bucket_idx] = new_node;

	if (table->is_full) {
		// Remove lra node.
		uint32_t lra_idx = table->bucket_indices[table->current_idx];
		table->buckets[lra_idx].pattern = 0;
		table->buckets[lra_idx].index   = 0;
		table->buckets[lra_idx].is_set  = false;
	}

	table->bucket_indices[table->current_idx++] = bucket_idx;

	if (table->current_idx >= (WINDOW_SIZE - 1) && !table->is_full) {
		table->is_full = true;
	}

	table->current_idx %= WINDOW_SIZE;
}

uint64_t find(HashTableArray* table, uint32_t pattern) {
	uint32_t bucket_idx = hash(pattern);

	while (
			table->buckets[bucket_idx].pattern != pattern
				&&
			table->buckets[bucket_idx].is_set
			) {
		++bucket_idx;
	}

	if (!table->buckets[bucket_idx].is_set) return UINT64_MAX;

	return table->buckets[bucket_idx].index;
}

inline void write_literal(
		char* buffer,
		char c,
		uint64_t* buffer_index
		) {
	// For now write whole byte as literal flag.
	buffer[(*buffer_index)++] = 0;
	buffer[(*buffer_index)++] = c;
}

inline void write_length_distance(
		char* buffer,
		uint8_t length,
		uint16_t distance,
		uint64_t* buffer_index
		) {
	// For now write whole byte as match flag.
	buffer[(*buffer_index)++] = 1;
	buffer[(*buffer_index)++] = distance & 0xFF;
	buffer[(*buffer_index)++] = (distance >> 8) & 0xFF;
	buffer[(*buffer_index)++] = length;
}

void lz77_compress(
		const char* input_buffer,
		uint64_t input_buffer_size,
		char* compressed_buffer,
		uint64_t* compressed_buffer_size,
		HashTableArray* table
		) {
	uint64_t buffer_index = 0;
	uint64_t compressed_buffer_index = 0;

	uint64_t window_size = (1 << MAX_WINDOW_BITS) - 1;

	const uint32_t MAX_LEN = (1 << MAX_LENGTH_BITS) - 1;

	while (buffer_index < input_buffer_size) {
		uint64_t match_length = 0;
		uint64_t match_offset = 0;

		uint32_t current_word = *(uint32_t*)(&input_buffer[buffer_index]);
		uint64_t orig_match_idx = find(table, current_word);
		uint64_t match_idx = orig_match_idx;

		if (match_idx == UINT64_MAX || buffer_index - match_idx >= window_size) {
			// No match found.
			// Add char literal to the buffer and add word pattern to the hash table.

			write_literal(compressed_buffer, input_buffer[buffer_index], &compressed_buffer_index);
			insert_hash_table(table, current_word, buffer_index);

			++buffer_index;
		}
		else {
			// Found match at match_idx of buffer.
			// Starting four chars later keep comparing until a mismatch is found.
			match_idx    += 4;
			buffer_index += 4;
			while (
					input_buffer[match_idx] == input_buffer[buffer_index]
						&&
					match_idx - orig_match_idx < MAX_LEN
					) {
				++match_idx;
				++buffer_index;
			}

			uint32_t length = match_idx - orig_match_idx;
			uint32_t offset = buffer_index - match_idx;
			if (offset >= window_size) {
				printf("Offset `%u` is too large. "
					   "Offset cannot exceed window size of `%lu`\n",
					   offset,
					   window_size
					   );
				printf("Current buffer_index: %lu Current match_idx: %lu"
						" DIFF: %lu\n"
						, buffer_index, match_idx, buffer_index - match_idx
						);
				exit(1);
			}

			write_length_distance(compressed_buffer, length, offset, &compressed_buffer_index);

			// Insert last length words into hash table.
			for (uint32_t idx = 0; idx < length; ++idx) {
				uint32_t word = *(uint32_t*)(&input_buffer[buffer_index - length + idx]);
				insert_hash_table(table, word, buffer_index - length + idx);
			}
		}
	}

	// Shrinking the buffer to the actual size. **Might not be a good idea here.
	*compressed_buffer_size = compressed_buffer_index;
}

void lz77_decompress(
		const char* compressed_buffer,
		uint64_t compressed_buffer_size,
		char* decompressed_buffer,
		uint64_t* decompressed_buffer_size
		) {
	*decompressed_buffer_size = compressed_buffer_size * 2;
	decompressed_buffer = (char*)malloc(*decompressed_buffer_size);

	uint64_t buffer_index = 0;

	while (buffer_index < compressed_buffer_size) {
		bool is_match = compressed_buffer[buffer_index++];
		if (is_match) {
			uint16_t distance    = compressed_buffer[buffer_index] | (compressed_buffer[buffer_index + 1] << 8);
			uint8_t match_length = compressed_buffer[buffer_index + 2];

			uint64_t match_start = buffer_index - distance;
			for (uint32_t idx = 0; idx < match_length; ++idx) {
				decompressed_buffer[(*decompressed_buffer_size)++] = decompressed_buffer[match_start + idx];
			}
			buffer_index += 3;
		}
		else {
			decompressed_buffer[(*decompressed_buffer_size)++] = compressed_buffer[buffer_index++];
		}
	}

	decompressed_buffer = (char*)realloc(decompressed_buffer, *decompressed_buffer_size);
}
