//========= ------------------------------------------------------ ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "vrad.h"
#include "filesystem_init.h"
#include "KeyValues.h"
#include <windows.h>
#include "cubemapbuilder.h"

/*
To enable CubemapBuilder in VRAD you have to setup gameinfo.txt, lets take for example Half-Life 2 (hl2)
Inside the "GameInfo" KeyValue put this code (Note: Vrad if compiled on 32 bits, vrad will call the 32 bits launcher, same for 64 bits):

	// Name of the game launcher without extension. 
	GameExecutableName32bits hl2
	// Name of the 64 bits game launcher without extension.
	GameExecutableName64bits hl2_win64

	// CubemapBuilder KeyValue controls cubemap building inside the compile tools, it is used to pass commandline/convars to the engine. 
	CubemapBuilder
	{
		// Sets up the game at maximun graphics
		SetGameToMaximunGraphic 1
		//VBSP option: Build the default cubemap
		BuildDefaultCubemap 	1

		// These are mounted in the command line when VRAD initiates the game launcher.  
		// They can be either a convar or a commandline argument.  
		// If '-BuildHdrCubemaps' is enabled, VRAD will load these commandline arguments/convars.
		Hdr
		{
			BuildParams		"+sv_cheats 1 +mat_specular 0 +mat_hdr_level 2 +building_cubemaps 1"
		}

		// If '-BuildLdrCubemaps' is enabled, VRAD will load these commandline arguments/convars
		Ldr
		{
			BuildParams		"+sv_cheats 1 +mat_specular 0 +mat_hdr_level 0"
		}
	}
*/


//-----------------------------------------------------------------------------
// Purpose: Defines CubemapBuilder KeyValues. (NOTE: sync this with VBSP!)
//-----------------------------------------------------------------------------
#define CUBEMAPBUILDER_KV				"CubemapBuilder"
#define CUBEMAPBUILDER_KV_MAX_GRAPHICS	"SetGameToMaximunGraphic"
#define CUBEMAPBUILDER_KV_32BITS_EXE	"GameExecutableName32bits"
#define CUBEMAPBUILDER_KV_64BITS_EXE	"GameExecutableName32bits"
#define	CUBEMAPBUILDER_KV_HDR			"Hdr"
#define	CUBEMAPBUILDER_KV_LDR			"Ldr"
#define CUBEMAPBUILDER_KV_PARAMSTRING	"BuildParams"
#define CUBEMAPBUILDER_MAX_GRAPH_CVAR	"+r_lightmap_bicubic 1 +r_waterforceexpensive 1 +mat_antialias 8 +mat_picmip -10 +mat_forceaniso 16"


//-----------------------------------------------------------------------------
// Purpose: Copies the BSP file to the game directory (e.g., path/foo.bsp -> game/maps/foo.bsp).
//-----------------------------------------------------------------------------
static void CopyBspToGameDir()
{
	char szMapDir[MAX_PATH], szMapGameDir[MAX_PATH], szMapGamedirFile[MAX_PATH], szTempGameDir[MAX_PATH];

	float start = Plat_FloatTime();

	V_strcpy(szTempGameDir, gamedir);
	V_StripTrailingSlash(szTempGameDir);

	V_snprintf(szMapDir, sizeof(szMapDir), "%s", source);
	V_snprintf(szMapGameDir, sizeof(szMapGameDir), "%s\\maps", szTempGameDir);
	V_snprintf(szMapGamedirFile, sizeof(szMapGamedirFile), "%s\\%s.bsp", szMapGameDir, level_name);

	Msg("Map original directory: %s\n", szMapDir);
	Msg("Map game folder:	%s\n", szMapGameDir);
	Msg("Map game directory: %s\n", szMapGamedirFile);
	Msg("\n");
	Msg("Copying source %s.bsp to game directory... ", level_name);

	// Ensure the "maps" directory exists before copying
	CreateDirectory(szMapGameDir, NULL);

	// Set attributes to normal in case of restrictions
	SetFileAttributes(szMapGamedirFile, FILE_ATTRIBUTE_NORMAL);
	SetFileAttributes(szMapDir, FILE_ATTRIBUTE_NORMAL);

	if (!CopyFile(szMapDir, szMapGamedirFile, FALSE))
	{
		DWORD error = GetLastError();
		Warning("\n"
				"Could not copy to source directory %s\n"
				"Error CopyFile() : %lu, %s\n",
				szMapGamedirFile, error,
				error == ERROR_ACCESS_DENIED ? "Access denied! Check permissions and file attributes.\n" : ""
		);
		return;
	}
	else
	{
		Msg("done (%.2f)\n", Plat_FloatTime() - start);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Copies the BSP file from the game directory back to the original path
//          (e.g., game/maps/foo.bsp -> path/foo.bsp).
//-----------------------------------------------------------------------------
static void CopyGameDirBspToOrignalBspDir()
{
	char szMapDir[MAX_PATH], szMapGameDir[MAX_PATH], szMapGamedirFile[MAX_PATH], szTempGameDir[MAX_PATH];

	float start = Plat_FloatTime();

	V_strcpy(szTempGameDir, gamedir);
	V_StripTrailingSlash(szTempGameDir);

	V_snprintf(szMapDir, sizeof(szMapDir), "%s", source);
	V_snprintf(szMapGameDir, sizeof(szMapGameDir), "%s\\maps", szTempGameDir);
	V_snprintf(szMapGamedirFile, sizeof(szMapGamedirFile), "%s\\%s.bsp", szMapGameDir, level_name);

	Msg("Copying game %s.bsp to source directory... ", level_name);

	// Ensure the "maps" directory exists before copying
	CreateDirectory(szMapGameDir, NULL);

	// Set attributes to normal in case of restrictions
	SetFileAttributes(szMapGamedirFile, FILE_ATTRIBUTE_NORMAL);
	SetFileAttributes(szMapDir, FILE_ATTRIBUTE_NORMAL);

	if (!CopyFile(szMapGamedirFile, szMapDir, FALSE))
	{
		DWORD error = GetLastError();
		Warning("\n"
				"Could not copy to source directory %s\n"
				"Error CopyFile() : %lu, %s\n",
				szMapGamedirFile, error,
				error == ERROR_ACCESS_DENIED ? "Access denied! Check permissions and file attributes.\n" : ""
		);
		return;
	}
	else
	{
		Msg("done (%.2f)\n", Plat_FloatTime() - start);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Loads ConVars or command-line parameters found in 'CubemapBuilder' 
//          within gameinfo.txt.
//-----------------------------------------------------------------------------
static void LoadGameInfoConvar(const char* pGameInfoPath, char* pConvar, const std::size_t uiConvarBufferSize, const bool bHdrMode) 
{
	float start = Plat_FloatTime();

	Msg("Loading KeyValues from gameinfo.txt... ");

	KeyValues* pKvGameInfoCubemap = ReadKeyValuesFile(pGameInfoPath);
	KeyValues* pKvCubemapBuilder = pKvGameInfoCubemap->FindKey(CUBEMAPBUILDER_KV);
	
	if(!pKvCubemapBuilder)
	{
		Warning("\n"
				"Warning: Could not load KeyValues for %s!\n"
				"Warning: Cubemaps might not look right!\n",
				CUBEMAPBUILDER_KV
		);
		return;
	}
	
	const char* pSetGameToMaximunGraphic = pKvCubemapBuilder->GetString(CUBEMAPBUILDER_KV_MAX_GRAPHICS, "0");
	const bool bSetGameToMaximunGraphic = atoi(pSetGameToMaximunGraphic) == 1 ? true : false;

	KeyValues* pKvLightingMode = pKvCubemapBuilder->FindKey(bHdrMode ? CUBEMAPBUILDER_KV_HDR : CUBEMAPBUILDER_KV_LDR);

	if (!pKvLightingMode)
	{
		Warning("\n"
				"Warning: Could not load KeyValues for %s!\n"
				"Warning: Cubemap compile might not look right!\n"
				,bHdrMode ? "Hdr" : "Ldr"
		);
		return;
	}

	const char* pParam = pKvLightingMode->GetString(CUBEMAPBUILDER_KV_PARAMSTRING);
	V_snprintf(pConvar, uiConvarBufferSize, "%s %s", !pParam || (pParam == "") ? "" : pParam, bSetGameToMaximunGraphic ? CUBEMAPBUILDER_MAX_GRAPH_CVAR : "");

	pKvGameInfoCubemap->deleteThis();

	Msg("done (%.2f)\n", Plat_FloatTime() - start);
}


//-----------------------------------------------------------------------------
// Purpose: Automatically builds cubemaps.
//-----------------------------------------------------------------------------
void BuildCubemaps(const bool bHdrMode)
{
	char	szGameInfoPath[MAX_PATH], szGameExecutableName[MAX_PATH], szTempGameDir[MAX_PATH];
	char	szKVconvars[2048], szGameExecutablePath[8192], szBuildCubemapsCommandLine[8192];

	float start = Plat_FloatTime();

	Msg("\n\n");
	Msg("====== Building cubemaps (%s) ======\n", bHdrMode ? "Hdr" : "Ldr");

	CopyBspToGameDir();
	
	g_pFullFileSystem->RelativePathToFullPath("gameinfo.txt", "MOD", szGameInfoPath, sizeof(szGameInfoPath));
	KeyValues* pKvGameInfoCubemap = ReadKeyValuesFile(szGameInfoPath);
	
	if (!pKvGameInfoCubemap)
	{
		Warning("Warning: Could not read gameinfo.txt KeyValues for Cubemap building at: %s\n"
			    "Warning: Skipping cubemap compile for %s, FAIL!\n",
				szGameInfoPath, 
				level_name
		);
		return;
	}

	// We want to get the name of the .exe, this is done through a keyvalue in gameinfo.txt, the target will depend of the arquitecture.
	// e.g (Half-Life 2): if on 32 bits, vrad will read GameExecutableName32bits and execute hl2.exe
	// GameExecutableName32bits		hl2
	// GameExecutableName64bits		hl2_win64
	const char* pGameExecutableName = pKvGameInfoCubemap->GetString(IsPlatform64Bits() ? CUBEMAPBUILDER_KV_64BITS_EXE : CUBEMAPBUILDER_KV_32BITS_EXE);

	if (!pGameExecutableName)
	{
		Warning("Warning: Could not locate \'%s\' key %s in %s\n"
				"Warning: Skipping cubemap compile for %s, FAIL!\n",
				IsPlatform64Bits() ? CUBEMAPBUILDER_KV_64BITS_EXE : CUBEMAPBUILDER_KV_32BITS_EXE,
				pGameExecutableName, 
				szGameInfoPath,
				level_name
		);
		return;
	}

	V_snprintf(szGameExecutableName, sizeof(szGameExecutableName), "%s.exe", pGameExecutableName);

	V_strcpy(szTempGameDir, gamedir);
	V_StripTrailingSlash(szTempGameDir);

	LoadGameInfoConvar(szGameInfoPath, szKVconvars, sizeof(szKVconvars), bHdrMode);

	V_snprintf(szBuildCubemapsCommandLine, sizeof(szBuildCubemapsCommandLine),
	" -sw -w %d -h %d -dev -novid -insecure -console -buildcubemaps %d -game \"%s\" +map %s %s ",
			GetSystemMetrics(SM_CXSCREEN), 
			GetSystemMetrics(SM_CYSCREEN), 
			bHdrMode ? g_iBuildHdrCubemapPasses : g_iBuildLdrCubemapPasses,
			szTempGameDir,
			level_name,
			szKVconvars
	);

	// Excecute (game).exe to build the cubemaps.
	Msg("Starting the executable (%s), Comamnd line:%s \n", szGameExecutableName, szBuildCubemapsCommandLine);
	FileSystem_GetAppInstallDir(szGameExecutablePath, sizeof(szGameExecutablePath));
	V_snprintf(szGameExecutablePath, sizeof(szGameExecutablePath), "%s\\%s %s", szGameExecutablePath, szGameExecutableName, szBuildCubemapsCommandLine);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, szGameExecutablePath, NULL, NULL, false, 0x00000000, NULL, NULL, &si, &pi))
	{
		Warning("Warning: %s could not start!\n"
				"Warning: Skipping cubemap compile for %s, FAIL!\n",
				szGameExecutablePath
		);
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	DWORD exitCode = 0;
	if (!GetExitCodeProcess(pi.hProcess, &exitCode))
	{
		if (exitCode > 0)
		{
			Warning("Warning: %s cubemaps compile failed: %d!\n"
					"Warning: The resulting BSP file may have issues!\n",
					szGameExecutableName, exitCode);
		}
		else
		{
			Msg("%s cubemaps compile complete!\n", szGameExecutableName);
		}
	}
	else
	{
		Warning("Warning: GetExitCodeProcess() failed!\n");
	}

	// Once the cubemap compile is complete we will copy the file again to the original bsp dir. This is done 
	// to not break older workflows (e.g: if the user wants use bspzip, vbspinfo or a postcompiler)
	CopyGameDirBspToOrignalBspDir();

	Msg("--> Cubemap builder complete in %.2f seconds\n\n\n", Plat_FloatTime() - start);
}