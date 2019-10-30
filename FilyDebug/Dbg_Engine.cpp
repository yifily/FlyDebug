#include "pch.h"
#include <tchar.h>
#include<cstdio>
#include "Dbg_Engine.h"
#include "Dbg_Disasm.h"
#include "Dbg_Tool.h"
#include "Dbg_Cmd.h"



//1. 包含头文件
#ifdef _WIN64 // 64位平台编译器会自动定义这个宏
#pragma 
comment(lib, "capstone/lib/capstone_x64.lib")
#else
#pragma comment(lib,"capstone/capstone.lib")
#endif // _64



Dbg_Engine::Dbg_Engine()
{
	m_context = new Dbg_Context();
	m_Command = new Dbg_Command();
	// 初始化 反汇编器
	Dbg_Disasm::Init();

	Dbg_Cmd* cmd = new Dbg_Cmd((TCHAR*)_T("default"));
	// 添加默认命令处理
	m_Command->Dbg_AddCmd(cmd);

}

Dbg_Engine::~Dbg_Engine()
{
	if (m_context)
	{
		delete m_context;
		m_context = nullptr;
	}
}

bool Dbg_Engine::Dbg_CreateDebug(TCHAR * szPath)
{
	// 保存进程路径
	lstrcpy(m_szPaht, szPath);

	// 如果进程创建成功，用于接收进程线程的句柄和id
	PROCESS_INFORMATION process_info = { 0 };
	STARTUPINFO startup_info = { sizeof(STARTUPINFO) };

	// 创建进程
	BOOL result = CreateProcess(
		szPath,
		nullptr, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL, NULL, &startup_info, &process_info);

	// 如果进程创建成功了，就关闭对应的句柄，防止句柄泄露
	if (result == TRUE)
	{
		// 备份句柄，当程序关闭时再进行释放
		m_context->CurPsHandle = process_info.hProcess;
		m_context->CurTsHandle  = process_info.hThread;
	}




	return result;
}

bool Dbg_Engine::Dbg_ActiveDebug(DWORD dwPid)
{
	m_Method = DBG_ACTIVE;			// 保存调试方式

	// 保存pid
	m_context->dwPid = dwPid;

	// 附加进程
	BOOL result = DebugActiveProcess(dwPid);

	// 返回结果
	return result;
}

bool Dbg_Engine::Dbg_Run()
{
	DWORD dwContinueStaus = DBG_CONTINUE;
	DEBUG_EVENT DbgEvent;



	// 等待调试事件
	while (WaitForDebugEvent(&DbgEvent, -1))
	{
		// 保存进程线程ID
		m_context->dwPid = DbgEvent.dwProcessId;
		m_context->dwTid = DbgEvent.dwThreadId;


		// 分发调试异常信息
		Dbg_DispatchDebugEvent(&DbgEvent);
		
		// 向调试子系统返回当前的处理结果
		ContinueDebugEvent(
			m_context->dwPid,
			m_context->dwTid,
			m_context->dwContinueStaus);
	}

	return TRUE;
}

DWORD Dbg_Engine::Dbg_DispatchDebugEvent(DEBUG_EVENT * dbg_event)
{
	
	// 保存当前调试类型
	m_context->DebugType = dbg_event->dwDebugEventCode;

	switch (dbg_event->dwDebugEventCode)
	{
	case EXCEPTION_DEBUG_EVENT:							// 异常调试事件
		Dbg_DispatchExcetpion(&dbg_event->u.Exception);	// 分发调试异常事件
		break;
	case CREATE_THREAD_DEBUG_EVENT: // 线程创建事件
		printf("CREATE_THREAD_DEBUG_EVENT\n"); break;
	case CREATE_PROCESS_DEBUG_EVENT:// 进程创建事件

		// 保存OEP
		m_context->EntryPointAddr = dbg_event->u.CreateProcessInfo.lpStartAddress;
		printf("CREATE_PROCESS_DEBUG_EVENT\n"); break;
	case EXIT_THREAD_DEBUG_EVENT:   // 退出线程事件
		printf("EXIT_THREAD_DEBUG_EVENT\n"); break;
	case EXIT_PROCESS_DEBUG_EVENT:  // 退出进程事件
		printf("EXIT_PROCESS_DEBUG_EVENT\n"); break;
	case LOAD_DLL_DEBUG_EVENT:      // 映射DLL事件
		printf("LOAD_DLL_DEBUG_EVENT\n"); break;
	case UNLOAD_DLL_DEBUG_EVENT:    // 卸载DLL事件 
		printf("UNLOAD_DLL_DEBUG_EVENT\n"); break;
	case OUTPUT_DEBUG_STRING_EVENT: // 调试输出事件
		printf("OUTPUT_DEBUG_STRING_EVENT\n"); break;
	case RIP_EVENT:                 // RIP事件(内部错误)
		printf("RIP_EVENT\n"); break;
	}
	return TRUE;
}

DWORD Dbg_Engine::Dbg_DispatchExcetpion(EXCEPTION_DEBUG_INFO * pException_Info)
{	
	// 异常处理事件
	m_context->dwContinueStaus = DBG_CONTINUE;

	// 获取异常代码
	m_context->ExceptionCode = pException_Info->ExceptionRecord.ExceptionCode;
	// 异常地址
	m_context->ExceptionAddr = pException_Info->ExceptionRecord.ExceptionAddress;

	
	// 根据代码进行处理
	switch (m_context->ExceptionCode)
	{
		// 设备访问异常: 和内存断点相关
	case EXCEPTION_ACCESS_VIOLATION:
		
		//处理内存断点
			Dbg_OnAccessExceptionHander(&pException_Info->ExceptionRecord);
		break;

		// 断点异常: int 3(0xCC) 会触发的异常
	case EXCEPTION_BREAKPOINT:
	{
		// 处理软件断点
			Dbg_OnSoftwareExceptionHandler(&pException_Info->ExceptionRecord);
		break;
	}

	// 硬件断点事件: TF单步  DrN断点
	case EXCEPTION_SINGLE_STEP:
		   //处理单步断点
			Dbg_OnSingleStepExceptionHandler(&pException_Info->ExceptionRecord);
		break;
	}

	// 是否接受命令输入
	while (m_context->IsInputFlags)
	{
		Dbg_Disasm::DisAsm(m_context->CurPsHandle, m_context->ExceptionAddr, 10);
		TCHAR buff[250];
		// 调用命令处理
		_tscanf_s(_T("%s"), buff, 250);
		m_Command->Dbg_RunCmd(buff, m_context);
	}

	return TRUE;
}

DWORD Dbg_Engine::Dbg_OnSoftwareExceptionHandler(EXCEPTION_RECORD * pException_recode)
{

	// 系统断点
	if (m_context->IsSystemBp)
	{
		m_context->IsSystemBp = FALSE;
		// 第一次系统断点  特殊处理
		Dbg_Cmd::Dbg_SetBp(m_context->EntryPointAddr, FALSE,m_context);
		// 恢复子系统
		m_context->dwContinueStaus = DBG_CONTINUE;
		// 不接受输入
		m_context->IsInputFlags = FALSE;

		// 加载符号
		SymInitialize(m_context->CurPsHandle, NULL, TRUE);

		// 注入反调试
		Dbg_Tool tool;
		DWORD NewEip = tool.inject1(m_context->CurPsHandle, (DWORD)pException_recode->ExceptionAddress + 1, (char*)"C:\\Users\\42140\\Desktop\\HOOKAPI.dll");
		Dbg_Cmd::Dbg_SetEip(m_context, NewEip);

		return TRUE;
	}

	//正常断点 需要修复
	Dbg_Cmd::Dbg_FixBp(pException_recode->ExceptionAddress, m_context);

	return TRUE;
}

DWORD Dbg_Engine::Dbg_OnSingleStepExceptionHandler(EXCEPTION_RECORD * pException_recode)
{
	// 处理单步事件

	// 1.单步类型  判断单步类型  软件恢复单步，硬件恢复单步，内存恢复单步，
	// 2.普通单步

	// 1.1 软件恢复单步
	if (m_context->TFFixFlags == Dbg_BreakPoint::SOFTWARE)
	{
		// 开启软件断点
		Dbg_Cmd::Dbg_EnableBp(m_context->FixBreakIndex, m_context);
		m_context->IsInputFlags = FALSE;
	}
	// 1.2 硬件恢复单步
	else if (m_context->TFFixFlags == Dbg_BreakPoint::HARD)
	{
		// ...
	}
	// 1.3 内存恢复单步
	else if (m_context->TFFixFlags == Dbg_BreakPoint::MEMORY)
	{
		// ...
	}

	//2.1 处理普通单步 "t"命令
	if (m_context->SingleStep)
	{
		// 关闭单步停止
		m_context->SingleStep = FALSE;
		// 命令t需要单步
		m_context->IsInputFlags = TRUE;
	}



	return TRUE;
}

DWORD Dbg_Engine::Dbg_OnAccessExceptionHander(EXCEPTION_RECORD * pException_recode)
{
	return 0;
}
