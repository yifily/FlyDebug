#pragma once
#include <Windows.h>
#include "Dbg_Context.h"

class Dbg_Tool
{
public:
	Dbg_Tool();
	~Dbg_Tool();

	BOOL inject(HANDLE hProces, char *shellcode, unsigned int code_size);

	// 返回目标进程申请的空间,  需要修改eip指向我们返回的地址
	DWORD inject1(HANDLE hprocess, DWORD Eip, char *dllPath);

};

