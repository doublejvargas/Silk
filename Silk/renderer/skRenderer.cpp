#include "skRenderer.h"

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace sk
{
	skRenderer::skRenderer(skWindow& window, skDevice& device)
		: m_skWindow{ window }, m_skDevice{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	skRenderer::~skRenderer()
	{
		freeCommandBuffers();
	}


	void skRenderer::recreateSwapChain()
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
			std::shared_ptr<skSwapChain> oldSwapChain = std::move(m_skSwapChain);
			m_skSwapChain = std::make_unique<skSwapChain>(m_skDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapChainFormats(*m_skSwapChain.get()))
			{
				throw std::runtime_error("Swap chain image or depth format has changed.\n"); 
			}
		}


		// future optimization: if renderpass is compatible, do nothing else
	}

	void skRenderer::createCommandBuffers()
	{
		m_commandBuffers.resize(skSwapChain::MAX_FRAMES_IN_FLIGHT);

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

	void skRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_skDevice.device(),
			m_skDevice.getCommandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data());

		m_commandBuffers.clear();
	}

	VkCommandBuffer skRenderer::beginFrame()
	{
		assert(!m_isFrameStarted && "Cant' call beginFrame while already in progress.\n");
	
		auto result = m_skSwapChain->acquireNextImage(&m_currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr; // indicates that frame was not successfully started
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image.\n");
		}

		m_isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer.\n");
		}

		return commandBuffer;
	}

	void skRenderer::endFrame()
	{
		assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress.\n");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer.\n");
		}
		
		auto result = m_skSwapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_skWindow.wasWindowResized())
		{
			m_skWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}

		 else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image.\n");
		}

		m_isFrameStarted = false;
		m_currentFrameIndex = (m_currentFrameIndex + 1) % skSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void skRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress.\n");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame.\n");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_skSwapChain->getRenderPass();
		renderPassInfo.framebuffer = m_skSwapChain->getFrameBuffer(m_currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		// swapchain extent here and not window extent: for high density displays, swap chain extent may be larger than window extent
		renderPassInfo.renderArea.extent = m_skSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		//clearValues[0].depthStencil = { 1.0f, 0 }; would be ignored because in our renderpass, we structured attachments to framebuffer such that index 0 = color attachment and index 1 = depth attachment 
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// VK_SUBPASS_CONTENTS_INLINE flag means that the subsequent commands will be recorded/embedded to a primary command buffer.
		// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS is the alternative to this flag and it means that subsequent commands will be executed from a secondary command buffer.
		// this implies that there is no mixing- that is, we cannot have a renderpass that uses both inline and secondary command buffers at the same time.
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_skSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_skSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ { 0, 0 }, m_skSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	}
	void skRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress.\n");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame.\n");
		vkCmdEndRenderPass(commandBuffer);
	}

} // namespace sk