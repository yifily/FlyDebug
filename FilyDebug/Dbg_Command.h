#pragma once
#include <Windows.h>
#include "Dbg_Context.h"
#include "Dbg_Cmd.h"
#include <vector>
using std::vector;

class Dbg_Command
{
public:
	Dbg_Command();
	~Dbg_Command();

// ÃüÁî´¦Àí

	// Ìí¼ÓÃüÁî
	bool Dbg_AddCmd(Dbg_Cmd* pCmd);

	// Ö´ÐÐÃüÁî 
	bool Dbg_RunCmd(TCHAR * szCmd, Dbg_Context * Conetxt);

	// É¾³ýÃüÁî
	bool Dbg_DelCmd(TCHAR * szCmdName);

	// ÃüÁîÁÐ±í
	vector<Dbg_Cmd*> m_CmdList;

};

