#pragma once
#include "skDevice.h"

#include <string>
#include <vector>

namespace sk
{
	// struct used to modify the fixed function pipeline stages in vulkan, i.e., input assembler, rasterization, etc..
	struct PipelineConfigInfo 
	{
		PipelineConfigInfo() = default;

		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo &operator = (const PipelineConfigInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class skPipeline
	{
	public:
		skPipeline() = default;

		skPipeline(
			skDevice &device, 
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const PipelineConfigInfo &configInfo);
		
		~skPipeline();
		skPipeline(const skPipeline&) = delete;
		skPipeline &operator=(const skPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

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
