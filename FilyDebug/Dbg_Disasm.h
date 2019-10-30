#pragma once
#include <Windows.h>
#include "Capstone/include/capstone.h"
#pragma comment(lib,"capstone/capstone.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"libcmtd.lib\"")

class Dbg_Disasm {
public:

	Dbg_Disasm();
	~Dbg_Disasm();

public:
	// 用于初始化和内存管理的句柄
	static csh Handle;
	static cs_opt_mem OptMem;

	// 用于初始化的函数
	static void Init();

	// 用于执行反汇编的函数
	static void DisAsm(HANDLE Handle, LPVOID Addr, DWORD Count);

};