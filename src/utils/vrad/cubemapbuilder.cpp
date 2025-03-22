//========= Copyright Valve Corporation, All rights reserved. ============//
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
To enable CubemapBuilder in VRAD you have to setup your gameinfo.txt, lets take for example Half-Life 2 (hl2)
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
			+mat_specular	 		0
			+mat_hdr_level			2
			+building_cubemaps		1
		}

		// If '-BuildLdrCubemaps' is enabled, VRAD will load these commandline arguments/convars
		Ldr
		{
			+mat_hdr_level			0
			+mat_specular	 		0
		}
	}

*/


void CopyBspToGameDir()
{
	char mapdir[MAX_PATH], mapgamedir[MAX_PATH], mapgamedir_file[MAX_PATH];
	float start = Plat_FloatTime();

	V_snprintf(mapdir, sizeof(mapdir), "%s", source);
	V_snprintf(mapgamedir, sizeof(mapgamedir), "%s\maps", gamedir);
	V_snprintf(mapgamedir_file, sizeof(mapgamedir_file), "%s\maps\\%s.bsp", gamedir, level_name);

	Msg("Map original directory: %s\n", mapdir);
	Msg("Map game folder: %s\n", mapgamedir);
	Msg("Map game directory: %s\n", mapgamedir_file);

	Msg("Copying source %s.bsp to game directory... ", level_name);

	// Ensure the "maps" directory exists before copying
	CreateDirectory(mapgamedir, NULL);

	// Set attributes to normal in case of restrictions
	SetFileAttributes(mapgamedir_file, FILE_ATTRIBUTE_NORMAL);
	SetFileAttributes(mapdir, FILE_ATTRIBUTE_NORMAL);

	if (!CopyFile(mapdir, mapgamedir_file, FALSE))
	{
		DWORD error = GetLastError();
		Error("\nCould not copy to game directory %s\n"
			"Error CopyFile() : %lu, %s\n",
			mapgamedir_file, error,
			error == ERROR_ACCESS_DENIED ? "Access denied! Check permissions and file attributes.\n" : "");
	}
	else
	{
		float end = Plat_FloatTime();
		Msg("done (%f)\n", end - start);
	}
}


void CopyGameDirBspToOrignalBspDir()
{
	char mapdir[MAX_PATH], mapgamedir[MAX_PATH], mapgamedir_file[MAX_PATH];
	float start = Plat_FloatTime();

	V_snprintf(mapdir, sizeof(mapdir), "%s", source);
	V_snprintf(mapgamedir, sizeof(mapgamedir), "%s\maps", gamedir);
	V_snprintf(mapgamedir_file, sizeof(mapgamedir_file), "%s\maps\\%s.bsp", gamedir, level_name);

	Msg("Copying game %s.bsp to source directory... ", level_name);

	// Ensure the "maps" directory exists before copying
	CreateDirectory(mapgamedir, NULL);

	// Set attributes to normal in case of restrictions
	SetFileAttributes(mapgamedir_file, FILE_ATTRIBUTE_NORMAL);
	SetFileAttributes(mapdir, FILE_ATTRIBUTE_NORMAL);

	if (!CopyFile(mapgamedir_file, mapdir, FALSE))
	{
		DWORD error = GetLastError();
		Error("\nCould not copy to source directory %s\n"
			"Error CopyFile() : %lu, %s\n",
			mapgamedir_file, error,
			error == ERROR_ACCESS_DENIED ? "Access denied! Check permissions and file attributes.\n" : "");
	}
	else
	{
		float end = Plat_FloatTime();
		Msg("done (%f)\n", end - start);
	}
}


void LoadGameInfoConvar(const char* GameInfoPath,char* Convar, std::size_t ConvarSize, bool bHdrMode) 
{
	float start, end;
	start = Plat_FloatTime();

	Msg("Loading Convars from gameinfo.txt... ");

	KeyValues* GameInfoKVCubemap = ReadKeyValuesFile(GameInfoPath);
	KeyValues* CubemapBuilder = GameInfoKVCubemap->FindKey("CubemapBuilder");
	
	if(CubemapBuilder == NULL)
	{
		Warning("\nCould not load KeyValues for CubemapBuilder!"
				"\nCubemaps might not look right!\n" );
		return;
	}
	
	const char* SetGameToMaximunGraphic = CubemapBuilder->GetString("SetGameToMaximunGraphic", "0");
	bool bSetGameToMaximunGraphic = atoi(SetGameToMaximunGraphic) == 1 ? true : false;

	KeyValues* LightingMode = CubemapBuilder->FindKey(bHdrMode ? "Hdr" : "Ldr");

	if (LightingMode == NULL)
	{
		Warning("\nCould not load KeyValues for %s!"
				"\nCubemap compile might not look right!\n"
				,bHdrMode ? "Hdr" : "Ldr");
		return;
	}

	char GameInfoConvar[4096] = " ";

	for (KeyValues* subKey = LightingMode->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey()) 
	{
		V_snprintf(GameInfoConvar, sizeof(GameInfoConvar), "%s %s %s", GameInfoConvar, subKey->GetName(), subKey->GetString());
	}

	V_snprintf(Convar, ConvarSize, "%s %s", GameInfoConvar, bSetGameToMaximunGraphic ?
		" +r_lightmap_bicubic 1"
		" +r_waterforceexpensive 1" 
		" +mat_antialias 8" 
		" +mat_picmip -10"
		" +mat_forceaniso 16" : ""
	);

	end = Plat_FloatTime();
	Msg("done (%f)\n", end - start);
}


void BuildCubemaps(bool bHdrMode)
{
	float start, end;
	start = Plat_FloatTime();
	Msg("\n\nBuilding %s.bsp cubemaps on %s mode\n", level_name, bHdrMode ? "Hdr" : "Ldr");

	CopyBspToGameDir();

	// We want to get the name of the .exe, this is done through a keyvalue in gameinfo.txt, the target will depend of the arquitecture.
	// e.g (Half-Life 2): if on 32 bits, vrad will read GameExecutableName32bits and execute hl2.exe
	// GameExecutableName32bits		hl2
	// GameExecutableName64bits		hl2_win64
	char	GameInfoPath[MAX_PATH], gameexecutablename[MAX_PATH], gamexecutablepath[MAX_PATH*8], KVconvars[1024], _gamedir[MAX_PATH];
	g_pFullFileSystem->RelativePathToFullPath("gameinfo.txt", "GAME", GameInfoPath, sizeof(GameInfoPath));
	KeyValues* GameInfoKVCubemap = ReadKeyValuesFile(GameInfoPath);
	
	if (!GameInfoKVCubemap)
	{
		Error("Could not locate gameinfo.txt for Cubemap building at %s\n", GameInfoPath);
	}

	// Generates the game executable name, e.g: hl2_win64.exe
	const char* GameExecutableName = GameInfoKVCubemap->GetString(PLATFORM_64BITS ? "GameExecutableName64bits" : "GameExecutableName32bits", NULL);

	if (!GameExecutableName)
	{
		Error("Could not locate '%s' key %s in %s\n", PLATFORM_64BITS ? "GameExecutableName64bits" : "GameExecutableName32bits", GameExecutableName, GameInfoPath);
	}

	V_snprintf(gameexecutablename, sizeof(gameexecutablename), "%s.exe", GameExecutableName);

	// Setup the commandline strings for (game).exe
	strcpy(_gamedir, gamedir);
	if (strlen(_gamedir) > 0 && _gamedir[strlen(_gamedir) - 1] == '\\')
		_gamedir[strlen(_gamedir) - 1] = '\0';

	char buildcubemapscommandline[MAX_PATH*8];

	LoadGameInfoConvar(GameInfoPath, KVconvars, sizeof(GameInfoPath), bHdrMode);

	V_snprintf(buildcubemapscommandline, sizeof(buildcubemapscommandline), 
	" -sw -w %d -h %d -dev -novid -insecure -console -buildcubemaps -game \"%s\" +map %s %s ",
			GetSystemMetrics(SM_CXSCREEN), 
			GetSystemMetrics(SM_CYSCREEN),  
			_gamedir,
			level_name,
			KVconvars
			);

	// Excecute (game).exe to build the cubemaps.
	Msg("Starting the executable (%s), Comamnd line:%s \n", gameexecutablename, buildcubemapscommandline);
	FileSystem_GetAppInstallDir(gamexecutablepath, sizeof(gamexecutablepath));
	V_snprintf(gamexecutablepath, sizeof(gamexecutablepath), "%s\\%s %s", gamexecutablepath, gameexecutablename, buildcubemapscommandline);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, gamexecutablepath, NULL, NULL, false, 0x00000000, NULL, NULL, &si, &pi))
	{
		Error("%s could not start!\n", gameexecutablename);
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
			Error("%s cubemaps compile failed: %d!\n", gameexecutablename, exitCode);
		}
		else
		{
			Msg("%s cubemaps compile complete!\n", gameexecutablename);
		}
	}
	else
	{
		Error("GetExitCodeProcess() failed!\n");
	}

	// Once the cubemap compile is complete we will copy the file again to the original bsp dir. This is done 
	// to not break older workflows (e.g: if the user wants use bspzip, vbspinfo or a postcompiler)
	CopyGameDirBspToOrignalBspDir();

	end = Plat_FloatTime();
	printf("--> Cubemap builder complete in %f\n\n\n", end - start);
}