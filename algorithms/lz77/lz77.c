#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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
	for (uint32_t idx = 0; idx < TABLE_SIZE; ++idx) {
		table->buckets[idx].pattern = 0;
		table->buckets[idx].index = 0;
		table->buckets[idx].is_set = false;
	}
	memset(table->bucket_indices, 0, sizeof(uint32_t) * (1 << WINDOW_BITS));
	table->current_idx = 0;
	table->is_full = false;
}

void insert_hash_table(HashTableArray* table, uint32_t pattern, uint64_t index) {
	// TODO: Get differential table->current_idx since last call, delete 
	//       differential previous entries from the table.
	uint32_t bucket_idx = hash(pattern);

	// Search hash table index and linear probe.
	while (table->buckets[bucket_idx].is_set) ++bucket_idx;

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

	// printf("Full: %d Index: %lu Current index: %u\n", table->is_full, index, table->current_idx);
	if (table->current_idx >= ((1 << WINDOW_BITS) - 1) && !table->is_full) {
		table->is_full = true;
	}

	table->current_idx %= (1 << WINDOW_BITS);
}

void print_array_node(const ArrayNode* node) {
	printf("Pattern: %u\n", node->pattern);
	printf("Index: %lu\n", node->index);
	printf("Is set: %d\n", node->is_set);
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


void print_bit_string(const char* buffer, uint64_t size) {
	for (uint64_t idx = 0; idx < size; ++idx) {
		for (int bit = 7; bit >= 0; --bit) {
			bool is_set = buffer[idx] & (1 << bit);
			printf("%d", is_set ? 1 : 0);
		}
	}
	printf("\n");
}

char* read_input_buffer(
		const char* filename,
		uint64_t* size
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

void init_bitstream(BitStream* stream, uint8_t* buffer) {
    stream->data 	  = buffer;
    stream->bit_index = 0;
}

inline void write_bit(BitStream* stream, bool bit) {
	uint64_t byte_index = stream->bit_index / 8;
	uint64_t bit_offset = stream->bit_index % 8;

	if (bit) {
		stream->data[byte_index] |= (1 << bit_offset);
	}
	else {
		stream->data[byte_index] &= ~(1 << bit_offset);
	}

	++(stream->bit_index);
}


inline bool read_bit(BitStream* stream) {
	uint64_t byte_index = stream->bit_index / 8;
	uint64_t bit_offset = stream->bit_index % 8;

	bool is_set = (stream->data[byte_index] >> bit_offset) & 1;
	++(stream->bit_index);

	return is_set;
}

void write_bits(BitStream* stream, uint64_t value, uint64_t num_bits) {
	for (uint64_t bit = 0; bit < num_bits; ++bit) {
		bool is_set = value & (1 << bit);
		write_bit(stream, is_set);
	}
}

uint64_t read_bits(BitStream* stream, uint64_t num_bits) {
	uint64_t value = 0;
	for (uint64_t bit = 0; bit < num_bits; ++bit) {
		if (read_bit(stream)) {
			value |= (1 << bit);
		}
	}
	return value;
}


BitStream* lz77_compress_old(
		const char* buffer,
		uint64_t size
		) {
	BitStream* stream = (BitStream*)malloc(sizeof(BitStream));
	uint64_t buffer_size = size * 2;
	uint64_t buffer_index = 0;

	uint64_t window_size = (1 << WINDOW_BITS) - 1;

	uint64_t window_start = 0;
	uint64_t window_end   = 0;

	const uint32_t MAX_LEN = (1 << LENGTH_BITS) - 1;

	init_bitstream(stream, (uint8_t*)malloc(buffer_size));

	while (buffer_index < size) {
		uint64_t match_length = 0;
		uint64_t match_offset = 0;

		window_start = max((uint64_t)0, buffer_index - window_size);
		window_end   = buffer_index;

		for (uint64_t window_idx = window_start; window_idx < window_end; ++window_idx) {
			uint64_t match_idx  = window_idx;
			uint64_t buffer_idx = buffer_index;

			if (
					*(uint32_t*)(&buffer[match_idx])
						!=
					*(uint32_t*)(&buffer[buffer_idx])
					) {
				continue;
			}

			while (
					buffer_idx < size 
						&& 
					buffer[match_idx] == buffer[buffer_idx]
					) {
				++match_idx;
				++buffer_idx;

				if (match_idx - window_idx >= MAX_LEN) {
					break;
				}
			}

			uint64_t length = buffer_idx - buffer_index;
			if (length > match_length) {
				match_length = length;
				match_offset = buffer_index - window_idx;
			}
		}

		// Match length at least 4 chars.
		if (match_length != 0) {
			write_bit(stream, 1);
			write_bits(stream, match_offset, WINDOW_BITS);
			write_bits(stream, match_length, LENGTH_BITS);
			buffer_index += match_length;
		}
		else {
			write_bit(stream, 0);
			write_bits(stream, buffer[buffer_index], 8);
			++buffer_index;
		}
	}

	// Shrinking the buffer to the actual size
	uint64_t stream_size = (stream->bit_index / 8) + 1;
	stream->data = (uint8_t*)realloc(stream->data, stream_size);

	return stream;
}

BitStream* lz77_compress(
		const char* buffer,
		uint64_t size
		) {
	BitStream* stream = (BitStream*)malloc(sizeof(BitStream));
	uint64_t buffer_size = size * 2;
	uint64_t buffer_index = 0;

	uint64_t window_size = (1 << WINDOW_BITS) - 1;

	const uint32_t MAX_LEN = (1 << LENGTH_BITS) - 1;

	init_bitstream(stream, (uint8_t*)malloc(buffer_size));

	HashTableArray table;
	init_hash_table(&table);

	while (buffer_index < size) {
		uint64_t match_length = 0;
		uint64_t match_offset = 0;

		uint32_t current_word = *(uint32_t*)(&buffer[buffer_index]);
		uint64_t orig_match_idx = find(&table, current_word);
		uint64_t match_idx = orig_match_idx;

		// if (match_idx == UINT64_MAX) {
		if (match_idx == UINT64_MAX || buffer_index - match_idx == window_size + 1) {
			// No match found.
			// Add char literal to the buffer and add word pattern to the hash table.
			write_bit(stream, 0);
			write_bits(stream, buffer[buffer_index], 8);
			insert_hash_table(&table, current_word, buffer_index);

			++buffer_index;
		}
		else {
			// Found match at match_idx of buffer.
			// Starting four chars later keep comparing until a mismatch is found.
			match_idx    += 4;
			buffer_index += 4;
			while (
					buffer[match_idx] == buffer[buffer_index]
						&&
					match_idx - orig_match_idx < MAX_LEN
					) {
				++match_idx;
				++buffer_index;
			}

			uint32_t length = match_idx - orig_match_idx;
			uint32_t offset = buffer_index - match_idx;
			if (offset >= window_size + 1) {
				printf("Offset `%u` is too large."
					   "Offset cannot exceed window size of `%lu`\n",
					   offset,
					   window_size + 1
					   );
				printf("Current buffer_index: %lu Current match_idx: %lu"
						" DIFF: %lu\n"
						, buffer_index, match_idx, buffer_index - match_idx
						);
				exit(1);
			}

			write_bit(stream, 1);
			write_bits(stream, offset, WINDOW_BITS);
			write_bits(stream, length, LENGTH_BITS);

			// Insert last length words into hash table.
			for (uint32_t idx = 0; idx < length; ++idx) {
				uint32_t word = *(uint32_t*)(&buffer[buffer_index - length + idx]);
				insert_hash_table(&table, word, buffer_index - length + idx);
			}
		}
	}

	// Shrinking the buffer to the actual size
	uint64_t stream_size = (stream->bit_index / 8) + 1;
	stream->data = (uint8_t*)realloc(stream->data, stream_size);

	return stream;
}

char* lz77_decompress(
		BitStream* compressed_stream,
		uint64_t  size,
		uint64_t* decompressed_size
		) {
	char* buffer = (char*)malloc(size);
	uint64_t buffer_index = 0;

	uint64_t total_compressed_bits = compressed_stream->bit_index;
	compressed_stream->bit_index = 0;

	while (buffer_index < size) {
		bool is_match = read_bit(compressed_stream);
		if (is_match) {
			uint64_t match_offset = read_bits(compressed_stream, WINDOW_BITS);
			uint64_t match_length = read_bits(compressed_stream, LENGTH_BITS);

			for (uint64_t idx = 0; idx < match_length; ++idx) {
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
		const char* buffer1,
		const char* buffer2,
		uint64_t   size
		) {
	uint64_t num_diff = 0;
	for (uint64_t idx = 0; idx < size; ++idx) {
		if (buffer1[idx] != buffer2[idx]) {
			num_diff++;
		}
	}
	printf("Number of differences: %lu\n", num_diff);
	return (num_diff == 0);
}
