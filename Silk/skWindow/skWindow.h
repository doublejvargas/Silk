#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace sk
{
	class skWindow
	{
	private:
		void m_InitWindow();
		static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

		int m_Width;
		int m_Height;
		bool m_frameBufferResized = false;

		std::string m_windowName;
		GLFWwindow* m_Window;

	public:
		skWindow(int w, int h, const std::string& name);
		~skWindow();

		// "resource acquisition is initialization", delete copy constructor & operator to avoid dangling pointers
		skWindow(const skWindow &) = delete;
		skWindow &operator=(const skWindow &) = delete;

		inline bool shouldClose() const { return glfwWindowShouldClose(m_Window); }
		inline VkExtent2D getExtent() const { return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) }; }
		inline bool wasWindowResized() const { return m_frameBufferResized; }
		inline void resetWindowResizedFlag() { m_frameBufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	};

} //namespace sk
