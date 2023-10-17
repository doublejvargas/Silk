#include "AppManager.h"
#include "renderer/SimpleRenderSystem.h"
#include "camera/skCamera.h"
#include "controller/KeyboardMovementController.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

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
		skCamera camera{};
		camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		auto viewerObject = skGameObject::createGameObject();
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		std::cout << "maxPushConstantSize = " << m_skDevice.properties.limits.maxPushConstantsSize << std::endl;
		while (!m_skWindow.shouldClose())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;
			
			//frameTime = glm::min(frameTime, MAX_FRAME_TIME);

			cameraController.moveInPlaneXZ(m_skWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = m_skRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.f);

			if (auto commandBuffer = m_skRenderer.beginFrame()) // beginFrame() will return a nullptr if the swapchain needs to be created
			{
				/*
				* beginFrame() and beginSwapChainRenderPass() aren't combined into a single function because this down the line this will help us
				*  integrate multiple renderpasses for things such as reflections, shadows and post-processing effects.
				*/
				m_skRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects, camera);
				m_skRenderer.endSwapChainRenderPass(commandBuffer);
				m_skRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(m_skDevice.device());
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
	std::unique_ptr<skModel> createCubeModel(skDevice &device, glm::vec3 offset) {
		skModel::Builder modelBuilder{};
		modelBuilder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}

		modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		return std::make_unique<skModel>(device, modelBuilder);
	}

	void AppManager::loadGameObjects()
	{
		std::shared_ptr<skModel> model = createCubeModel(m_skDevice, { .0f, .0f, .0f });
		auto cube = skGameObject::createGameObject();
		cube.model = model;
		cube.transform.translation = { .0f, .0f, 2.5f };
		cube.transform.scale = { .5f, .5f, .5f };
		m_gameObjects.push_back(std::move(cube));
	}

} // namespace sk