#include "pch.h"
#include "Dbg_Command.h"
#include <tchar.h>




Dbg_Command::Dbg_Command()
{
}


Dbg_Command::~Dbg_Command()
{
	if (!m_CmdList.empty())
	{
		for (DWORD i=0;i<m_CmdList.size();i++)
		{
			delete m_CmdList[i];
		}
		m_CmdList.clear();
	}
}



bool Dbg_Command::Dbg_AddCmd(Dbg_Cmd * pCmd)
{
	if (pCmd)
	{
		// 加入命令
		m_CmdList.push_back(pCmd);
	}
	return true;
}

bool Dbg_Command::Dbg_RunCmd(TCHAR * szCmd,Dbg_Context * Conetxt)
{
	// 遍历命令列表
	for (auto cmd : m_CmdList)
	{
		// 处理命令
		if (cmd->Run(szCmd, Conetxt))
		{
			// 处理成功
			return true;
		}
	}
	// 没有处理
	return false;
}

bool Dbg_Command::Dbg_DelCmd(TCHAR * szCmdName)
{
	// 遍历命令列表
	for (DWORD i=0;i<m_CmdList.size();i++)
	{	
		if (_tcsicmp(szCmdName, m_CmdList[i]->m_Name) == 0)
		{
			// 删除命令
			m_CmdList.erase(m_CmdList.begin()+i);
			return true;
		}
	}

	return false;
}
