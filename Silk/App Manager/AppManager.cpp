#include "AppManager.h"

#include <stdexcept>
#include <array>

namespace sk
{

	AppManager::AppManager()
	{
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	AppManager::~AppManager()
	{
		vkDestroyPipelineLayout(m_skDevice.device(), m_pipelineLayout, nullptr); 
		//destroyCommandBuffers()?
		// When our skDevice is deleted, the command pool is also deleted as well as any buffers allocated from that pool.
	}

	void AppManager::run()
	{
		while (!m_skWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(m_skDevice.device());
	}

	// for fun
	void AppManager::sierpinski(
		std::vector<skModel::Vertex>& vertices,
		int depth,
		glm::vec2 left,
		glm::vec2 right,
		glm::vec2 top) {
		if (depth <= 0) {
			vertices.push_back({ top });
			vertices.push_back({ right });
			vertices.push_back({ left });
		}
		else {
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			sierpinski(vertices, depth - 1, left, leftRight, leftTop);
			sierpinski(vertices, depth - 1, leftRight, right, rightTop);
			sierpinski(vertices, depth - 1, leftTop, rightTop, top);
		}
	}

	void AppManager::loadModels()
	{
		std::vector<skModel::Vertex> vertices {
			{ {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
			{ {0.5f,  0.5f}, {0.0f, 1.0f, 0.0f} },
			{ {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }
		};
		
		m_skModel = std::make_unique<skModel>(m_skDevice, vertices);
	}

	void AppManager::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(m_skDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline object. \n");
		}
	}

	void AppManager::createPipeline()
	{
		assert(m_skSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		skPipeline::defaultPipelineConfigInfo(pipelineConfig);
		//a render pass is basically an outline for the structure/format of the framebuffer.
		pipelineConfig.renderPass = m_skSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_skPipeline = std::make_unique<skPipeline>(
			m_skDevice,
			"res\\shaders\\bin\\simple_shader.vert.spv",
			"res\\shaders\\bin\\simple_shader.frag.spv",
			pipelineConfig);
	}

	void AppManager::recreateSwapChain()
	{
		auto extent = m_skWindow.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = m_skWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_skDevice.device());

		if (m_skSwapChain == nullptr)
			m_skSwapChain = std::make_unique<skSwapChain>(m_skDevice, extent);
		else
		{
			m_skSwapChain = std::make_unique<skSwapChain>(m_skDevice, extent, std::move(m_skSwapChain));
			freeCommandBuffers();
			createCommandBuffers();
		}
			

		// future optimization: if renderpass is compatible, do nothing else
		createPipeline();
	}

	void AppManager::createCommandBuffers()
	{
		// resize to the number of frame buffers (or command buffers?) (will be 2)
		m_commandBuffers.resize(m_skSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		// Primary command buffers can be submitted to a queue for execution, but can't be called by another command buffer.
		// Secondary command buffers can't be submitted to a queue for execution, but can be called by another command buffer.
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_skDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_skDevice.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers.\n");
		}
	}

	void AppManager::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_skDevice.device(), 
			m_skDevice.getCommandPool(), 
			static_cast<uint32_t>(m_commandBuffers.size()), 
			m_commandBuffers.data());

		m_commandBuffers.clear();
	}

	void AppManager::recordCommandBuffer(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer.\n");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_skSwapChain->getRenderPass();
		renderPassInfo.framebuffer = m_skSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		// swapchain extent here and not window extent: for high density displays, swap chain extent may be larger than window extent
		renderPassInfo.renderArea.extent = m_skSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		//clearValues[0].depthStencil = { 1.0f, 0 }; would be ignored because in our renderpass, we structured attachments to framebuffer such that index 0 = color attachment and index 1 = depth attachment 
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// VK_SUBPASS_CONTENTS_INLINE flag means that the subsequent commands will be recorded/embedded to a primary command buffer.
		// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS is the alternative to this flag and it means that subsequent commands will be executed from a secondary command buffer.
		// this implies that there is no mixing- that is, we cannot have a renderpass that uses both inline and secondary command buffers at the same time.
		vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_skSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_skSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ { 0, 0 }, m_skSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

		m_skPipeline->bind(m_commandBuffers[imageIndex]);
		// 3 vertices, 1 instance, 0 & 0 offsets
		// instances are used when we want to draw multiple copies of the same vertex data (similar to batch rendering). One application of this is particle systems.
		//vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);

		//drawing using vertex buffer
		m_skModel->bind(m_commandBuffers[imageIndex]);
		m_skModel->draw(m_commandBuffers[imageIndex]);

		vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer.\n");
		}
	}

	void AppManager::drawFrame()
	{
		uint32_t imageIndex;
		auto result = m_skSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image.\n");
		}

		recordCommandBuffer(imageIndex);  
		result = m_skSwapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_skWindow.wasWindowResized())
		{
			m_skWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image.\n");
		}
	}

} // namespace sk