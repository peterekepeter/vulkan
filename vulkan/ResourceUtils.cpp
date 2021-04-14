#include "pch.h"
#include "ResourceUtils.h"

std::vector<char> read_file(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("failed to open file! ") + filename);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

std::vector<char> read_shader(const std::string& filename) {
	std::string newname = filename; // copy;
	newname = "build\\" + filename + ".opt.spv";
	return read_file(newname);
}

