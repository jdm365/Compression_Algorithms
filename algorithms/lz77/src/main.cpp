#include <iostream>
#include <fstream>
#include <string>
// Read data into char array buffer form ../data/declaration_of_independence.txt




char* read_file(const char* filename) {
	// Creater buffer of indeterminate size
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


int main() {
	const char* FILENAME = "../../data/declaration_of_independence.txt";
	// const char* FILENAME = "../../data/enwik9";

	char* buffer = read_file(FILENAME);
	std::cout << "Size of buffer: " << get_buffer_size_bytes(buffer) << " bytes" << std::endl;
	return 0;
}
