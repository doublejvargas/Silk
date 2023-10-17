#include "skModel.h"

// std
#include <cassert>

namespace sk
{
	skModel::skModel(skDevice& device, const skModel::Builder &builder)
		: m_skDevice(device)
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	skModel::~skModel()
	{
		vkDestroyBuffer(m_skDevice.device(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_skDevice.device(), m_vertexBufferMemory, nullptr);

		if (m_hasIndexBuffer)
		{
			vkDestroyBuffer(m_skDevice.device(), m_indexBuffer, nullptr);
			vkFreeMemory(m_skDevice.device(), m_indexBufferMemory, nullptr);
		}
	}

	void skModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (m_hasIndexBuffer)
		{
			// args: command buffer, index buffer(vkbuffer type), initial offset, vk type enum ---> this MUST match the type 
			//    of the indices in the idx buffer VECTOR (not to be confused with the vkbuffer m_indexBuffer object, in this case uint32_t) 
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
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
	
	/* the createVertexBuffers and createIndexBuffers functions' purpose is to write data to the device's (GPU's) memory
	      the data is also "linked" between the host (CPU) and the device via the vkMapMemory function call, such that when we use
		  memcpy to write to the CPU memory, the same information will also be flushed to the corresponding (linked) gpu memory. */
	void skModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

		// staging (temp) buffer that will be used to 1) receive data from CPU and 2) transfer data to a more optimized gpu memory type
		//  that can't normally receive data directly from CPU.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		// allocate memory in device (GPU)
		m_skDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		/* This step copies data from CPU to staging buffer in GPU */
		// allocate and link a region in cpu memory to an existing region in gpu memory
		void *data;
		vkMapMemory(m_skDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		// *** this memcpy call flushes the same info that's copied to CPU memory, to the linked gpu memory ***
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		// unmap (unlink) the cpu memory from gpu memory, the cpu memory will then be cleaned up.
		vkUnmapMemory(m_skDevice.device(), stagingBufferMemory);

		m_skDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // this gpu memory type is more optimized and efficient than used in previous tutorials
			m_vertexBuffer,
			m_vertexBufferMemory);

		m_skDevice.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

		vkDestroyBuffer(m_skDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_skDevice.device(), stagingBufferMemory, nullptr);
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
		
		// staging (temp) buffer that will be used to 1) receive data from CPU and 2) transfer data to a more optimized gpu memory type
		//  that can't normally receive data directly from CPU.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		// allocate memory in device (GPU)
		m_skDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		/* This step copies data from CPU to staging buffer in GPU */
		// allocate and link a region in cpu memory to an existing region in gpu memory
		void* data;
		vkMapMemory(m_skDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		// *** this memcpy call flushes the same info that's copied to CPU memory, to the linked gpu memory ***
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		// unmap (unlink) the cpu memory from gpu memory, the cpu memory will then be cleaned up.
		vkUnmapMemory(m_skDevice.device(), stagingBufferMemory);

		m_skDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // this gpu memory type is more optimized and efficient than used in previous tutorials
			m_indexBuffer,
			m_indexBufferMemory);

		// copy staging buffer into optimized device buffer
		m_skDevice.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

		// clean up staging buffer
		vkDestroyBuffer(m_skDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_skDevice.device(), stagingBufferMemory, nullptr);
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
		// attribute: position
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0; // has to do with interleaving attributes in one buffer, or separate buffers for each attribute
		attributeDescriptions[0].location = 0; // refers to layout location
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position); // offset from start of vertex data to attribute (stride is the byte size of a vertex basically, or byte distance to jump from one vertex to the next)

		// attribute: color
		attributeDescriptions[1].binding = 0; // binding remains as 0 here for color attribute, as we are interleaving attributes in one buffer
		attributeDescriptions[1].location = 1; // make sure layout here matches with layout location in shader files
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //r32g32b32 because of 3 components (vec3) as opposed to location's 2 components (vec2)
		attributeDescriptions[1].offset = offsetof(Vertex, color); //This calculates byte offset. type as first argument and member name as second argument

		return attributeDescriptions;
	}

} // namespace sk