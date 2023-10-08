#pragma once

#include "skPipeline.h"
#include "skDevice.h"
#include "model/skModel.h"
#include "skGameObject.h"

// std
#include <memory>
#include <vector>

namespace sk
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(skDevice &device, VkRenderPass renderpass);
		~SimpleRenderSystem();

		// delete copy constructors because we're managing vulkan objects in this class
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<skGameObject> &gameObjects);


	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		skDevice &m_skDevice;
		std::unique_ptr<skPipeline> m_skPipeline;
		VkPipelineLayout m_pipelineLayout;
	};
} // namespace sk