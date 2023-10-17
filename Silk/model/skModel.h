#pragma once

#include "core/skDevice.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace sk
{
	class skModel
	{
	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		// temporary helper object to hold vertices and indices of models
		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
		};

		skModel(skDevice &device, const skModel::Builder &builder);
		~skModel();

		// delete copy constructors to avoid dangling pointers
		skModel(const skModel&) = delete;
		skModel &operator=(const skModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		skDevice &m_skDevice;

		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		uint32_t m_vertexCount;

		bool m_hasIndexBuffer = false;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;
		uint32_t m_indexCount;
	};
} // namespace sk
