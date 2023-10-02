#pragma once

#include "skWindow/skWindow.h"

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
	};
} // namespace sk