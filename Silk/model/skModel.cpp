#include "skModel.h"
#include "skUtils.h"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <iostream>
#include <unordered_map>

namespace std
{
	template<>
	struct hash<sk::skModel::Vertex>
	{
		size_t operator()(sk::skModel::Vertex const &vertex) const {
			size_t seed = 0;
			sk::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace sk
{
	skModel::skModel(skDevice& device, const skModel::Builder &builder)
		: m_Device(device)
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	skModel::~skModel() {}

	void skModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (m_hasIndexBuffer)
		{
			// args: command buffer, index buffer(vkbuffer type), initial offset, vk type enum ---> this MUST match the type 
			//    of the indices in the idx buffer VECTOR (not to be confused with the vkbuffer m_indexBuffer object, in this case uint32_t) 
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void skModel::draw(VkCommandBuffer commandBuffer)
	{
		if (m_hasIndexBuffer)
		{
			// args: command buffer, index count, instance count, first index, vertex offset, first instance
			vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
		}
		else
		{
			// args: vkCmdDraw(command buffer, vertex count, instance count, first vertex, first instance)
			vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
		}
	}

	std::unique_ptr<skModel> skModel::createModelFromFile(skDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(filepath);
		std::cout << "Vertex count for " << filepath << " : " << builder.vertices.size() << std::endl;
		return std::make_unique<skModel>(device, builder);
	}

	/* the createVertexBuffers and createIndexBuffers functions' purpose is to write data to the device's (GPU's) memory
	      the data is also "linked" between the host (CPU) and the device via the vkMapMemory function call, such that when we use
		  memcpy to write to the CPU memory, the same information will also be flushed to the corresponding (linked) gpu memory. */
	void skModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		// staging (temp) buffer that will be used to 1) receive data from CPU and 2) transfer data to a more optimized gpu memory type
		//  that can't normally receive data directly from CPU.
		skBuffer stagingBuffer{
			m_Device,
			vertexSize,
			m_vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		// Copy data from CPU to staging buffer in GPU
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		// Create vertex buffer (smart ptr)
		m_vertexBuffer = std::make_unique<skBuffer>(
			m_Device,
			vertexSize,
			m_vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT // this gpu memory type is more optimized and efficient than used in previous tutorials
			);
		
		// Copy contents from staging buffer to optimized device buffer
		m_Device.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
	}

	/* see comment on createVertexBuffers function */
	void skModel::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		m_indexCount = static_cast<uint32_t>(indices.size());
		m_hasIndexBuffer = m_indexCount > 0;

		if (!m_hasIndexBuffer)
			return;

		assert(m_indexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		// staging (temp) buffer that will be used to 1) receive data from CPU and 2) transfer data to a more optimized gpu memory type
		//  that can't normally receive data directly from CPU
		skBuffer stagingBuffer{
			m_Device,
			indexSize,
			m_indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		
		// Copy data from CPU to staging buffer in GPU
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		// Create index buffer (smart ptr)
		m_indexBuffer = std::make_unique<skBuffer>(
			m_Device,
			indexSize,
			m_indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT // this gpu memory type is more optimized and efficient than used in previous tutorials
		);

		// copy staging buffer into optimized device buffer
		m_Device.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
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
		
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		// element components: binding, location (as in layout location, see shaders), format (vk format enum), offset (offset of attribute inside
		//   the "Vertex" struct. names of these components correspond to fields inside the "Vertex" struct.
		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

		/* More verbose description, see comments for understanding.
		attributeDescriptions[0].binding = 0; // has to do with interleaving attributes in one buffer, or separate buffers for each attribute
		attributeDescriptions[0].location = 0; // refers to layout location
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position); // offset from start of vertex data to attribute (stride is the byte size of a vertex basically, or byte distance to jump from one vertex to the next)
		*/

		return attributeDescriptions;
	}

	void skModel::Builder::loadModel(const std::string& filepath)
	{
		tinyobj::attrib_t attrib;					// stores positions, colors, normals & texture coordinates
		std::vector<tinyobj::shape_t> shapes;		// stores indices
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
				}

				if (index.normal_index >= 0)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size()); // clever way of determining index, size of vertices per iteration corresponds to unique index
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

} // namespace sk