#include "libTh10Ai/Common.h"
#include "libTh10Ai/Th10Ai.h"

#include <sstream>
#include <boost/log/trivial.hpp>

#include "libTh10Ai/libTh10Ai.h"
#include "libTh10Ai/Path.h"

namespace th
{
	Th10Ai::Th10Ai() :
		m_done(false),
		m_active(false),
		m_bombCount(0),
		m_prevDir(DIR_HOLD),
		m_prevSlow(false),
		m_data(m_reader), p(true), planar(Scene::SIZE.width - 16, Scene::SIZE.height)
	{
		m_scene.split(6);

		AllocConsole();
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);

		HWND window = GetConsoleWindow();
		HMENU menu = GetSystemMenu(window, FALSE);
		EnableMenuItem(menu, SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);

		m_thread = std::thread(&Th10Ai::run, this);
		// TODO 读入文件
		std::cout << "initialize planar" << std::endl;
		planar.initialize(std::string("D:/linear_weights"));
	}

	Th10Ai::~Th10Ai()
	{
		m_done = true;
		m_d3d9Hook.notifyPresent();
		if (m_thread.joinable())
			m_thread.join();
	}

	bool Th10Ai::IsKeyPressed(int vKey)
	{
		return (GetAsyncKeyState(vKey) & 0x8000) != 0;
	}

	void Th10Ai::run()
	{
		try
		{
			std::cout << "A to start AI, S to stop AI, D to quit AI" << std::endl;

			while (!m_done)
			{
				if (IsKeyPressed('A'))
				{
					start();
				}
				else if (IsKeyPressed('S'))
				{
					stop();
				}
				else if (IsKeyPressed('D'))
				{
					break;
				}
				else if (IsKeyPressed('F'))
				{
					print();
				}

				update();
			}

			stop();
			std::cout << "exit AI" << std::endl;
		}
		catch (...)
		{
			std::ostringstream oss;
			PrintAllException(oss);
			BOOST_LOG_TRIVIAL(error) << oss.str() << std::endl;
		}

		g_libTh10Ai.exit();
	}

	void Th10Ai::print()
	{
		p = true;

		m_data.print();
	}

	void Th10Ai::start()
	{
		if (!m_active)
		{
			m_di8Hook.clear();
			m_di8Hook.commit();

			m_d3d9Hook.enable(true);
			m_di8Hook.enable(true);

			m_active = true;
			std::cout << "startAI" << std::endl;
		}
	}

	void Th10Ai::stop()
	{
		if (m_active)
		{
			m_di8Hook.clear();
			m_di8Hook.commit();

			m_di8Hook.enable(false);
			m_d3d9Hook.enable(false);

			m_active = false;
			std::cout << "stop AI" << std::endl;
			std::cout << "death:" << m_bombCount << std::endl;
		}
	}

	void Th10Ai::update()
	{
		if (!m_active)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			return;
		}

		if (!m_d3d9Hook.waitPresent())
			std::cout << "read is delay" << std::endl;

		//static int_t fps = 0;
		//static std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		//++fps;
		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//std::chrono::milliseconds interval = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		//if (interval.count() >= 1000)
		//{
		//	std::cout << "fps: " << fps << std::endl;
		//	fps = 0;
		//	begin += std::chrono::milliseconds(1000);
		//}

		//std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

		m_data.update();
		m_data.getPlayer().checkPrevMove(m_prevDir, m_prevSlow);

		//std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		//time_t e1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
		//std::cout << "e1: " << e1 << std::endl;

		m_scene.clearAll();
		m_scene.splitEnemies(m_data.getEnemies());
		m_scene.splitBullets(m_data.getBullets());
		m_scene.splitLasers(m_data.getLasers());

		//std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		//time_t e2 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		//std::cout << "e2: " << e2 << std::endl;

		//if (p)
		//{
		//	CellCollideResult ccResult = m_scene.collideAll(m_data.getPlayer(), 0.0f);
		//	std::cout << ccResult.collided;
		//}

		handleBomb();
		handleTalk();
		handleShoot();
		handleMove();

		//if (p)
		//{
		//	Player temp = m_data.getPlayer();
		//	temp.advance(m_prevDir, m_prevSlow);
		//	CellCollideResult ccResult1 = m_scene.collideAll(temp, 1.0f);
		//	std::cout << " " << ccResult1.collided << std::endl;
		//}

		//std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
		//time_t e3 = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
		//std::cout << "e3: " << e3 << std::endl;

		m_di8Hook.commit();
	}

	// 处理炸弹
	bool Th10Ai::handleBomb()
	{
		if (m_data.getPlayer().isColliding())
		{
			//p = false;
			//m_data.print();

			//CellCollideResult ccr = m_scene.collideAll(m_data.getPlayer(), 0.0f);
			//std::cout << ccr.collided << std::endl;

			m_di8Hook.keyPress(DIK_X);
			++m_bombCount;
			std::cout << "death:" << m_bombCount << std::endl;
			return true;
		}
		else
		{
			m_di8Hook.keyRelease(DIK_X);
			return false;
		}
	}

	// 处理对话
	bool Th10Ai::handleTalk()
	{
		if (m_data.isTalking())
		{
			m_di8Hook.keyPress(DIK_LCONTROL);
			return true;
		}
		else
		{
			m_di8Hook.keyRelease(DIK_LCONTROL);
			return false;
		}
	}

	// 处理攻击
	bool Th10Ai::handleShoot()
	{
		if (m_data.hasEnemy())
		{
			m_di8Hook.keyPress(DIK_Z);
			return true;
		}
		else
		{
			m_di8Hook.keyRelease(DIK_Z);
			return false;
		}
	}

	// 处理移动
	bool Th10Ai::handleMove_dfs()
	{
		if (!m_data.getPlayer().isNormalStatus())
			return false;

		ItemTarget itemTarget = m_data.findItem();
		EnemyTarget enemyTarget = m_data.findEnemy();
		bool underEnemy = m_data.isUnderEnemy();

		float_t bestScore = std::numeric_limits<float_t>::lowest();
		Direction bestDir = DIR_NONE;
		bool bestSlow = false;

		for (int_t i = DIR_HOLD; i < DIR_MAXCOUNT; ++i)
		{
			Direction dir = static_cast<Direction>(i);

			Path path(m_data, m_scene, itemTarget, enemyTarget, underEnemy);
			Result result = path.find(dir);

			if (result.valid && path.m_bestScore > bestScore)
			{
				bestScore = path.m_bestScore;
				bestDir = path.m_dir;
				bestSlow = result.slow;
			}
		}

		if (bestDir != DIR_NONE)
		{
			move(bestDir, bestSlow);
		}
		else
		{
			move(DIR_HOLD, false);
			std::cout << "no path be safe" << std::endl;
		}

		return true;
	}

	// 读表，处理移动
	bool Th10Ai::handleMove()
	{
		if (!m_data.getPlayer().isNormalStatus())
			return false;

		//std::vector<Enemy> enemies = m_data.getEnemies();
		std::vector<Bullet> bullets = m_data.getBullets();

		Pointf player = m_data.getPlayer().getPosition();

		int action = planar.chooseAction(bullets, player.x, player.y);

		// 坐标原点移到左上角
		//player.x += (Scene::SIZE.width / 2.0f);

		//// count bullet and enemy distribution
		//std::vector<int> region_dens(4);
		//// closest region
		//int min_dist_id = 0;
		//float min_dist = 100000;
		//int relative_dist_type = 0;
		//int id = 3 * int(player.x / (Scene::SIZE.width / 3)) + int(player.y / (Scene::SIZE.height / 3));

		//for (const Bullet& bullet : bullets)
		//{
		//	if (!Scene::IsInScene(bullet.getPosition())) {
		//		continue;
		//	}
		//	float x = bullet.x + (Scene::SIZE.width / 2.0f);
		//	float y = bullet.y;
		//	int bin = 0;
		//	if (x >= player.x && y >= player.y) {
		//		bin = 0;
		//	}
		//	else if (x < player.x && y >= player.y) {
		//		bin = 1;
		//	}
		//	else if (x < player.x && y < player.y) {
		//		bin = 2;
		//	}
		//	else {
		//		bin = 3;
		//	}
		//	region_dens[bin] += 1;
		//	float dist = abs(x - player.x) + abs(y - player.y);
		//	if (dist < min_dist) {
		//		min_dist_id = bin;
		//		min_dist = dist;
		//	}
		//}

		//float ratio = min_dist / (Scene::SIZE.width + Scene::SIZE.height);
		//if (ratio > 0.5) {
		//	relative_dist_type = 0;
		//}
		//else if (ratio > 0.05) {
		//	relative_dist_type = 1;
		//}
		//else {
		//	relative_dist_type = 2;
		//}

		//if (relative_dist_type == 2) {
		//	return handleMove_dfs();
		//}

		//int max_dense = -1;
		//int max_id = -1;
		//for (int i = 0; i < 4; i++) {
		//	if (region_dens[i] > max_dense) {
		//		max_dense = region_dens[i];
		//		max_id = i;
		//	}
		//}
		//int index = 48 * id + 16 * relative_dist_type + 4 * max_id + min_dist_id;
		//int policy[432] = { 2,1,3,3,0,3,3,0,1,0,1,2,2,3,0,1,2,3,1,1,2,2,1,1,3,1,1,3,0,2,0,3,2,0,1,1,3,1,1,1,1,0,2,3,1,0,3,3,1,2,2,1,3,0,0,2,3,0,1,0,1,1,0,2,1,3,1,1,3,3,1,2,2,1,0,3,3,1,2,0,2,3,2,2,2,1,1,1,2,0,0,0,3,1,1,2,3,2,0,1,1,2,2,1,0,3,0,2,2,1,0,0,3,1,1,3,3,3,0,2,2,1,0,2,2,2,3,1,3,1,3,2,2,1,2,0,1,3,0,0,0,0,1,2,0,2,0,3,1,0,1,2,3,0,0,3,0,3,1,1,2,3,3,2,1,1,3,2,2,3,1,0,3,2,1,3,0,3,3,0,2,1,3,1,2,3,1,2,3,3,2,2,2,2,3,1,2,1,1,3,1,3,0,3,3,2,3,2,0,3,1,2,3,1,3,3,2,3,0,3,0,3,3,1,3,0,3,1,0,0,3,0,3,0,3,1,3,2,1,1,0,1,1,1,0,3,3,2,0,3,0,3,2,3,2,3,0,3,1,3,2,3,2,0,0,0,0,0,3,2,0,3,2,0,3,0,2,2,0,2,2,3,3,0,0,0,3,1,0,2,1,0,3,1,2,1,0,1,1,1,2,1,0,1,2,0,0,2,0,1,1,3,0,1,1,1,0,3,1,1,0,3,0,0,0,2,3,1,3,1,0,1,3,1,1,1,2,2,3,3,0,1,1,3,3,3,1,1,2,2,2,3,3,3,0,0,2,0,1,3,3,0,2,1,2,0,1,1,0,3,3,3,2,3,2,2,0,0,3,0,3,3,1,3,1,0,1,3,1,3,2,1,3,3,2,1,0,1,1,1,3,1,2,0,1,2,1,3,2,0,2,1,2,3,0,0,1,3,3,1,1,1,2,1,0,3,2,1,1,0,2,1 };
		//int action = policy[index];

		
		switch (action)
		{
		case 0:
			move(DIR_UP, false);
			break;
		case 1:
			move(DIR_DOWN, false);
			break;
		case 2:
			move(DIR_LEFT, false);
			break;
		case 3:
			move(DIR_RIGHT, false);
			break;
		default:
			break;
		}
		return true;
	}


	void Th10Ai::move(Direction dir, bool slow)
	{
		if (slow)
			m_di8Hook.keyPress(DIK_LSHIFT);
		else
			m_di8Hook.keyRelease(DIK_LSHIFT);

		switch (dir)
		{
		case DIR_HOLD:
			m_di8Hook.keyRelease(DIK_LEFT);
			m_di8Hook.keyRelease(DIK_RIGHT);
			m_di8Hook.keyRelease(DIK_UP);
			m_di8Hook.keyRelease(DIK_DOWN);
			break;

		case DIR_LEFT:
			m_di8Hook.keyPress(DIK_LEFT);
			m_di8Hook.keyRelease(DIK_RIGHT);
			m_di8Hook.keyRelease(DIK_UP);
			m_di8Hook.keyRelease(DIK_DOWN);
			break;

		case DIR_RIGHT:
			m_di8Hook.keyRelease(DIK_LEFT);
			m_di8Hook.keyPress(DIK_RIGHT);
			m_di8Hook.keyRelease(DIK_UP);
			m_di8Hook.keyRelease(DIK_DOWN);
			break;

		case DIR_UP:
			m_di8Hook.keyRelease(DIK_LEFT);
			m_di8Hook.keyRelease(DIK_RIGHT);
			m_di8Hook.keyPress(DIK_UP);
			m_di8Hook.keyRelease(DIK_DOWN);
			break;

		case DIR_DOWN:
			m_di8Hook.keyRelease(DIK_LEFT);
			m_di8Hook.keyRelease(DIK_RIGHT);
			m_di8Hook.keyRelease(DIK_UP);
			m_di8Hook.keyPress(DIK_DOWN);
			break;

		case DIR_LEFTUP:
			m_di8Hook.keyPress(DIK_LEFT);
			m_di8Hook.keyRelease(DIK_RIGHT);
			m_di8Hook.keyPress(DIK_UP);
			m_di8Hook.keyRelease(DIK_DOWN);
			break;

		case DIR_RIGHTUP:
			m_di8Hook.keyRelease(DIK_LEFT);
			m_di8Hook.keyPress(DIK_RIGHT);
			m_di8Hook.keyPress(DIK_UP);
			m_di8Hook.keyRelease(DIK_DOWN);
			break;

		case DIR_LEFTDOWN:
			m_di8Hook.keyPress(DIK_LEFT);
			m_di8Hook.keyRelease(DIK_RIGHT);
			m_di8Hook.keyRelease(DIK_UP);
			m_di8Hook.keyPress(DIK_DOWN);
			break;

		case DIR_RIGHTDOWN:
			m_di8Hook.keyRelease(DIK_LEFT);
			m_di8Hook.keyPress(DIK_RIGHT);
			m_di8Hook.keyRelease(DIK_UP);
			m_di8Hook.keyPress(DIK_DOWN);
			break;
		}

		m_prevDir = dir;
		m_prevSlow = slow;
	}
}
