#pragma once

#include "core/skDevice.h"
#include "skBuffer.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace sk
{
	class skModel
	{
	public:

		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex &other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

		// temporary helper object to hold vertices and indices of models
		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);
		};

		skModel(skDevice &device, const skModel::Builder &builder);
		~skModel();

		// delete copy constructors to avoid dangling pointers
		skModel(const skModel&) = delete;
		skModel &operator=(const skModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		static std::unique_ptr<skModel> createModelFromFile(skDevice& device, const std::string& filepath);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		skDevice &m_skDevice;

		std::unique_ptr<skBuffer> m_vertexBuffer;
		uint32_t m_vertexCount;

		bool m_hasIndexBuffer = false;
		std::unique_ptr<skBuffer> m_indexBuffer;
		uint32_t m_indexCount;
	};
} // namespace sk
