#include "AppManager.h"

namespace sk
{
	void AppManager::run()
	{
		while (!m_skWindow.shouldClose())
		{
			glfwPollEvents();
		}
	}
} // namespace sk