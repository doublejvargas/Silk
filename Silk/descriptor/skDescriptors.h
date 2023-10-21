#pragma once

#include "core\skDevice.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace sk {

	class skDescriptorSetLayout {
	public:
		class Builder {
		public:
			Builder(skDevice& device) : m_Device{ device } {}

			Builder& addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1);
			std::unique_ptr<skDescriptorSetLayout> build() const;

		private:
			skDevice& m_Device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		skDescriptorSetLayout(
			skDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~skDescriptorSetLayout();
		skDescriptorSetLayout(const skDescriptorSetLayout&) = delete;
		skDescriptorSetLayout& operator=(const skDescriptorSetLayout&) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

	private:
		skDevice& m_Device;
		VkDescriptorSetLayout descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class skDescriptorWriter;
	};

	class skDescriptorPool {
	public:
		class Builder {
		public:
			Builder(skDevice& device) : m_Device{ device } {}

			Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& setMaxSets(uint32_t count);
			std::unique_ptr<skDescriptorPool> build() const;

		private:
			skDevice& m_Device;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		skDescriptorPool(
			skDevice& device,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes);
		~skDescriptorPool();
		skDescriptorPool(const skDescriptorPool&) = delete;
		skDescriptorPool& operator=(const skDescriptorPool&) = delete;

		bool allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();

	private:
		skDevice& m_Device;
		VkDescriptorPool descriptorPool;

		friend class skDescriptorWriter;
	};

	class skDescriptorWriter {
	public:
		skDescriptorWriter(skDescriptorSetLayout& setLayout, skDescriptorPool& pool);

		skDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		skDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		bool build(VkDescriptorSet& set);
		void overwrite(VkDescriptorSet& set);

	private:
		skDescriptorSetLayout& setLayout;
		skDescriptorPool& pool;
		std::vector<VkWriteDescriptorSet> writes;
	};
}