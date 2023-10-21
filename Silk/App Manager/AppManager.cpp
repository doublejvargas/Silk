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
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is intensity
		glm::vec4 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ 1.f };		// w is light intensity
	};

	AppManager::AppManager()
	{
		// I'm able to link function calls like this because each function/method returns a REFERENCE to the object.
		m_globalPool = skDescriptorPool::Builder(m_Device)
			.setMaxSets(skSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, skSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
	}

	AppManager::~AppManager() {}

	void AppManager::run()
	{
		std::vector<std::unique_ptr<skBuffer>> uboBuffers(skSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<skBuffer>(
				m_Device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->map();
		}

		auto globalSetLayout =
			sk::skDescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(skSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			skDescriptorWriter(*globalSetLayout, *m_globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ m_Device, m_skRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
		skCamera camera{};
		camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		auto viewerObject = skGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		std::cout << "maxPushConstantSize = " << m_Device.properties.limits.maxPushConstantsSize << std::endl;
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
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);

			if (auto commandBuffer = m_skRenderer.beginFrame()) // beginFrame() will return a nullptr if the swapchain needs to be created
			{
				int frameIndex = m_skRenderer.getFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};

				// update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				/* beginFrame() and beginSwapChainRenderPass() aren't combined into a single function because this down the line this will help us
				 *  integrate multiple renderpasses for things such as reflections, shadows and post-processing effects. */
				// render
				m_skRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, m_gameObjects);
				m_skRenderer.endSwapChainRenderPass(commandBuffer);
				m_skRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(m_Device.device());
	}

	void AppManager::loadGameObjects()
	{
		std::shared_ptr<skModel> model = skModel::createModelFromFile(m_Device, "res\\models\\flat_vase.obj");
		auto flatVase = skGameObject::createGameObject();
		flatVase.model = model;
		flatVase.transform.translation = { -.5f, .5f, 0.f };
		flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.push_back(std::move(flatVase));
		
		model = skModel::createModelFromFile(m_Device, "res\\models\\smooth_vase.obj");
		auto smoothVase = skGameObject::createGameObject();
		smoothVase.model = model;
		smoothVase.transform.translation = { .5f, .5f, 0.f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.push_back(std::move(smoothVase));

		model = skModel::createModelFromFile(m_Device, "res\\models\\quad.obj");
		auto floor = skGameObject::createGameObject();
		floor.model = model;
		floor.transform.translation = { .0f, .5f, 0.f };
		floor.transform.scale = { 3.f, 1.f, 3.f };
		m_gameObjects.push_back(std::move(floor));
	}

} // namespace sk