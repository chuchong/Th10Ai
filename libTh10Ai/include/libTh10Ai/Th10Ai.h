#pragma once

#include <thread>
#include <atomic>

#include "libTh10Ai/D3D9Hook.h"
#include "libTh10Ai/DI8Hook.h"
#include "libTh10Ai/Reader.h"
#include "libTh10Ai/Data.h"
#include "libTh10Ai/Scene.h"
#include "../LinearPlanner.h"
namespace th
{
	class Th10Ai
	{
	public:
		Th10Ai();
		~Th10Ai();

	private:
		static bool IsKeyPressed(int vKey);

		void run();
		void print();
		void start();
		void stop();
		void update();

		bool handleBomb();
		bool handleTalk();
		bool handleShoot();
		bool handleMove();
		bool handleMove_dfs();

		void move(Direction dir, bool slow);

		std::thread m_thread;
		std::atomic_bool m_done;
		bool m_active;
		int_t m_bombCount;
		Direction m_prevDir;
		bool m_prevSlow;

		D3D9Hook m_d3d9Hook;
		DI8Hook m_di8Hook;
		Reader m_reader;
		Data m_data;
		Scene m_scene;

		LinearPlanner planar;
		bool p;
	};
}
