#pragma once

#include "camera/skCamera.h"

// lib
#include <vulkan/vulkan.h>

namespace sk
{
	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		skCamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};
} // namespace sk
