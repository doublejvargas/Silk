#pragma once

#include "skWindow/skWindow.h"
#include "skPipeline.h"
#include "skDevice.h"

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
		skDevice m_skDevice{ m_skWindow };
		skPipeline m_skPipeline{ 
			m_skDevice, 
			"res\\shaders\\bin\\simple_shader.vert.spv", 
			"res\\shaders\\bin\\simple_shader.frag.spv", 
			skPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT) };
	};
} // namespace sk