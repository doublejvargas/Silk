#pragma once

#include "skWindow/skWindow.h"
#include "skPipeline.h"
#include "skSwapChain.h"
#include "skDevice.h"
#include "model/skModel.h"
#include "skGameObject.h"

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
		AppManager &operator=(const AppManager&) = delete;

		void run();

	private:
		void loadGameObjects();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void renderGameObjects(VkCommandBuffer commandBuffer);
		
		// for fun
		void sierpinski(std::vector<skModel::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top);

		skWindow m_skWindow{ WIDTH, HEIGHT, "Hello Silk!" };
		skDevice m_skDevice{ m_skWindow };
		std::unique_ptr<skSwapChain> m_skSwapChain;
		std::unique_ptr<skPipeline> m_skPipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<skGameObject> m_gameObjects;
	};
} // namespace sk