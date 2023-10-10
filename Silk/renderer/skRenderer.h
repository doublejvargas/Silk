#pragma once

#include "skWindow/skWindow.h"
#include "skSwapChain.h"
#include "skDevice.h"
#include "model/skModel.h"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace sk
{
	class skRenderer
	{
	public:
		skRenderer(skWindow &window, skDevice &device);
		~skRenderer();

		// delete copy constructors because we're managing vulkan objects in this class
		skRenderer(const skRenderer&) = delete;
		skRenderer& operator=(const skRenderer&) = delete;

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		// getters
		inline VkRenderPass getSwapChainRenderPass() const { return m_skSwapChain->getRenderPass(); }
		inline float getAspectRatio() const { return m_skSwapChain->extentAspectRatio(); }
		inline bool isFrameInProgress() const { return m_isFrameStarted; }
		inline int getFrameIndex() const {
			assert(m_isFrameStarted && "Cannot get frame index when frame not in progress.\n");
			return m_currentFrameIndex;
		}
		inline VkCommandBuffer getCurrentCommandBuffer() const {
			assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress.\n");
			return m_commandBuffers[m_currentFrameIndex];
		}

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		skWindow &m_skWindow;
		skDevice &m_skDevice;
		std::unique_ptr<skSwapChain> m_skSwapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex;
		bool m_isFrameStarted{ false };
	};
} // namespace sk