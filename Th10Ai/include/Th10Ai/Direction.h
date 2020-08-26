#pragma once

#include "Th10Ai/Common.h"

#include <Base/Enum.h>

namespace th
{
	// 移动方向
	enum class DIR : uint_t
	{
		HOLD,			// 不动
		LEFT,			// 左
		RIGHT,			// 右
		UP,				// 上
		DOWN,			// 下
		LEFTUP,			// 左上
		RIGHTUP,		// 右上
		LEFTDOWN,		// 左下
		RIGHTDOWN		// 右下
	};

	constexpr DIR DIR_ENTRIES[] =
	{
		DIR::HOLD,
		DIR::LEFT,
		DIR::RIGHT,
		DIR::UP,
		DIR::DOWN,
		DIR::LEFTUP,
		DIR::RIGHTUP,
		DIR::LEFTDOWN,
		DIR::RIGHTDOWN
	};

	constexpr uint_t DIR_MAXCOUNT = sizeof(DIR_ENTRIES) / sizeof(DIR_ENTRIES[0]);
}
