// FilyDebug.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <stdio.h>
#include "Dbg_Engine.h"

int main()
{
	TCHAR buff[MAX_PATH];
	wscanf_s(L"%s", buff, MAX_PATH);
	Dbg_Engine Dbg;
	Dbg.Dbg_CreateDebug(buff);
	Dbg.Dbg_Run();

}