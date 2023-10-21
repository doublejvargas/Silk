 #pragma once

#include "core/skPipeline.h"
#include "core/skDevice.h"
#include "model/skModel.h"
#include "skGameObject.h"
#include "camera/skCamera.h"
#include "renderer/skFrameInfo.h"

// std
#include <memory>
#include <vector>

namespace sk
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(skDevice &device, VkRenderPass renderpass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		// delete copy constructors because we're managing vulkan objects in this class
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo &frameInfo);


	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		skDevice &m_Device;
		std::unique_ptr<skPipeline> m_skPipeline;
		VkPipelineLayout m_pipelineLayout;
	};
} // namespace sk