#include "AppManager.h"
#include "renderer/SimpleRenderSystem.h"
#include "camera/skCamera.h"
#include "controller/KeyboardMovementController.h"
#include "model/skBuffer.h"

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
	struct GlobalUbo
	{
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};

	AppManager::AppManager()
	{
		loadGameObjects();
	}

	AppManager::~AppManager() {}

	void AppManager::run()
	{
		skBuffer globalUboBuffer{
			m_skDevice,
			sizeof(GlobalUbo),
			skSwapChain::MAX_FRAMES_IN_FLIGHT,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_skDevice.properties.limits.minUniformBufferOffsetAlignment // since this uniform buffer will have multiple instances, we need to specify min offset alignment
		};
		globalUboBuffer.map(); // enables writing to this buffer's memory

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
				int frameIndex = m_skRenderer.getFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera };

				// update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				globalUboBuffer.writeToIndex(&ubo, frameIndex);
				globalUboBuffer.flushIndex(frameIndex);

				/* beginFrame() and beginSwapChainRenderPass() aren't combined into a single function because this down the line this will help us
				 *  integrate multiple renderpasses for things such as reflections, shadows and post-processing effects. */
				// render
				m_skRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, m_gameObjects);
				m_skRenderer.endSwapChainRenderPass(commandBuffer);
				m_skRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(m_skDevice.device());
	}

	void AppManager::loadGameObjects()
	{
		std::shared_ptr<skModel> model = skModel::createModelFromFile(m_skDevice, "res\\models\\smooth_vase.obj");
		auto smoothVase = skGameObject::createGameObject();
		smoothVase.model = model;
		smoothVase.transform.translation = { -.5f, .5f, 2.5f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.push_back(std::move(smoothVase));

		model = skModel::createModelFromFile(m_skDevice, "res\\models\\flat_vase.obj");
		auto flatVase = skGameObject::createGameObject();
		flatVase.model = model;
		flatVase.transform.translation = { .5f, .5f, 2.5f };
		flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.push_back(std::move(flatVase));
	}

} // namespace sk