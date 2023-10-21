#pragma once

#include "window/skWindow.h"
#include "renderer/skRenderer.h"
#include "core/skDevice.h"
#include "skGameObject.h"
#include "descriptor/skDescriptors.h"

// std
#include <memory>
#include <vector>

namespace sk
{
	class AppManager
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		AppManager();
		~AppManager();

		// delete copy constructors because we're managing vulkan objects in this class
		AppManager(const AppManager&) = delete;
		AppManager &operator=(const AppManager&) = delete;

		void run();

	private:
		void loadGameObjects();

		skWindow m_skWindow{ WIDTH, HEIGHT, "Hello Silk!" };
		skDevice m_Device{ m_skWindow };
		skRenderer m_skRenderer{ m_skWindow, m_Device };

		// note: order of declarations matters here
		// memory is allocated for declared objects from top to bottom, memory is deallocated from bottom to top
		std::unique_ptr<skDescriptorPool> m_globalPool{};
		std::vector<skGameObject> m_gameObjects;
	};
} // namespace sk