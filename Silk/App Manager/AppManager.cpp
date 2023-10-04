#include "AppManager.h"

#include <stdexcept>

namespace sk
{

	AppManager::AppManager()
	{
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}

	AppManager::~AppManager()
	{
		vkDestroyPipelineLayout(m_skDevice.device(), m_PipelineLayout, nullptr);
	}

	void AppManager::run()
	{
		while (!m_skWindow.shouldClose())
		{
			glfwPollEvents();
		}
	}

	void AppManager::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(m_skDevice.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline object. \n");
		}
	}

	void AppManager::createPipeline()
	{
		auto pipelineConfig = skPipeline::defaultPipelineConfigInfo(m_skSwapChain.width(), m_skSwapChain.height());
		//a render pass is basically an outline for the structure/format of the framebuffer.
		pipelineConfig.renderPass = m_skSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = m_PipelineLayout;
		m_skPipeline = std::make_unique<skPipeline>(
			m_skDevice,
			"res\\shaders\\bin\\simple_shader.vert.spv",
			"res\\shaders\\bin\\simple_shader.frag.spv",
			pipelineConfig);
	}

	void AppManager::createCommandBuffers()
	{

	}

	void AppManager::drawFrame()
	{

	}

} // namespace sk