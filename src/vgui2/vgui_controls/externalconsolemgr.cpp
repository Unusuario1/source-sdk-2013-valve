#ifndef EXTERNALCONSOLESNDMSG_HPP
#define EXTERNALCONSOLESNDMSG_HPP
#ifdef _WIN32
#pragma once
#endif // _WIN32


#include <windows.h>
#include "tier0/icommandline.h"
#include "tier0/dbg.h"
#include "../../public/vgui_controls/consoledialog.h" // TODO: Fix the path??
#include "../../public/filesystem.h" // TODO: Fix the path??
#include "../../utils/common/colorschemetools.h"
#include "../../utils/common/pipeline_shareddefs.h"

class CConsolePanel;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CExternalConsoleMgr : CConsolePanel
{
private:
	bool m_bEnableExternalConsole = false;


private:
	inline void LaunchExternalConsole();
	inline void CreateKernelPipeline();

public:
	inline CExternalConsoleMgr(const char* pszKernelPipelineName);
	inline ~CExternalConsoleMgr();
};



//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline CExternalConsoleMgr::CExternalConsoleMgr(const char* pszKernelPipelineName)
{
	// check if we are in the right mode.
	const char* rgpCmmd[] = { "-dev", "-tools", "-allowdebug", "-makedevshots" };
	for (const char* Cmmd : rgpCmmd)
	{
		if (CommandLine()->FindParm(Cmmd))
		{
			this->m_bEnableExternalConsole = true;
			break;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline CExternalConsoleMgr::~CExternalConsoleMgr()
{
	// Do nothing....
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline void CExternalConsoleMgr::CreateKernelPipeline()
{
	if (!this->m_bEnableExternalConsole)
		return;

	DevMsg("[ExternalConsole] Creating kernel pipeline...");
	char szPipeLineName[MAX_PATH];
	V_sprintf_safe(szPipeLineName, "\\\\.\\pipe\\");
	HANDLE hPipe = CreateNamedPipeA(
		szPipeLineName,
		PIPE_ACCESS_OUTBOUND,               // server writes
		PIPE_TYPE_BYTE | PIPE_WAIT,         // byte stream, blocking
		1,                                  // max instances
		8192,                               // out buffer
		8192,                               // in buffer
		0,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		DevWarning("[ExternalConsole] Failed to Create namedpipeline!\n");
		return;
	}

	DevMsg("[ExternalConsole] Waiting for %s to connect...\n", EXTERNAL_CONSOLE);
	ConnectNamedPipe(hPipe, NULL);
	DevMsg("[ExternalConsole] done\n");

	const char* pszMsg = "Game connected to" EXTERNAL_CONSOLE;
	DWORD written;

	WriteFile(hPipe, pszMsg, V_strlen(pszMsg), &written, NULL);

	CloseHandle(hPipe);
	DevMsg("[ExternalConsole] done\n");
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CExternalConsoleMgr::LaunchExternalConsole()
{
	char* szExternalConsolePath[MAX_PATH];
	g_pFullFileSystem->GetSearchPath_safe("BIN", false, szExternalConsolePath);
	V_sprintf_safe(szExternalConsolePath, "%s\\%s", szExternalConsolePath, EXTERNAL_CONSOLE);

	STARTUPINFOA si{};
	PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);

	bool bOk = CreateProcessA(szExternalConsolePath, NULL, NULL, NULL, FALSE, DETACHED_PROCESS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	if (bOk)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
}


CExternalConsoleMgr* g_ExternalConsoleMgr = nullptr;


#endif // EXTERNALCONSOLESNDMSG_HPP