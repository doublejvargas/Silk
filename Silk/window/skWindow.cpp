#include "skWindow.h"

#include <cassert>
#include <stdexcept>

namespace sk
{
	skWindow::skWindow(int w, int h, const std::string& name) : m_Width(w), m_Height(h), m_windowName(name)
	{
		m_InitWindow();
	}

	

	skWindow::~skWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void skWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface \n");
		}
	}

	void skWindow::m_InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, frameBufferResizeCallback);

		assert(m_Window != NULL);
	}


	void skWindow::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto newWindow = reinterpret_cast<skWindow*>(glfwGetWindowUserPointer(window));
		newWindow->m_frameBufferResized = true;
		newWindow->m_Width = width;
		newWindow->m_Height = height;
	}

} // namespace sk