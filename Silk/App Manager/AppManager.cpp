#include "AppManager.h"
#include "renderer/SimpleRenderSystem.h"

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
	AppManager::AppManager()
	{
		loadGameObjects();
	}

	AppManager::~AppManager() {}

	void AppManager::run()
	{
		SimpleRenderSystem simpleRenderSystem{ m_skDevice, m_skRenderer.getSwapChainRenderPass() };

		std::cout << "maxPushConstantSize = " << m_skDevice.properties.limits.maxPushConstantsSize << std::endl;
		while (!m_skWindow.shouldClose())
		{
			glfwPollEvents();
			if (auto commandBuffer = m_skRenderer.beginFrame()) // beginFrame() will return a nullptr if the swapchain needs to be created
			{
				/*
				* beginFrame() and beginSwapChainRenderPass() aren't combined into a single function because this down the line this will help us
				*  integrate multiple renderpasses for things such as reflections, shadows and post-processing effects.
				*/
				m_skRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects);
				m_skRenderer.endSwapChainRenderPass(commandBuffer);
				m_skRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(m_skDevice.device());
	}

	void AppManager::loadGameObjects()
	{
		std::vector<skModel::Vertex> vertices {
			{ {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
			{ {0.5f,  0.5f}, {0.0f, 1.0f, 0.0f} },
			{ {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }
		};

		auto triangle = skGameObject::createGameObject();
		auto model = std::make_shared<skModel>(m_skDevice, vertices);
		triangle.model = model;
		triangle.color = { .1f, .8f, .1f };
		triangle.transform2D.translation.x = .2f;
		triangle.transform2D.rotation = .25f * glm::two_pi<float>();

		m_gameObjects.push_back(std::move(triangle));
	}

} // namespace sk