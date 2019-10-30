#pragma once
#include <Windows.h>

#include "Dbg_BreakPoint.h"
#include "Dbg_Command.h"
#include "Dbg_Plugin.h"
#include "Dbg_Context.h"


class Dbg_Engine
{
public:
	Dbg_Engine();
	~Dbg_Engine();

	// 调试器以创建进程方式启动
	bool Dbg_CreateDebug(TCHAR * szPath);

	// 调试器以附加方式启动
	bool Dbg_ActiveDebug(DWORD dwPid);

	// 调试循环
	bool Dbg_Run();

	// 调试事件处理
	DWORD Dbg_DispatchDebugEvent(DEBUG_EVENT * dbg_event);

	// 异常事件处理
	DWORD Dbg_DispatchExcetpion(EXCEPTION_DEBUG_INFO * pException_Info);

	// 软件异常处理
	DWORD Dbg_OnSoftwareExceptionHandler(EXCEPTION_RECORD * pException_recode);

	// 单步（硬件）异常处理
	DWORD Dbg_OnSingleStepExceptionHandler(EXCEPTION_RECORD * pException_recode);

	// 访问异常处理
	DWORD Dbg_OnAccessExceptionHander(EXCEPTION_RECORD * pException_recode);



public :
	// 目标程序路径
	TCHAR m_szPaht[MAX_PATH];

	// 调试方法枚举方式
	typedef enum DEBUG_METHOD {
		DBG_CREATE,		// 创建方式
		DBG_ACTIVE		// 附加方式
	}DEBUG_METHOD;
	// 目标调试方式
	DEBUG_METHOD m_Method;

	// 当前调试器运行中的信息，与各个组件沟通
	Dbg_Context *m_context;

	// 命令解析器
	Dbg_Command *m_Command; //命令  命令回调函数回调函数原型("cmd",context);



	// 模块列表

	// 线程列表


};

