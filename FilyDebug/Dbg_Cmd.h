#pragma once
#include "Dbg_Context.h"
#include "dbghelp.h"
#pragma comment(lib,"Dbghelp.lib")

class Dbg_Cmd
{
public:
	Dbg_Cmd(TCHAR *Name);
	~Dbg_Cmd();

	// 通用命令
	// 设置软件断点
	static BOOL Dbg_SetBp(LPVOID lpAddr, BOOL IsForever, Dbg_Context * Conetxt);

	// 设置单步断点
	static BOOL Dbg_SetTF(Dbg_Context * Conetxt);

	// 恢复软件断点
	static BOOL Dbg_FixBp(LPVOID lpAddr, Dbg_Context * Conetxt);

	// 启用软件断点
	static BOOL Dbg_EnableBp(DWORD i, Dbg_Context * Conetxt);

	// 设置EIP
	static BOOL Dbg_SetEip(Dbg_Context * Conetxt, DWORD Eip);

	// 获取函数名，通过地址
	static BOOL GetFunctionName(HANDLE hProcess, SIZE_T nAddress, const TCHAR * pBuff);

	// 通过名字获取地址
	static SIZE_T FindApiAddress(HANDLE hProcess, const TCHAR * pszName);



// 命令处理 一次
	virtual bool Run(TCHAR * szCmd, Dbg_Context * Conetxt);

// 命令处理 二次
	virtual bool Handler(TCHAR * szCmd,Dbg_Context * Conetxt);

// 命令名字，删除时用
	TCHAR m_Name[100];
};

