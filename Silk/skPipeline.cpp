#include "skPipeline.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace sk
{
	skPipeline::skPipeline(const std::string& vertFilePath, const std::string& fragFilePath)
	{
		createGraphicsPipeline(vertFilePath, fragFilePath);
	}

	std::vector<char> skPipeline::readFile(const std::string& filepath)
	{
		// std::ios::ate bit flag seeks the end of the file immediately (helps with determining file size), and std::ios::binary specifies we're reading binary data
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file: " + filepath + '\n');

		size_t fileSize = static_cast<size_t>(file.tellg()); // tellg() gets the size at where "cursor" is at, in this case since we used ate bit flag, it's at the end which then will return size
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	void skPipeline::createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilePath)
	{
		auto vertCode = readFile(vertFilepath);
		auto fragCode = readFile(fragFilePath);

		std::cout << "Vertex Shader code size: " << vertCode.size() << '\n';
		std::cout << "Fragment Shader code size: " << fragCode.size() << '\n';
	}
} // namespace sk