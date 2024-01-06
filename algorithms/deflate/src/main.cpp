#include <vector>
#include <fstream>

#include <stdio.h>
#include <zlib.h>
#include <time.h>

#include "deflate.h"

// Function to compress data using zlib and write in gzip format
bool gzipCompress(
		const std::vector<unsigned char>& inputData, 
		std::vector<unsigned char>& compressedData
		) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit2(
				&stream, 
				Z_BEST_COMPRESSION,
				Z_DEFLATED, 
				MAX_WBITS + 16,
				8, 
				Z_DEFAULT_STRATEGY
				) != Z_OK
			) {
        return false;
    }
	/*
	// Fastest compression
	if (deflateInit2(
				&stream,
				Z_BEST_SPEED,
				Z_DEFLATED,
				MAX_WBITS + 16,
				8,
				Z_DEFAULT_STRATEGY
				) != Z_OK
			) {
		return false;
	}
	*/

    stream.avail_in = inputData.size();
    stream.next_in = const_cast<Bytef*>(inputData.data());

    // Perform compression
    do {
        compressedData.resize(compressedData.size() + 1024);
        stream.avail_out = 1024;
        stream.next_out = &compressedData[compressedData.size() - 1024];
        deflate(&stream, Z_FINISH);
    } while (stream.avail_out == 0);

    // Resize to actual size
    compressedData.resize(compressedData.size() - stream.avail_out);

    deflateEnd(&stream);

	printf("Original size:   %d\n", (int)inputData.size());
	printf("Compressed size: %d\n", (int)compressedData.size());
    return true;
}

void gzip_default_test(const char* filename) {
	std::ifstream inputFile(filename, std::ios::binary);
	std::ofstream outputFile("output.gz", std::ios::binary);

	if (!inputFile.is_open() || !outputFile.is_open()) {
		perror("Error opening file");
		return;
	}

	std::vector<unsigned char> inputData(
			(std::istreambuf_iterator<char>(inputFile)), 
			std::istreambuf_iterator<char>()
			);
	std::vector<unsigned char> compressedData;

	if (gzipCompress(inputData, compressedData)) {
		outputFile.write(
				reinterpret_cast<const char*>(
					compressedData.data()
					), compressedData.size()
				);
	} 
	else {
		perror("Error");
		return;
	}

	remove("output.gz");
}

void custom_test(const char* filename) {
	u64 filesize;
	char* buffer = read_input_buffer(filename, &filesize);

	printf("File size uncompressed: %d\n", (int)filesize);

	const int LENGTH_BITS = 6; 
	const int WINDOW_BITS = 12;

	clock_t start = clock();

	StateData state_data = deflate_compress(
			buffer, 
			filesize, 
			WINDOW_BITS,
			LENGTH_BITS
			);
	u64   compressed_bytes = state_data.huffman_compressed_size;
	printf("File size compressed:   %d\n", (int)compressed_bytes);

	exit(0);
	u64   decompressed_bytes = state_data.lz77_compressed_size;
	char* decompressed_buffer = deflate_decompress(
			state_data,
			&decompressed_bytes,
			WINDOW_BITS,
			LENGTH_BITS
			);

	if (compare_buffers(buffer, decompressed_buffer, filesize)) {
		printf("SUCCESS\n");
	}
	else {
		printf("FAILURE\n");
	}

	printf("========================================================================\n");
	printf("============================ DEFLATE  ==================================\n");
	printf("========================================================================\n");
	printf("Uncompressed size:  %lu\n", filesize);
	printf("Compressed size:    %lu\n", compressed_bytes);
	printf("Reconstructed size: %lu\n", decompressed_bytes);
	printf("Compression ratio:  %f\n",  (double)filesize / compressed_bytes);
	printf("Total time:         %fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	printf("========================================================================\n");

	free(buffer);
	free(decompressed_buffer);
}

int main(int argc, char* argv[]) {
	// const char* FILENAME = "../../data/enwik6";
	const char* FILENAME = "../../data/enwik7";
	// const char* FILENAME = "../../data/enwik8";
	// const char* FILENAME = "../../data/enwik9";
	// const char* FILENAME = "../../data/declaration_of_independence.txt";

	gzip_default_test(FILENAME);
	custom_test(FILENAME);
    return 0;
}
