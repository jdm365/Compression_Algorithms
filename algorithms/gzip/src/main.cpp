#include <iostream>
#include <fstream>
#include <vector>
#include <zlib.h>

// Function to compress data using zlib and write in gzip format
bool gzipCompress(
		const std::vector<unsigned char>& inputData, 
		std::vector<unsigned char>& compressedData
		) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    // Initialize for gzip compression
    if (deflateInit2(
				&stream, 
				Z_DEFAULT_COMPRESSION, 
				Z_DEFLATED, 
				MAX_WBITS + 16, 
				8, 
				Z_DEFAULT_STRATEGY
				) != Z_OK
			) {
        return false;
    }

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

	std::cout << "Original size:   " << inputData.size() << std::endl;
	std::cout << "Compressed size: " << compressedData.size() << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
	const char* FILENAME = "../../data/enwik9";
	// const char* FILENAME = "../../data/declaration_of_independence.txt";

    std::ifstream inputFile(FILENAME, std::ios::binary);
    std::ofstream outputFile("output.gz", std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
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
        std::cerr << "Compression failed" << std::endl;
        return 1;
    }

    return 0;
}
