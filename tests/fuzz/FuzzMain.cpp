/**
	Runs the libFuzzer target of PikaCmd.cpp (built with `LIBFUZZ`) over files instead of under libFuzzer, so that
	normal builds can replay the fuzz corpus. Reads one file path per line from stdin.
**/

#include <stdint.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

int main(int argc, char** argv) {
	LLVMFuzzerInitialize(&argc, &argv);
	int count = 0;
	std::string path;
	while (std::getline(std::cin, path)) {
		if (!path.empty() && path[path.size() - 1] == '\r') path.erase(path.size() - 1);
		if (path.empty()) continue;
		std::ifstream file(path.c_str(), std::ios::binary);
		if (!file) {
			fprintf(stderr, "Could not open %s\n", path.c_str());
			return 1;
		}
		std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		const size_t size = bytes.size();
		bytes.push_back(0);	// So that `&bytes[0]` is valid for an empty file.
		LLVMFuzzerTestOneInput(&bytes[0], size);
		++count;
	}
	if (count == 0) {
		fprintf(stderr, "No files to run\n");
		return 1;
	}
	printf("Fuzz target ran on %d files\n", count);
	return 0;
}
