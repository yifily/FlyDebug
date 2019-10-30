#pragma once
#include <Windows.h>
#include "Dbg_BreakPoint.h"
#include <vector>
using std::vector;


class Dbg_Context
{
public:
	Dbg_Context();
	~Dbg_Context();

	// 异常地址
	LPVOID ExceptionAddr =0;
	// 异常类型
	DWORD DebugType =0;
	// 异常代码
	DWORD ExceptionCode = 0;
	// 处理异常状态
	DWORD dwContinueStaus = DBG_CONTINUE;

	// 断点恢复标志
	Dbg_BreakPoint::BREAK_POINT_TYPE TFFixFlags;
	// 单步停止标志
	BOOL SingleStep = FALSE;


	// 断点位置 单步执行后 恢复断点的下标
	int FixBreakIndex =-1;

	// 输入标志
	BOOL IsInputFlags = TRUE;
	// 目标进程PID
	DWORD dwPid = 0 ;
	// 目标进程TID
	DWORD dwTid = 0;
	// 目标进程全局句柄
	HANDLE  CurPsHandle = NULL;
	HANDLE  CurTsHandle = NULL;
	// 系统断点
	BOOL IsSystemBp = TRUE;
	// 程序入口点
	LPVOID EntryPointAddr = 0;

	// 插件列表
	PVOID  PluginList = nullptr;
	// 模块列表
	PVOID  ModlueList= nullptr;
	// 线程列表
	PVOID  ThredList = nullptr;

	// 管理断点列表
	vector<Dbg_BreakPoint> m_BreakList;

};

