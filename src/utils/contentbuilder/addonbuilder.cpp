//========= --------------------------------------------------- ============//
//
// Purpose: AddonBuilder – A ContentBuilder subsystem for organizing and 
//          packaging game content for Steam Workshop release (mp maps).
//
// $NoKeywords: $
//=============================================================================//
#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "addonbuilder.h"
#include "colorscheme.h"
#include "shared.h"

/*
        // This will only work if -addonbuild and -mp_addon is enabled in contentbuilder
        BspZipMapPacking
        {
            LevelToBePack   " "     // (e.g: template.bsp)
            BuildParams     " "

            // We exclude by default (mapsrc, materialsrc, modelsrc, scenes, caption, shaders, bin, _build)
            // also .ptr, .lin, .log files
            ExcludeBspPacking
            {
                ExcludeFileOrFolder "gameinfo.txt"
                ExcludeFileOrFolder "contentbuilder.txt"
            }
        }
*/

namespace AddonBuilder
{
    //-----------------------------------------------------------------------------
    // Purpose:	Check if bspip.exe exists
    //-----------------------------------------------------------------------------
    void AssetToolCheck(const char* pGameBin)
    {
        Shared::AssetToolCheck(pGameBin, NAME_MAP_ZIP_TOOL, ADDONBUILDER_KV);
    }


    static void LoadGameInfoKv(char* pToolArgv, std::size_t uiBufferSize, const char* bspzip_contentlist)
    {
        char szKvToolArgv[2048] = "";
        char *bsppath = "";
        //TODO: LOAD THESE KEYVALUES AND pass it to bspzip!

        V_snprintf(pToolArgv, uiBufferSize, " %s %s \"%s\" \"%s\" \"%s\" ", DEFAULT_ZIP_COMMANDLINE, szKvToolArgv, bsppath, bspzip_contentlist, bsppath);
    }


    void AddonCompile()
    {
        // we generate the list and save it to txt, then we run the command line and exec that
        const char* PackFolderList[] = { MATERIALS_DIR, MODELS_DIR, SOUNDS_DIR, "particles", "expressions", "cfg", "scripts", "resource", /*tf stuff*/ "classes","media"};
        char szBspZipToolArgv[8192] = "", szBspZipListPath[MAX_PATH] = "", szBspZipListFile[MAX_PATH] = "";

        Shared::PrintHeaderCompileType("Addon");

        //scan all folders and genereate the list
        V_snprintf(szBspZipListPath, sizeof(szBspZipListPath), "%s\\%s", g_contentbuilderPath, TEMP_BSPZIP_DIR);

        if (!Shared::CreateDirectoryRecursive(szBspZipListPath))
        {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS)
            {
                Shared::qError("AssetSystem -> Could not create temporary directory at: \"%s\" (Error code: %lu)\n", szBspZipListPath, err);
                g_process_error++;
                return;
            }
        }

        SYSTEMTIME st;
        GetLocalTime(&st);

        V_snprintf(szBspZipListFile, sizeof(szBspZipListFile), "%s\\addon_bspzip_build(%02d:%02d:%04d %02d:%02d:%02d).txt",
            szBspZipListPath,
            st.wMonth, st.wDay,
            st.wYear, st.wHour,
            st.wMinute, st.wSecond);

        LoadGameInfoKv(szBspZipToolArgv, sizeof(szBspZipToolArgv), szBspZipListFile);

        for (const char* folder : PackFolderList) 
        {
            if (!Shared::ScanFolderSaveContents(folder, szBspZipListFile, ADDONBUILDER_KV))
            {
                Shared::qWarning("AssetSytem -> Skipping \"\\%s\" folder!\n", folder);
            }
        }

        //Start bspzip
        Shared::StartExe("Addon", NAME_MAP_ZIP_TOOL, szBspZipToolArgv);

        if(g_cleanuptempcontent)
        {
            if(remove(szBspZipListFile)!=0)
            {
                Shared::qWarning("AssetSystem -> Could not remove temp file: \"%s\"\n", szBspZipListFile);
            }
        }
    }
}