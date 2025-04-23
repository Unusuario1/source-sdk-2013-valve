#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"
#include "KeyValues.h"

#include "mapbuilder.h"
#include "colorscheme.h"
#include "shared.h"


//Here is how the 'MapBuilder' KV looks like inside 'ContentBuilder' KV
//		MapBuilder
//		{
//			Vbsp
//			{
//		
//			}
//
//			Vvis
//			{
//		
//			}
//
//			Vrad
//			{
//		
//			}
//
//			VbspInfo
//			{
//
//			}
//
//			//NavBuilder 1
//			//NodeGraphBuilder 1
//		}


namespace MapBuilder
{
	//-----------------------------------------------------------------------------
	// Purpose:	Check if vbsp, vvis, vrad & vbspinfo exists
	//-----------------------------------------------------------------------------
	void AssetToolCheck(const char* gamebin)
	{
		//Do vbsp, vvis, vrad, vbspinfo exist??
		Shared::AssetToolCheck(gamebin, NAME_MAP_GEOMETRY_TOOL, "MapBuiler");
		Shared::AssetToolCheck(gamebin, NAME_MAP_VISIBILITY_TOOL, "MapBuiler");
		Shared::AssetToolCheck(gamebin, NAME_MAP_RADIOSITY_TOOL, "MapBuiler");
		Shared::AssetToolCheck(gamebin, NAME_MAP_BPSINFO_TOOL, "MapBuiler");
	}


	//-----------------------------------------------------------------------------
	// Purpose: This returns the command line strings of vbsp, vvis , vrad & vbspinfo,
	//			since maps have a complex compile we use a custom version of Shared::LoadGameInfoKv
	//-----------------------------------------------------------------------------
	void LoadGameInfoKv(char* vbsp_command, char* vvis_command, char* vrad_command, char* vbspinfo_command, std::size_t bufferSize)
	{
		KeyValues* GameInfoKVCubemap = ReadKeyValuesFile(g_gameinfodir);

		if (!GameInfoKVCubemap)
		{
			Shared::qError("Could not get KeyValues from %s!\n", g_gameinfodir);
		}

		KeyValues *ContentBuilderKV = GameInfoKVCubemap->FindKey(CONTENTBUILDER_KV, false);

		if (!ContentBuilderKV)
		{
			Shared::qError("Could not get KeyValues from %s!\n", g_gameinfodir);
		}

		KeyValues* MapBuilderKV = ContentBuilderKV->FindKey(MAPBUILDER_KV, false);

		if (!MapBuilderKV)
		{
			Shared::qError("Could not get \'%s\' KeyValues from \'%s\'!\n", MAPBUILDER_KV, g_gameinfodir);
		}

		KeyValues* VbspKv = MapBuilderKV->FindKey(MAP_GEOMETRY_KV, false);
		KeyValues* VvisKv = MapBuilderKV->FindKey(MAP_VISIBILITY_KV, false);
		KeyValues* VradKv = MapBuilderKV->FindKey(MAP_RADIOSITY_KV, false);
		KeyValues* VbspinfoKv = MapBuilderKV->FindKey(MAP_BSPINFO_KV, false);

		if (!VbspKv || !VvisKv || !VradKv || !VbspinfoKv)
		{
			Shared::qError("Could not get %s or %s or %s or %s KeyValues from %s!\n", 
				MAP_GEOMETRY_KV, MAP_VISIBILITY_KV, MAP_RADIOSITY_KV, MAP_BSPINFO_KV, MAPBUILDER_KV);
		}

		for (KeyValues* subKey = VbspKv->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
		{
			V_snprintf(vbsp_command, bufferSize, "%s %s", subKey->GetName(), subKey->GetString());
		}

		for (KeyValues* subKey = VvisKv->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
		{
			V_snprintf(vvis_command, bufferSize, "%s %s", subKey->GetName(), subKey->GetString());
		}		
		
		for (KeyValues* subKey = VradKv->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
		{
			V_snprintf(vrad_command, bufferSize, "%s %s", subKey->GetName(), subKey->GetString());
		}		
		
		for (KeyValues* subKey = VbspinfoKv->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
		{
			V_snprintf(vbspinfo_command, bufferSize, "%s %s", subKey->GetName(), subKey->GetString());
		}
		
		// Note: we dont mount the .vmf file here, we do it in MapCompile since there it scans mapsrc!!
		V_snprintf(vbsp_command, bufferSize, " %s %s %s -game \"%s\"  ", "", TOOL_VERBOSE_MODE, vbsp_command, gamedir);
		V_snprintf(vvis_command, bufferSize, " %s %s %s -game \"%s\"  ", "", TOOL_VERBOSE_MODE, vvis_command, gamedir);
		V_snprintf(vrad_command, bufferSize, " %s %s %s -game \"%s\"  ", "", TOOL_VERBOSE_MODE, vrad_command, gamedir);
		V_snprintf(vbspinfo_command, bufferSize, " %s %s %s -game \"%s\"  ", "", TOOL_VERBOSE_MODE, vbspinfo_command, gamedir); //Check vbspinfo command line!!
	}


	//-----------------------------------------------------------------------------
	// Purpose:	We copy the .bsp to the game dir (e.g: mapsrc -> maps)
	//-----------------------------------------------------------------------------
	void CopyBspToGameDir(const char* mapsrc, const char* level_name)
	{
		char mapdir[MAX_PATH], mapgamedir[MAX_PATH], mapgamedir_file[MAX_PATH];
		float start = Plat_FloatTime();

		V_snprintf(mapdir, sizeof(mapdir), "%s", mapsrc);							// (e.g: "C:\Half Life 2\hl2\mapsrc\test.bsp")
		V_snprintf(mapgamedir, sizeof(mapgamedir), "%s\\%s", gamedir, MAPS_DIR);	// (e.g: "C:\Half Life 2\hl2\maps")
		V_snprintf(mapgamedir_file, sizeof(mapgamedir_file), "%s",					/*This causes a memory leak!*/
				Shared::ReplaceSubstring(mapsrc, MATERIALSRC_DIR, MATERIALS_DIR));	// (e.g: "C:\Half Life 2\hl2\maps\test.bsp")

		Msg("Map original directory: ");	ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", mapdir);
		Msg("Map game folder:		 ");	ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", mapgamedir);
		Msg("Map game directory:     ");	ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", mapgamedir_file);

		Msg("Copying source .bsp to game directory... ");

		// Ensure the "maps" directory exists before copying
		if (!CreateDirectory(mapgamedir, NULL))
		{
			DWORD err = GetLastError();
			if (err != ERROR_ALREADY_EXISTS)
			{
				Error("\nCould not create temporary directory at: \"%s\" (Error code: %lu)\n", mapgamedir, err);
			}
		}

		// Set attributes to normal in case of restrictions
		SetFileAttributes(mapgamedir_file, FILE_ATTRIBUTE_NORMAL);
		SetFileAttributes(mapdir, FILE_ATTRIBUTE_NORMAL);

		if (!CopyFile(mapdir, mapgamedir_file, FALSE))
		{
			DWORD error = GetLastError();
			Shared::qError("\nCould not copy to game directory %s\n"
				"Error CopyFile() : %lu, %s\n",
				mapgamedir_file, error,
				error == ERROR_ACCESS_DENIED ? "Access denied! Check permissions and file attributes.\n" : "");
		}
		else
		{
			ColorSpewMessage(SPEW_MESSAGE, &done_color, "done (%f)\n", Plat_FloatTime() - start);
		}
	}


	//-----------------------------------------------------------------------------
	// Purpose:	given a string we replace the .vmf or .vmn to .bsp
	//-----------------------------------------------------------------------------
	void ReplaceVmfBspExten(char* szPath)
	{
		char* szBspPath = strdup(szPath);

		if(strlen(szPath) > 0 && (strstr(szPath, MAPSRC_EXTENSION) || strstr(szPath, MAPSRC_EXTENSION2)))
		{
			szBspPath[strlen(szPath) - strlen(MAPSRC_EXTENSION)] = '\0';
			
			V_snprintf(szPath, MAX_PATH, "%s%s", szBspPath, MAPS_EXTENSION);
		}

		free(szBspPath);
	}


	//TODO: Add here exclusion folders!! smth like ExclusionPathVmf { } K
	//-----------------------------------------------------------------------------
	// Purpose:	Compile all the assets found in the given directory
	//-----------------------------------------------------------------------------
	void MapProcessRec(	char * vbsp_command, char* vvis_command,char* vrad_command, char* vbspinfo_command, const char* directory, const char* extension)
	{
		char searchPath[MAX_PATH];
		V_snprintf(searchPath, sizeof(searchPath), "%s\\*", directory);

		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(searchPath, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			const char* name = findFileData.cFileName;
			if (V_strcmp(name, ".") == 0 || V_strcmp(name, "..") == 0) 
			{
				continue;
			}

			char fullPath[MAX_PATH];
			V_snprintf(fullPath, sizeof(fullPath), "%s\\%s", directory, name);

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				MapProcessRec(	vbsp_command, vvis_command, vrad_command,
								vbspinfo_command, fullPath, extension);
			}
			else if (Shared::HasExtension(name, extension))
			{
				char vmfPath[MAX_PATH] = "", bspPath[MAX_PATH] = "";
				char _temp_vbsp[4096] = "", _temp_vvis[4096] = "", _temp_vrad[4096] = "", _temp_vbspinfo[4096] = "";

				if (!Shared::PartialBuildAsset(fullPath, MAPSRC_DIR, MAPS_DIR))
					continue;

				V_snprintf(vmfPath, sizeof(vmfPath), "%s", fullPath); // game/mod/mapsrc/vmfname.vmf
				V_snprintf(bspPath, sizeof(bspPath), "%s", fullPath); // game/mod/mapsrc/vmfname.bsp
				ReplaceVmfBspExten(bspPath);

				// Now we get append the path of the .vmf file to vbsp
				V_snprintf(_temp_vbsp, sizeof(_temp_vbsp), "%s \"%s\"", vbsp_command, vmfPath);
				// Now we get append the path of the .bsp file to vvis, vrad & vbspinfo
				V_snprintf(_temp_vvis, sizeof(_temp_vvis), "%s \"%s\"", vvis_command, bspPath);
				V_snprintf(_temp_vrad, sizeof(_temp_vrad), "%s \"%s\"", vrad_command, bspPath);
				V_snprintf(_temp_vbspinfo, sizeof(_temp_vbspinfo), "%s \"%s\"", vbspinfo_command, bspPath);

				Shared::StartExe("Geometry", NAME_MAP_GEOMETRY_TOOL, _temp_vbsp);		//vbsp
				Shared::StartExe("Visibility", NAME_MAP_VISIBILITY_TOOL, _temp_vvis);	//vvis
				Shared::StartExe("Radiosity", NAME_MAP_RADIOSITY_TOOL, _temp_vrad);		//vrad

				// Extra .bsp info!
				if (verbose)
				{
					Shared::StartExe("Bsp Info", NAME_MAP_BPSINFO_TOOL, _temp_vbspinfo); //vbspinfo
				}

				// Now that we have all the .bsp compiled we copy it to game/mod/mapsrc -> game/mod/maps 
				CopyBspToGameDir(bspPath, findFileData.cFileName);

				// We cleanup the temp files generated by vbsp, vvis, vrad
				if (g_cleanuptempcontent)
				{
					float start = Plat_FloatTime();

					Msg("Deteling temp files... ");
					char* szLogFile = Shared::ReplaceSubstring(bspPath, MAPS_EXTENSION, ".log");
					char* szPortalFile = Shared::ReplaceSubstring(bspPath, MAPS_EXTENSION, ".prt");
					char* szLinFile = Shared::ReplaceSubstring(bspPath, MAPS_EXTENSION, ".lin");

					// We delete the .log, .lin, .ptr, .bsp in mapsrc
					const char* filesToDelete[] = { bspPath, szLogFile, szPortalFile, szLinFile };
					for (const char* file : filesToDelete)
					{
						if (remove(file) != 0)
						{
							Warning("Could not delete \"%s\" file!\n", file);
						}
					}

					free(szLogFile);
					free(szPortalFile);
					free(szLinFile);
					ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);
				}

				//TODO: build nav and nodegraph meshes! (at the moment the engine doenst support it)
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Build all the inside maps given a dir		
	//-----------------------------------------------------------------------------
	void MapCompile()
	{
		char vbsp_command[4096] = "", vvis_command[4096] = "", vrad_command[4096] = "", vbspinfo_command[4096] = "", mapSrcPath[MAX_PATH];
		bool bContinueVmf = true, bContinueVmn = true;

		Shared::PrintHeaderCompileType("Maps");

		V_snprintf(mapSrcPath, sizeof(mapSrcPath), "%s\\%s", gamedir, MAPSRC_DIR); // (e.g: "C:\Half Life 2\hl2\mapsrc")
			
		bContinueVmf = Shared::DirectoryAssetTypeExist(mapSrcPath, MAPSRC_EXTENSION, "maps");
		bContinueVmn = Shared::DirectoryAssetTypeExist(mapSrcPath, MAPSRC_EXTENSION2, "maps");
		if (!bContinueVmf && !bContinueVmn)
			return;

		Msg("%s", (g_quiet || !g_spewallcommands) ? "Asset report:\n" : "");
		Shared::AssetInfoBuild(mapSrcPath, MAPSRC_EXTENSION);
		Shared::AssetInfoBuild(mapSrcPath, MAPSRC_EXTENSION2);
		if (g_infocontent)
			return;

		MapBuilder::LoadGameInfoKv(vbsp_command, vvis_command, vrad_command, vbspinfo_command, sizeof(char)*4096);

		// We do this because in case we have to compile both .vmf & .vmn files
		if (bContinueVmf) 
		{
			MapProcessRec(	vbsp_command, vvis_command, vrad_command,
							vbspinfo_command, mapSrcPath, MAPSRC_EXTENSION);
		}		
		if (bContinueVmn)
		{
			MapProcessRec(	vbsp_command, vvis_command, vrad_command,
							vbspinfo_command, mapSrcPath, MAPSRC_EXTENSION2);
		}
	}
}
