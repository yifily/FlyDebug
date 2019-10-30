#pragma once
#include<Windows.h>

class Dbg_BreakPoint
{
public:
	Dbg_BreakPoint();
	~Dbg_BreakPoint();

	// 断点类型
	typedef enum {
		SOFTWARE = 1,	// 软件断点
		HARD,			// 硬件断点
		MEMORY,			// 内存断点
		CONDITION,		// 条件断点
	} BREAK_POINT_TYPE;
	BREAK_POINT_TYPE Type;

	// 地址
	LPVOID lpAddr;

	// 长度
	DWORD Length;

	// 是否永久断点
	BOOL isForever;

	// 需要保存的数据 软件断点使用
	BYTE buff[10];


};

