#include <iostream>
#include <fstream>

#include "utils.h"

char* read_file(const char* filename) {
	char* buffer = nullptr;

	std::ifstream fin;
	fin.open(filename, std::ios::binary);
	if (!fin.is_open()) {
		std::cout << "Error opening file" << std::endl;
	}

	fin.seekg(0, std::ios::end);
	int length = fin.tellg();
	fin.seekg(0, std::ios::beg);

	buffer = (char*)malloc(length * sizeof(char));

	fin.read(buffer, length);
	fin.close();

	return buffer;
}

int get_buffer_size_bytes(const char* buffer) {
	int size = 0;
	while (buffer[size] != '\0') {
		size++;
	}
	return size;
}
