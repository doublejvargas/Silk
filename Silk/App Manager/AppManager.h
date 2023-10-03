#pragma once

#include "skWindow/skWindow.h"
#include "skPipeline.h"

#include <string>

namespace sk
{
	class AppManager
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void run();

	private:
		skWindow m_skWindow{ WIDTH, HEIGHT, "Hello Silk!" };
		skPipeline m_skPipeline{ "res\\shaders\\bin\\simple_shader.vert.spv", "res\\shaders\\bin\\simple_shader.frag.spv" };
	};
} // namespace sk