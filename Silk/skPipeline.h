#pragma once

#include <string>
#include <vector>

namespace sk
{
	class skPipeline
	{
	public:
		skPipeline(const std::string& vertFilePath, const std::string& fragFilePath);

	private:
		static std::vector<char> readFile(const std::string& filepath);
		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilePath);
	};
} // namespace sk