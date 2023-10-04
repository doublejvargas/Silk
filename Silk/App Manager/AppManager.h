#pragma once

#include "skWindow/skWindow.h"
#include "skPipeline.h"
#include "skSwapChain.h"
#include "skDevice.h"

// std
#include <memory>
#include <vector>

namespace sk
{
	class AppManager
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		AppManager();
		~AppManager();

		// delete copy constructors because we're managing vulkan objects in this class
		AppManager(const AppManager&) = delete;
		AppManager& operator=(const AppManager&) = delete;

		void run();

	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();

		skWindow m_skWindow{ WIDTH, HEIGHT, "Hello Silk!" };
		skDevice m_skDevice{ m_skWindow };
		skSwapChain m_skSwapChain{ m_skDevice, m_skWindow.getExtent() };

		std::unique_ptr<skPipeline> m_skPipeline;
		VkPipelineLayout m_PipelineLayout;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
} // namespace sk