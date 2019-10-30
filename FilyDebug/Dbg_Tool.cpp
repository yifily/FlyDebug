#include "pch.h"
#include "Dbg_Tool.h"
#include <Windows.h>



Dbg_Tool::Dbg_Tool()
{
}


Dbg_Tool::~Dbg_Tool()
{
}

BOOL Dbg_Tool::inject(HANDLE hProcess, char * shellcode, unsigned int code_size)
{
	SIZE_T Count = 0;
	LPVOID pMem = VirtualAllocEx(hProcess, NULL, code_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, pMem, shellcode, code_size, &Count);
	HANDLE hTread = CreateRemoteThread(
		hProcess,
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)pMem,
		NULL,
		NULL,
		NULL
	);
	if (hTread == INVALID_HANDLE_VALUE || hTread == NULL)
	{
		return FALSE;
	}

	WaitForSingleObject(hTread, -1);
	CloseHandle(hTread);
	CloseHandle(hProcess);

	return TRUE;

	return 0;
}




//void _declspec(naked) shellcode()
//{
//	_asm {
//		pushad;				     // 保存环境
//		mov eax, 0xFFFFFFFF;     // LoadLibrary地址
//
//		call Next;				// 获取Eip
//	Next:
//		pop esi;
//		add esi, 0xF;			// 获取DLL路径
//
//		push esi;				// dll路径名
//		call eax;				// 调用LoadLibrary
//
//		popad;					// 恢复环境
//
//
//		mov eax, 0xFFFFFFFF;	// 保存返回主程序的地址
//		push eax;
//		ret;					// 跳转回主程序
//
//		// dll路径字符串
//		_asm __emit(0x11);		_asm __emit(0x11);         _asm __emit(0x11);       _asm __emit(0x11);
//		_asm __emit(0x11);      _asm __emit(0x11);		   _asm __emit(0x11);		_asm __emit(0x11);
//	}
//}
//

DWORD Dbg_Tool::inject1(HANDLE hprocess,DWORD Eip,char *dllPath) {

	DWORD dwSize;
	DWORD dwAddr;

	// 1. 目标进程申请空间
	char * Target = (char*)VirtualAllocEx(hprocess,0, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// 2.构建shellcode 
	const char *shellcode = "\x60\xB8\xFF\xFF\xFF\xFF\xE8\x00\x00\x00\x00\x5E\x83\xC6\x0F\x56\xFF\xD0\x61\xB8\xFF\xFF\xFF\xFF\x50\xC3\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11";

	// 2.1 拷贝shellcode
	WriteProcessMemory(hprocess,Target, shellcode, 0x27, &dwSize);

	// 2.2 填充地址
	// 2.2.1 获取LoadLibraryA函数地址
	HMODULE hmod = LoadLibraryA("kernel32.dll");
	dwAddr = (DWORD)GetProcAddress(hmod, "LoadLibraryA");

	// 填充LoadLibraryA函数地址
	WriteProcessMemory(hprocess, Target + 2, &dwAddr, 4, &dwSize);

	// 填充dll路径
	WriteProcessMemory(hprocess,Target + 26,dllPath,strlen(dllPath)+1,&dwSize);

	// 跳转回正常地址  eip
	WriteProcessMemory(hprocess, Target + 20, &Eip, 4, &dwSize);

	// 目标进程空间地址
	return (DWORD)Target;

}