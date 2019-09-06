#pragma once

#include <thread>
#include <atomic>
#include <memory>

#include "libTH10AI/Th10Ai.h"

namespace th
{
	class HookThread
	{
	public:
		HookThread();
		~HookThread();

		bool isDone() const;

	private:
		void hookProc();

		std::thread m_thread;
		std::atomic_bool m_done;
	};
}