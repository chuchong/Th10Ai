#pragma once

#include "Th10Ai/Common.h"

#include <vector>

#include "Th10Ai/Th10Data.h"
#include "Th10Ai/Player.h"
#include "Th10Ai/Item.h"
#include "Th10Ai/Enemy.h"
#include "Th10Ai/Bullet.h"
#include "Th10Ai/Laser.h"

namespace th
{
	struct ItemTarget
	{
		bool found;
		Item item;
	};

	struct EnemyTarget
	{
		bool found;
		Enemy enemy;
	};

	class Status
	{
	public:
		Status();

		void update(const StatusData& data);
		void print();

		// Player

		// Item
		ItemTarget findItem();

		// Enemy
		bool hasEnemy() const;
		bool isBoss() const;
		bool isTalking() const;
		bool isUnderEnemy() const;
		EnemyTarget findEnemy();

		const Player& getPlayer() const;
		const std::vector<Item>& getItems() const;
		const std::vector<Enemy>& getEnemies() const;
		const std::vector<Bullet>& getBullets() const;
		const std::vector<Laser>& getLasers() const;

	private:
		Player m_player;
		std::vector<Item> m_items;
		std::vector<Enemy> m_enemies;
		std::vector<Bullet> m_bullets;
		std::vector<Laser> m_lasers;

		int64_t m_findItemTime;
	};
}
