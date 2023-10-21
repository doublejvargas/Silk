#include "SimpleRenderSystem.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace sk
{
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f }; // Corresponds to projection * view * model [read right to left]. initialized main diagonal to 1.0f, i.e., an identity matrix	 
		glm::mat4 normalMatrix{ 1.f }; // model transform only (?). corresponds to translate * rotate * scale [read right to left]
	};

	SimpleRenderSystem::SimpleRenderSystem(skDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: m_Device{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(m_Device.device(), m_pipelineLayout, nullptr); }

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline object. \n");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		skPipeline::defaultPipelineConfigInfo(pipelineConfig);
		//a render pass is basically an outline for the structure/format of the framebuffer.
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_skPipeline = std::make_unique<skPipeline>(
			m_Device,
			"res\\shaders\\bin\\simple_shader.vert.spv",
			"res\\shaders\\bin\\simple_shader.frag.spv",
			pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo &frameInfo)
	{
		// do not forget to bind the pipeline!
		m_skPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		// update (akin to onUpdate function)
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;

			// push constants before issuing draw call
			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4(); // returns transformation of this object ( projection * view * model)
			push.normalMatrix = obj.transform.normalMatrix(); // transformation of normal matrices when obj is transformed (requires diff procedure than transforming obj itself)

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);

			//bind model and draw
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

} // namespace sk