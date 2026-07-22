#include "enum_bitmask.hpp"

#pragma once

enum class MouseClickState : DWORD {
	LDown = 0x1,
	MDown = 0x2,
	RDown = 0x4,
	Down = 0x8, // 有键按下
	LUp = 0x10,
	MUp = 0x20,
	RUp = 0x40,
	Move = 0x80,
	LDB = 0x100,
	MDB = 0x200,
	RDB = 0x400,
	LClick = 0x800, // 鼠标左键点击
	MClick = 0x1000,
	RClick = 0x2000,
	X1Down = 0x4000,
	X2Down = 0x8000,
	X1Up = 0x10000,
	X2Up = 0x20000,
	X1DB = 0x40000,
	X2DB = 0x80000,
	X1Click = 0x100000,
	X2Click = 0x200000
};
ENABLE_BITMASK_OPERATORS(MouseClickState)