#pragma once

namespace th
{
	class Player;
	class Item;
	class Enemy;
	class Bullet;
	class Laser;

	class Entity
	{
	public:
		Entity() :
			x(), y(), width(), height(), dx(), dy() {}
		Entity(float_t x0, float_t y0, float_t width0, float_t height0, float_t dx0, float_t dy0) :
			x(x0), y(y0), width(width0), height(height0), dx(dx0), dy(dy0) {}

		float_t getDistance(const Pointf& other) const;
		float_t getDistance(const Entity& other) const;
		float_t getAngle(const Entity& other) const;
		bool hitTest(const Entity& other, float_t epsilon = 0.0) const;

		Pointf getCenter() const;
		Pointf getLeftTop() const;
		Pointf getNextPos() const;

		float_t x;
		float_t y;
		float_t width;
		float_t height;
		float_t dx;
		float_t dy;
	};

	class Player :
		public Entity
	{
	public:
		Player() {};
		Player(float_t x0, float_t y0, float_t width0, float_t height0, float_t dx0, float_t dy0) :
			Entity(x0, y0, width0, height0, dx0, dy0) {}

		bool hitTestSAT(const Laser& laser, float_t epsilon = 0.0) const;

		float_t powers;

		int_t life;

		// type == 0 Reimu
		// type == 1 Marisa
		int_t type;

		// slow == 0 High Speed
		// slow == 1 Low Speed
		int_t slow;

		float_t itemObtainRange;

		// status == 0 ����״̬����ʱ�޵�
		// status == 1 ����״̬
		// status == 2 ����
		// status == 3 Unknown
		// status == 4 ��������������ʱ��B���Ծ���
		int_t status;

		// ʣ���޵�ʱ��
		int_t invinibleTime;
	};

	class Item :
		public Entity
	{
	public:
		Item() {};
		Item(float_t x0, float_t y0, float_t width0, float_t height0, float_t dx0, float_t dy0) :
			Entity(x0, y0, width0, height0, dx0, dy0) {}
	};

	class Enemy :
		public Entity
	{
	public:
		Enemy() {};
		Enemy(float_t x0, float_t y0, float_t width0, float_t height0, float_t dx0, float_t dy0) :
			Entity(x0, y0, width0, height0, dx0, dy0) {}
	};

	class Bullet :
		public Entity
	{
	public:
		Bullet() {};
		Bullet(float_t x0, float_t y0, float_t width0, float_t height0, float_t dx0, float_t dy0) :
			Entity(x0, y0, width0, height0, dx0, dy0) {}

		Bullet advance() const;
	};

	//       w
	// +---(x,y)---+
	// |     |     |
	// |     |     |
	// |     |     |
	// |     |     | h
	// |     |     |
	// |     |     |
	// |     |     |
	// +-----+-----+
	class Laser :
		public Entity
	{
	public:
		Laser() :
			arc() {}
		Laser(float_t x0, float_t y0, float_t width0, float_t height0, float_t dx0, float_t dy0, float_t arc0) :
			Entity(x0, y0, width0, height0, dx0, dy0), arc(arc0) {}

		float_t arc;
	};

	class SATBox
	{
	public:
		static void PointRotate(float_t& x, float_t& y, float_t cx, float_t cy, float_t arc);
		static bool HitTest(float_t c1, float_t d1, float_t c2, float_t d2, float_t epsilon);

		//SATBox() :
		//	x1(), y1(),
		//	x2(), y2(),
		//	x3(), y3(),
		//	x4(), y4() {}

		float_t x1, y1;
		float_t x2, y2;
		float_t x3, y3;
		float_t x4, y4;
	};

	struct LaserBox :
		SATBox
	{
		//LaserBox() = default;
		LaserBox(const Laser& laser);

		bool hitTestSAT(const Player& player, float_t epsilon = 0.0) const;
	};

	struct PlayerBox :
		SATBox
	{
		//PlayerBox() = default;
		PlayerBox(const Player& player, const Laser& laser);

		bool hitTestSAT(const Laser& laser, float_t epsilon = 0.0) const;
	};
}