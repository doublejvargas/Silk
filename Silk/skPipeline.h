#pragma once
#include "skDevice.h"

#include <string>
#include <vector>

namespace sk
{
	struct PipelineConfigInfo {};

	class skPipeline
	{
	public:
		skPipeline(
			skDevice &device, 
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const PipelineConfigInfo &configInfo);
		
		~skPipeline() {}
		skPipeline(const skPipeline&) = delete;
		void operator=(const skPipeline&) = delete;

		static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

	private:
		static std::vector<char> readFile(const std::string& filepath);
		
		void createGraphicsPipeline(
			const std::string& vertFilepath, 
			const std::string& fragFilePath, 
			const PipelineConfigInfo& configInfo);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		// Potentially dangerous as could lead to a dangling pointer if device is destroyed before pipeline,
		// however, relationship between Pipeline and device is aggregation, meaning that device is guaranteed to exist during lifetime of Pipeline
		// i.e., m_skDevice will always outlive skPipeline object.
		skDevice& m_skDevice;
		VkPipeline m_GraphicsPipeline;
		VkShaderModule m_VertShaderModule;
		VkShaderModule m_FragShaderModule;
	};
} // namespace sk
