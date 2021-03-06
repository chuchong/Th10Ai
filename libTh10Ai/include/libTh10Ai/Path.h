#pragma once

#include "libTh10Ai/Data.h"
#include "libTh10Ai/Scene.h"

namespace th
{
	struct Action
	{
		Pointf fromPos;
		Direction fromDir;
		float_t frame;

		//int_t willCollideCount;
		//float_t minCollideFrame;
		float64_t score;
	};

	struct Result
	{
		bool valid;
		bool slow;		// 实际是否慢速
		float_t score;
		//int_t ttd;
	};

	class Path
	{
	public:
		Path(Data& data, Scene& scene,
			ItemTarget& itemTarget, EnemyTarget& enemyTarget, bool underEnemy);

		Result find(Direction dir);
		Result dfs(const Action& action);

	//private:
		static float_t CalcFarScore(Pointf player, Pointf target);
		static float_t CalcNearScore(Pointf player, Pointf target);
		static float_t CalcShootScore(Pointf player, Pointf enemy);
		static float_t CalcDepthScore(float_t frame);

		static const Direction FIND_DIRS[DIR_MAXCOUNT][DIR_MAXCOUNT];
		static const int_t FIND_SIZES[DIR_MAXCOUNT];
		static const int_t FIND_LIMIT;
		static const float_t FIND_DEPTH;
		static const Pointf RESET_POS;

		Data& m_data;
		Scene& m_scene;
		ItemTarget& m_itemTarget;
		EnemyTarget& m_enemyTarget;
		bool m_underEnemy;

		Direction m_dir;
		bool m_slowFirst;			// 是否慢速优先

		float_t m_bestScore;
		int_t m_count;
	};
}
