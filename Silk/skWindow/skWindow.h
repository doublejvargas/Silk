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

		const int m_Width;
		const int m_Height;
		std::string m_WindowName;
		GLFWwindow* m_Window;

	public:
		skWindow(int w, int h, const std::string& name);
		~skWindow();

		// "resource acquisition is initialization", delete copy constructor & operator to avoid dangling pointers
		skWindow(const skWindow &) = delete;
		skWindow& operator=(const skWindow &) = delete;

		inline bool shouldClose() const { return glfwWindowShouldClose(m_Window); }
	};

} //namespace sk
