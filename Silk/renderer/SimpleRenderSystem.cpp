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
		glm::mat4 transform{ 1.f }; // initialized main diagonal to 1.0f, i.e., an identity matrix

		// this is required by the SPIR-V explicit layout validation rules
		// vec3s and vec4s must be aligned to a multiple of 4N where N is the size of the component literal (in this case, it is a scalar float -> N = 4 bytes)
		//   therefore, vec2 -> 2N = 8 bytes and vec3 -> 4N = 16 bytes, thus the alignas(16)
		alignas(16) glm::vec3 color;
	};

	SimpleRenderSystem::SimpleRenderSystem(skDevice &device, VkRenderPass renderPass)
		: m_skDevice{device}
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(m_skDevice.device(), m_pipelineLayout, nullptr); }

	void SimpleRenderSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(m_skDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
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
			m_skDevice,
			"res\\shaders\\bin\\simple_shader.vert.spv",
			"res\\shaders\\bin\\simple_shader.frag.spv",
			pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<skGameObject> &gameObjects)
	{
		// update (akin to onUpdate function)
		int i = 0;
		for (auto& obj : gameObjects) {
			i += 1;
			obj.transform.rotation.y =
				glm::mod<float>(obj.transform.rotation.y + 0.01f * i, 2.f * glm::pi<float>());
			obj.transform.rotation.x =
				glm::mod<float>(obj.transform.rotation.x + 0.005f * i, 2.f * glm::pi<float>());
		}

		// render (akin to onRender)
		m_skPipeline->bind(commandBuffer);
		for (auto& obj : gameObjects)
		{
			// push constants (uniforms) before issuing draw call
			SimplePushConstantData push{};
			push.color = obj.color;
			push.transform = obj.transform.mat4(); // returns transformation of this object

			vkCmdPushConstants(
				commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			//bind model and draw
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}

} // namespace sk