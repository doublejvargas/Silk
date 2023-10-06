#include "skModel.h"

// std
#include <cassert>

namespace sk
{
	skModel::skModel(skDevice& device, const std::vector<Vertex>& vertices)
		: m_skDevice(device)
	{
		createVertexBuffers(vertices);
	}

	skModel::~skModel()
	{
		vkDestroyBuffer(m_skDevice.device(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_skDevice.device(), m_vertexBufferMemory, nullptr);
	}

	void skModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void skModel::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}

	void skModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
		m_skDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_vertexBuffer,
			m_vertexBufferMemory);

		void *data;
		vkMapMemory(m_skDevice.device(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_skDevice.device(), m_vertexBufferMemory);
	}

	std::vector<VkVertexInputBindingDescription> skModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> skModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0; // has to do with interleaving attributes in one buffer, or separate buffers for each attribute
		attributeDescriptions[0].location = 0; // refers to layout location
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position); // offset from start of vertex data to attribute (stride is the byte size of a vertex basically, or byte distance to jump from one vertex to the next)

		attributeDescriptions[1].binding = 0; // binding remains as 0 here for color attribute, as we are interleaving attributes in one buffer
		attributeDescriptions[1].location = 1; // make sure layout here matches with layout location in shader files
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //r32g32b32 because of 3 components (vec3) as opposed to location's 2 components (vec2)
		attributeDescriptions[1].offset = offsetof(Vertex, color); //This calculates byte offset. type as first argument and member name as second argument

		return attributeDescriptions;
	}

} // namespace sk