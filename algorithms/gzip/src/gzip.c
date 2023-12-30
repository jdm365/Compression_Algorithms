#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gzip.h"

u32 u32_hash(const u8* data, int size) {
	u32 hash = 0;

	for (int idx = 0; idx < size; ++idx) {
        hash += data[idx];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

void u32_add_item(
		u32_HashMap* map, 
		u8* key, 
		u32 value, 
		int key_size
		) {
	u32 hash = u32_hash(key, key_size);
	
	map->keys[hash]   = key;
	map->values[hash] = value;

	map->key_sizes[hash] = key_size;
}

u32 u32_get_item(
		u32_HashMap* map, 
		u8* key, 
		int key_size
		) {
	u32 hash = u32_hash(key, key_size);

	// Check if item exists in hash table
	if (map->keys[hash] != NULL) {
		return map->values[hash];
	}

	return UINT32_MAX;
}

void build_huffman_tree(
		const u32* 	  weights,
		u32        	  num_weights,
		MinHeapNode** root
		) {
}
