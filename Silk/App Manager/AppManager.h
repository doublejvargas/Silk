#pragma once

#include "window/skWindow.h"
#include "renderer/skRenderer.h"
#include "core/skDevice.h"
#include "skGameObject.h"

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
		skDevice m_skDevice{ m_skWindow };
		skRenderer m_skRenderer{ m_skWindow, m_skDevice };

		std::vector<skGameObject> m_gameObjects;
	};
} // namespace sk