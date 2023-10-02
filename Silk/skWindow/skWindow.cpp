#include "skWindow.h"
#include <cassert>

namespace sk
{
	skWindow::skWindow(int w, int h, const std::string& name) : m_Width(w), m_Height(h), m_WindowName(name)
	{
		m_InitWindow();
	}

	skWindow::~skWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void skWindow::m_InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);

		assert(m_Window != NULL);
	}

	

} // namespace sk