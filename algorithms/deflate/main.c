#include <stdio.h>
#include <time.h>

#include "deflate.h"

int main(int argc, char* argv[]) {
	const char* FILENAME = "../../data/enwik8";

	clock_t start = clock();
	StateData state_data = compress(FILENAME);
	clock_t end = clock();

	printf("Compression took %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    return 0;
}
