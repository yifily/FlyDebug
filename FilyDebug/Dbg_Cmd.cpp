#include "pch.h"
#include "Dbg_Cmd.h"
#include <tchar.h>




Dbg_Cmd::Dbg_Cmd(TCHAR *Name)
{
	_stprintf_s(m_Name,100, _T("%s"), Name);
}

Dbg_Cmd::~Dbg_Cmd()
{
}


bool Dbg_Cmd::Run(TCHAR * szCmd, Dbg_Context * Conetxt)
{
	//基础命令
	TCHAR szCmdLine[250];

	// 解析命令
	_stscanf_s(szCmd, _T("%s"), szCmdLine,250);

	// 基础命令解析
	// 单步
	if (_tcsicmp(szCmdLine, _T("t")) == 0)
	{
		Dbg_SetTF(Conetxt);
		Conetxt->IsInputFlags = FALSE;
		// 单步停止标志
		Conetxt->SingleStep = TRUE;
		return true;
	}
	// 运行命令
	else if (_tcsicmp(szCmdLine, _T("g")) == 0)
	{
		Conetxt->IsInputFlags = FALSE;
		return true;
	}
	// 断点
	else if (_tcsicmp(szCmdLine, _T("bp")) == 0)
	{
		Conetxt->IsInputFlags = TRUE;
		// 提取地址
		DWORD addr;
		_tscanf_s(_T("%X"),&addr);
		Dbg_SetBp((LPVOID)addr, true, Conetxt);
		return true;
	}
	// api地址
	else if (_tcsicmp(szCmdLine, _T("api")) == 0) {
		Conetxt->IsInputFlags = TRUE;
			DWORD addr;
			TCHAR szApiName[250];
			_tscanf_s(_T("%s"),szApiName,250);
			addr = FindApiAddress(Conetxt->CurPsHandle, szApiName);
			_tprintf(_T("%s : [0x%08x]"), szApiName, addr);
			return true;
	}

	else {
		// 都没有处理
		return Handler(szCmd,Conetxt);
	}

	return false;
}

bool Dbg_Cmd::Handler(TCHAR * szCmd,Dbg_Context * Conetxt)
{
	return true;
}



BOOL Dbg_Cmd::Dbg_SetBp(LPVOID lpAddr, BOOL IsForever, Dbg_Context * Conetxt)
{
	// 构建一个软件断点
	Dbg_BreakPoint bp;
	bp.isForever = IsForever;
	bp.lpAddr = lpAddr;

	//类型软件断点
	bp.Type = Dbg_BreakPoint::SOFTWARE;

	//检查是否有重复断点
	for (auto bps : Conetxt->m_BreakList)
	{
		if (bps.lpAddr == lpAddr)
		{
			return FALSE;
		}
	}

	// 修改目标进程地址
	DWORD OldProtect;
	DWORD Size;
	VirtualProtectEx(Conetxt->CurPsHandle, lpAddr, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	ReadProcessMemory(Conetxt->CurPsHandle, lpAddr, bp.buff, 1, &Size);
	WriteProcessMemory(Conetxt->CurPsHandle, lpAddr, "\xcc", 1, &Size);
	VirtualProtectEx(Conetxt->CurPsHandle, lpAddr, 1, OldProtect, &OldProtect);

	//加入列表
	Conetxt->m_BreakList.push_back(bp);
	return TRUE;
}

BOOL Dbg_Cmd::Dbg_SetTF(Dbg_Context * Conetxt)
{
	// 打开线程句柄
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, Conetxt->dwTid);

	CONTEXT ct = { CONTEXT_CONTROL };
	// 获取线程上下文
	GetThreadContext(hThread, &ct);
	// 设置单步
	ct.EFlags |= 0x100;
	SetThreadContext(hThread, &ct);

	//关闭句柄
	CloseHandle(hThread);
	return TRUE;
}

BOOL Dbg_Cmd::Dbg_FixBp(LPVOID lpAddr, Dbg_Context * Conetxt)
{
	// 遍历断点
	for (DWORD i = 0; i < Conetxt->m_BreakList.size(); i++)
	{
		// 如果是软件断点，进行处理  通过地址找到的相应的断点位置
		if (Conetxt->m_BreakList[i].Type == Dbg_BreakPoint::SOFTWARE
			&& Conetxt->m_BreakList[i].lpAddr == lpAddr)
		{
			// 恢复原始字节
			DWORD OldProtect;
			DWORD Size;
			VirtualProtectEx(Conetxt->CurPsHandle, lpAddr, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
			WriteProcessMemory(Conetxt->CurPsHandle, lpAddr, Conetxt->m_BreakList[i].buff, 1, &Size);
			VirtualProtectEx(Conetxt->CurPsHandle, lpAddr, 1, OldProtect, &OldProtect);

			// 同时eip -1 
			// 打开线程句柄
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, Conetxt->dwTid);
			CONTEXT ct = { CONTEXT_FULL };
			// 获取线程上下文
			GetThreadContext(hThread, &ct);
			ct.Eip -= 1;
			SetThreadContext(hThread, &ct);
			//关闭句柄
			CloseHandle(hThread);

			// 设置接受输入标志
			Conetxt->IsInputFlags = TRUE;

			//断点是否永久性
			if (Conetxt->m_BreakList[i].isForever)
			{
				// 设置单步
				Dbg_SetTF(Conetxt);
				// 保存 单步后恢复cc时的下标
				Conetxt->FixBreakIndex = i;
				// 单步类型
				Conetxt->TFFixFlags = Dbg_BreakPoint::SOFTWARE;
			}
			else
			{
				// 删除这个断点
				Conetxt->m_BreakList.erase(Conetxt->m_BreakList.begin() + i);
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL Dbg_Cmd::Dbg_EnableBp(DWORD i, Dbg_Context * Conetxt)
{
	// 操作失败
	if (i<0 || i>Conetxt->m_BreakList.size())
	{
		return FALSE;
	}

	// 修改目标进程地址
	DWORD OldProtect;
	DWORD Size;
	VirtualProtectEx(Conetxt->CurPsHandle, Conetxt->m_BreakList[i].lpAddr, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	WriteProcessMemory(Conetxt->CurPsHandle, Conetxt->m_BreakList[i].lpAddr, "\xcc", 1, &Size);
	VirtualProtectEx(Conetxt->CurPsHandle, Conetxt->m_BreakList[i].lpAddr, 1, OldProtect, &OldProtect);

	return TRUE;
}


BOOL Dbg_Cmd::Dbg_SetEip(Dbg_Context * Conetxt,DWORD Eip) {

	// 打开线程句柄
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, Conetxt->dwTid);

	CONTEXT ct = { CONTEXT_ALL };
	// 获取线程上下文
	GetThreadContext(hThread, &ct);
	// 设置Eip
	ct.Eip = Eip;
	SetThreadContext(hThread, &ct);

	//关闭句柄
	CloseHandle(hThread);
	return TRUE;

}



BOOL Dbg_Cmd::GetFunctionName(HANDLE hProcess, SIZE_T nAddress, const TCHAR  *buff)
{
	DWORD64  dwDisplacement = 0;
	char buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	if (!SymFromAddrW(hProcess, nAddress, &dwDisplacement, pSymbol))
		return FALSE;
	memcpy(buffer, pSymbol->Name, sizeof(pSymbol->Name));
	return TRUE;

}

// 通过名字获取地址
SIZE_T Dbg_Cmd::FindApiAddress(HANDLE hProcess, const TCHAR * pszName)
{
	//MultiByteToWideChar(CP_ACP, NULL, pszName, 50, buff, 50);
	DWORD64  dwDisplacement = 0;
	TCHAR buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	if (!SymFromNameW(hProcess, pszName, pSymbol))
	{
		return 0;
	}
	return (SIZE_T)pSymbol->Address;
}