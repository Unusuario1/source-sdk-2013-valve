#ifndef EXTERNALCONSOLESNDMSG_HPP
#define EXTERNALCONSOLESNDMSG_HPP
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include <windows.h>
#include "tier0/icommandline.h"
#include "tier0/dbg.h"
#include "tier1/convar.h"
#include "utlbuffer.h"
#include "tier1/KeyValues.h"
#include "vgui_controls/consoledialog.h" 
#include "filesystem.h" 
#include "common/colorschemetools.h"
#include "common/pipeline_shareddefs.h"


// TODO: Move this to the game/shared dir
//	Also to make it rembender 	g_GameConsole.m_pConsole->DumpConsoleTextToFile();
// and the condump convar

#define EXTERNAL_CONSOLE_SETTINGS	"scripts/tools/xtrconsole_settings.txt"
#define EXTERNAL_CONSOLE_VERSION	1.0f


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CExternalConsoleMgr
{
private:
	bool m_bEnableExternalConsole = false;

private:
	inline void LaunchExternalConsole();
	inline void CreateKernelPipeline();
	inline const char* CreateDefaultConfiguration();

public:
	inline CExternalConsoleMgr();
	inline ~CExternalConsoleMgr();

	inline void Init();
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline void CExternalConsoleMgr::Init()
{
	this->CreateKernelPipeline();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline CExternalConsoleMgr::CExternalConsoleMgr()
{
	// check if we are in the right mode.
	const char* rgpCmmd[] = { "-dev", "-tools", "-allowdebug", "-makedevshots" };
	for (const char* Cmmd : rgpCmmd)
	{
		if (CommandLine()->FindParm(Cmmd))
		{
			this->m_bEnableExternalConsole = true;
			return;
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
// Purpose:	Create the default configuration or load the existing one.
//-----------------------------------------------------------------------------
inline const char* CExternalConsoleMgr::CreateDefaultConfiguration()
{
	if (g_pFullFileSystem->FileExists(EXTERNAL_CONSOLE_SETTINGS, "MOD"))
	{
		KeyValues* pKvCfg = new KeyValues("");
		pKvCfg->LoadFromFile(g_pFullFileSystem, EXTERNAL_CONSOLE_SETTINGS, "MOD", true);
	}
	else
	{
		// Create the file.
		KeyValues* pKvCfg = new KeyValues("ExternalConsole");
		pKvCfg->CreateKey("Version");
		pKvCfg->SetInt("Version", EXTERNAL_CONSOLE_VERSION);

		KeyValues* pKvSubCfg = new KeyValues("Configuration");
		pKvSubCfg->CreateKey("KernelPipeLineName");
		pKvSubCfg->SetString("KernelPipeLineName", "generic_game");
		pKvCfg->AddSubKey(pKvSubCfg);
		pKvCfg->SaveToFile(g_pFullFileSystem, EXTERNAL_CONSOLE_SETTINGS);

		return "generic_game";
	}
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
	V_sprintf_safe(szPipeLineName, "\\\\.\\pipe\\%s", this->CreateDefaultConfiguration());
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
	char szExternalConsolePath[MAX_PATH];
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



CON_COMMAND_F(connect_external_console, "Connect to the external console.", FCVAR_NONE)
{
	if (!g_ExternalConsoleMgr)
		g_ExternalConsoleMgr = new CExternalConsoleMgr();

	g_ExternalConsoleMgr->Init();
}

#endif // EXTERNALCONSOLESNDMSG_HPP