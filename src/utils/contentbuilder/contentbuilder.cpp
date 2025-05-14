//========= --------------------------------------------------- ============//
//
// Purpose: ContentBuilder - Tool for compiling, processing, and packaging game assets 
//
// $NoKeywords: $
//=============================================================================//
#include <windows.h>
#include "stdlib.h"
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "loadcmdline.h"
#include "cmdlib.h"
#include "filesystem_init.h"
#include "filesystem_tools.h"

#include "contentbuilder.h"
#include "materialbuilder.h"
#include "modelbuilder.h"
#include "soundbuilder.h"
#include "scenebuilder.h"
#include "captionbuilder.h"
#include "mapbuilder.h"
#include "vpkbuilder.h"
#include "shared.h"
#include "colorscheme.h"

#ifdef MP_ADDON_SUPPORT
#include "addonbuilder.h"
#endif // MP_ADDON_SUPPORT


// TODO List:
// - Add multithreading support for 
// -    materialbuilder 
// -    modelbuilder 
// -    captionbuilder 
// -    (soundbuilder) -?
// - Fix the FUCKING issue with reading keyvalues for contentbuilder.txt
// - Fix the -lb build not working.
// - Fix the issue with exclude paths.
// - Properly implement support for -mp_addon command. ------------------------------------------------------- DONE
// - Fix the log function not working.
// - Add a -steambuild command. -?
// - (Optional) Use the Valve asset system for paths (not strictly necessary).
// - Test the following:
//     * nosteam addon
//     * nosteam game 
//     * steam addon
//     * game build
//     * game/tf_addons folder
// - Replace usage of gameinfo.txt with contentbuilder.txt. -------------------------------------------------- DONE
// - Set up a logging system that includes:
//     * Standard logs --------------------------------------------------------------------------------------- DONE
//     * Warnings: warning.log ------------------------------------------------------------------------------- DONE
//     * Errors: error.log ----------------------------------------------------------------------------------- DONE
//     * Example outputs:
//         _build/build(date time).log ----------------------------------------------------------------------- DONE
//         _build/asset_report_source.contentlist ------------------------------------------------------------ DONE
//         _build/asset_report_compiled.contentlist ---------------------------------------------------------- DONE
// - In VPK builder, add all folders (and files) that need to be packed.
// - VPK builder maybe add a santity check to se if we can alloc all the temp files
// - Finish addonbuiler for mp addons.
// - Rename vars to have the hungarian notation:
// -	contentbuilder.cpp
// -	contentbuilder.h
// -	colorscheme.cpp
// -	colorscheme.h
// -	captionbuilder.cpp ----------------------------------------------------------------------------------- DONE
// -	captionbuilder.h ------------------------------------------------------------------------------------- DONE
// -	scenebuilder.cpp ------------------------------------------------------------------------------------- DONE
// -	scenebuilder.h --------------------------------------------------------------------------------------- DONE
// -	mapbuilder.cpp
// -	mapbuilder.h
// -	materialbuilder.cpp ---------------------------------------------------------------------------------- DONE
// -	materialbuilder.h ------------------------------------------------------------------------------------ DONE
// -	modelbuilder.cpp ------------------------------------------------------------------------------------- DONE
// -	modelbuilder.h --------------------------------------------------------------------------------------- DONE
// -	soundbuilder.cpp
// -	soundbuilder.h
// -	vpkbuilder.cpp --------------------------------------------------------------------------------------- DONE
// -	vpkbuilder.h ----------------------------------------------------------------------------------------- DONE
// -	addonbuilder.cpp
// -	addonbuilder.h
// -	shared.cpp
// -	shared.h
// - DO A MAJOR CLEANUP OF THE CODE mf!!
// - Add the valve header and its purpose smth like that ----------------------------------------------------- DONE
// - Maybe rework how the paths are handled in mat, mod, cap, sce, etc... ------------------------------------ NNAA
// - FIX scheme path issues ---------------------------------------------------------------------------------- DONE
// - Fix some paths not been in path_color ------------------------------------------------------------------- DONE
// - Add more verbose in the program!!


//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
std::size_t g_process_completed = 0;
std::size_t g_process_error = 0;
std::size_t g_timer         = 0;        // global timer for assets!
bool g_force32bits          = PLATFORM_64BITS ? true : false;
bool g_force64bits          = PLATFORM_64BITS ? true : false;
bool g_cleanuptempcontent   = true;     //Do we cleanup temp content?
bool g_infocontent          = false;    //Dont build, only print assets
bool g_nosteam              = false;    //No steam funtions
bool g_addonbuild           = false;    //Build the -game dir as addon instead of a game
bool g_buildcontent         = false;    
bool g_buildoutofdatecontent = false;
bool g_buildmaterials       = true;
bool g_buildmodels          = true;
bool g_buildsounds          = true;
bool g_buildscene           = true;
bool g_buildcaption         = true;
bool g_buildmap             = true;
bool g_buildvpk             = false;
bool g_ignoreerrors         = false;
bool g_pause                = false;    
bool g_spewallcommands      = false;
bool g_compileverbose       = false;
bool g_quiet                = false;
bool g_createlog            = true;
bool g_mp_addon             = true;
char g_gamebin[MAX_PATH]    = "";           //  game/bin or game/bin/x64
char g_steamdir[MAX_PATH]   = "";           //  game
char g_gameinfodir[MAX_PATH] = "";          //  game/mod/gameinfo.txt
char g_contentbuilderdir[MAX_PATH] = "";    //  game/mod/contentbuilder.txt
char g_contentbuilderPath[MAX_PATH] = "";   // game/mod/_build


//-----------------------------------------------------------------------------
// Purpose:   Wait until the user wants to exit the program
//-----------------------------------------------------------------------------
void HitKeyToContinue()
{
    if (g_pause)
    {
        system("pause");
    }
}


//-----------------------------------------------------------------------------
// Purpose:  Count how many assets we have to compile
//-----------------------------------------------------------------------------
std::size_t CountAllAssets()
{
    std::size_t i = 0;
    if (g_buildmaterials)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION1);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION2);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION3);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), MATERIALS_EXTENSION);
    }
    if (g_buildmodels)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, MODELSRC_DIR), MODELSRC_EXTENSION);
    }
    if (g_buildsounds)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, SOUNDSRC_DIR), SOUNDSRC_EXTENSION);
    }
    if (g_buildscene)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, SCENESRC_DIR), SCENESRC_EXTENSION);
    }
    if (g_buildcaption)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, CAPTIONSRC_DIR), CAPTIONSRC_EXTENSION);
    }
    if (g_buildmap)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, MAPSRC_DIR), MAPSRC_EXTENSION1);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MAPSRC_DIR), MAPSRC_EXTENSION2);
    }
    if (g_buildvpk)
    {
        i += 1;
    }

    return i;
}


//-----------------------------------------------------------------------------
// Purpose:   Get how many skips assets we got in the build
//-----------------------------------------------------------------------------
std::size_t GetSkippedAssets()
{
    std::size_t i = 0;
    if (!g_buildmaterials)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION1);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION2);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION3);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), MATERIALS_EXTENSION);
    }
    if (!g_buildmodels)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, MODELSRC_DIR), MODELSRC_EXTENSION);
    }
    if (!g_buildsounds)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, SOUNDSRC_DIR), SOUNDSRC_EXTENSION);
    }
    if (!g_buildscene)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, SCENESRC_DIR), SCENESRC_EXTENSION);
    }
    if (!g_buildcaption)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, CAPTIONSRC_DIR), CAPTIONSRC_EXTENSION);
    }
    if (!g_buildmap)
    {
        i += Shared::CountAssets(Shared::MergeString(gamedir, MAPSRC_DIR), MAPSRC_EXTENSION1);
        i += Shared::CountAssets(Shared::MergeString(gamedir, MAPSRC_DIR), MAPSRC_EXTENSION2);
    }
    if (!g_buildvpk)
    {
        i += 1;
    }

    return i;
}


//-----------------------------------------------------------------------------
// Purpose:     
//-----------------------------------------------------------------------------
void PreInit()
{
    // Note: Even though it's standard to add a '\' at the end of the string,
    // we remove it here because it makes managing paths much easier across the tool.
    V_StripTrailingSlash(gamedir);
}


//-----------------------------------------------------------------------------
// Purpose:   Create the log file  
//-----------------------------------------------------------------------------
void SetUpLogFile()
{
    char logFile[MAX_PATH] = "";

    V_snprintf(g_contentbuilderPath, sizeof(g_contentbuilderPath), "%s\\%s", gamedir, CONTENTBUILDER_OUTPATH);

    // Remove old files
    if (Shared::CheckIfPathOrFileExist(g_contentbuilderPath) && g_cleanuptempcontent)
    {
        Shared::DeleteFolderWithContents(g_contentbuilderPath);
    }

    if(!Shared::CreateDirectoryRecursive(g_contentbuilderPath))
    {
        DWORD err = GetLastError();

        if (err != ERROR_ALREADY_EXISTS)
        {
             Shared::qError("\nAssetSystem -> Could not create temporary directory at: \"%s\" (Error code: %lu)\n", g_contentbuilderPath, err);
            return;
        }
    }

    if (g_createlog)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);

        V_snprintf(logFile, sizeof(logFile), "%s\\%s_build(%02d:%02d:%04d %02d:%02d:%02d).log",
            g_contentbuilderPath, g_addonbuild ? "addon" : "game",
            st.wMonth, st.wDay,
            st.wYear, st.wHour,
            st.wMinute, st.wSecond);

        SetSpewFunctionLogFile(logFile);
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Check if the tools are in bin or bin/x64
//-----------------------------------------------------------------------------
void Init_AssetTools()
{
    char szExclude[128] = "";
    float start = Plat_FloatTime();

    g_timer = start;

    //Basic setup
    if (!g_nosteam)
    {
        FileSystem_GetAppInstallDir(g_steamdir, sizeof(g_steamdir));
        if (g_steamdir == nullptr)
        {
            Shared::qError("AssetSystem -> steam game dir is NULL! Check \'-steamgamedir\' command!\n");
            exit(-1);
        }
    }

    Shared::SetUpBinDir(g_gamebin, sizeof(g_gamebin));
    V_snprintf(g_gameinfodir, sizeof(g_gameinfodir), "%s\\%s", gamedir, GAMEINFO_FILENAME);
    V_snprintf(g_contentbuilderdir, sizeof(g_contentbuilderdir), "%s\\%s", gamedir, CONTENTBUILDER);

    SetUpLogFile();

    // Basic info
    Msg("\n");
    Msg("Working Directory:   ");   ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_steamdir);
    Msg("Mod:                 ");   ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", gamedir);
    Msg("Operation:           %s\n", g_buildcontent && !g_buildoutofdatecontent ? "Force-building content" : "Partial-building content");
    Msg("Verbosity:           %s %s\n", (verbose ? "High" : (g_quiet ? "Quiet" : "Standard")), (g_spewallcommands ? "(showing paths)" : ""));
    Msg("Building:            %s\n", g_addonbuild ? "Partial (addon)" : (g_buildmaterials && g_buildmodels && g_buildsounds &&
                                        g_buildscene && g_buildcaption && g_buildmap &&
                                        g_buildvpk) ? "Full (game)" : "Partial (game)"); // lol... peak c++ code

    if (!g_buildmaterials)   V_strcat(szExclude, "(materials) ", sizeof(szExclude));
    if (!g_buildmodels)      V_strcat(szExclude, "(models) ", sizeof(szExclude));
    if (!g_buildsounds)      V_strcat(szExclude, "(sounds) ", sizeof(szExclude));
    if (!g_buildscene)       V_strcat(szExclude, "(scene) ", sizeof(szExclude));
    if (!g_buildcaption)     V_strcat(szExclude, "(caption) ", sizeof(szExclude));
    if (!g_buildmap)         V_strcat(szExclude, "(maps) ", sizeof(szExclude));
    if (!g_buildvpk)         V_strcat(szExclude, "(vpk) ", sizeof(szExclude));
    else                     V_strcat(szExclude, "None ", sizeof(szExclude));
    Msg("Excluding:          %s\n\n", szExclude);

    if (g_spewallcommands)
    {
        //General paths
        ColorSpewMessage(SPEW_MESSAGE, &header_color, "\nGeneral paths:\n");
        Msg("\tPath - gameinfo.txt:         ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gameinfodir);
        Msg("\tPath - Game binary (tools):  ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gamebin);
        //g_pFullFileSystem->PrintSearchPaths();

        //Game/Mod source assets (src) paths
        ColorSpewMessage(SPEW_MESSAGE, &header_color, "Game/Mod Source/Compiled assets paths:\n");
        if (g_buildmaterials)   { Msg("\tContent Source - Materials:  ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MATERIALSRC_DIR); }
        if (g_buildmodels)      { Msg("\tContent Source - Models:     ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MODELSRC_DIR); }
        if (g_buildsounds)      { Msg("\tContent Source - Sounds:     ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SOUNDSRC_DIR); }
        if (g_buildscene)       { Msg("\tContent Source - Scene:      ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SCENE_DIR); }
        if (g_buildcaption)     { Msg("\tContent Source - Caption:    ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, CAPTIONSRC_DIR); }
        if (g_buildmap)         { Msg("\tContent Source - Maps:       ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MAPSRC_DIR); }
        if (g_buildvpk)         { Msg("\tContent Source - Valve Pack File (vpk): ");    ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", gamedir); }
        if (g_buildmaterials)   { Msg("\tContent Compiled - Materials:");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MATERIALS_DIR); }
        if (g_buildmodels)      { Msg("\tContent Compiled - Models:   ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MODELS_DIR); }
        if (g_buildsounds)      { Msg("\tContent Compiled - Sounds:   ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SOUNDS_DIR); }
        if (g_buildscene)       { Msg("\tContent Compiled - Scene:    ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SCENE_DIR); }
        if (g_buildcaption)     { Msg("\tContent Compiled - Caption:  ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, CAPTION_DIR); }
        if (g_buildmap)         { Msg("\tContent Compiled - Maps:     ");               ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MAPS_DIR); }
        if (g_buildcaption)     { Msg("\tContent Compiled - Valve Pack File (vpk): ");  ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", gamedir); }
    }

    //Create asset compiled dir
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &header_color : &white, "Initializing AssetSystem... %s", g_spewallcommands ? "\n" : "");
    
    const char* rgpszFolderList[] = { MATERIALS_DIR, MODELS_DIR, SOUNDS_DIR, SCENE_DIR, CAPTION_DIR, MAPS_DIR };
    for (const char* pFolder : rgpszFolderList)
    {
        Shared::CreateAssetSystemGamePath(gamedir, pFolder);
    }

    if (g_spewallcommands)
    {
        //scan source assets
        if (g_buildmaterials) Msg("\tAssetSystem -> Materials: %llu count\n", Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), MATERIALS_EXTENSION));
        if (g_buildmaterials) Msg("\tAssetSystem -> Textures:  %llu count\n", Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION1) +
                                                                              Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION2) +
                                                                              Shared::CountAssets(Shared::MergeString(gamedir, MATERIALSRC_DIR), TEXTURESRC_EXTENSION3));
        if (g_buildmodels)    Msg("\tAssetSystem -> Models:    %llu count\n", Shared::CountAssets(Shared::MergeString(gamedir, MODELSRC_DIR), MODELSRC_EXTENSION));
        if (g_buildsounds)    Msg("\tAssetSystem -> Sounds:    %llu count\n", Shared::CountAssets(Shared::MergeString(gamedir, SOUNDSRC_DIR), SOUNDSRC_EXTENSION));
        if (g_buildscene)     Msg("\tAssetSystem -> Scene:     %llu count\n", Shared::CountAssets(Shared::MergeString(gamedir, SCENESRC_DIR), SCENESRC_EXTENSION));
        if (g_buildcaption)   Msg("\tAssetSystem -> Caption:   %llu count\n", Shared::CountAssets(Shared::MergeString(gamedir, CAPTIONSRC_DIR), CAPTIONSRC_EXTENSION));
        if (g_buildmap)       Msg("\tAssetSystem -> Maps       %llu count\n", Shared::CountAssets(Shared::MergeString(gamedir, MAPSRC_DIR), MAPSRC_EXTENSION1) +
                                                                              Shared::CountAssets(Shared::MergeString(gamedir, MAPSRC_DIR), MAPSRC_EXTENSION2));
    }
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &sucesfullprocess_color : &done_color, "Done in %.2f seconds.\n", Plat_FloatTime() - start);

    //Tools paths
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &header_color : &white, "Initializing AssetTools systems (%s)... %s", Shared::TargetPlatform() ? "64 bits" : "32 bits", g_spewallcommands ? "\n" : ""); // I know....
    if (g_buildmaterials)   MaterialBuilder::AssetToolCheck(g_gamebin);
    if (g_buildmodels)      ModelBuilder::AssetToolCheck(g_gamebin);
    if (g_buildsounds)      SoundBuilder::AssetToolCheck(g_gamebin);
    if (g_buildscene)       SceneBuilder::AssetToolCheck(g_gamebin);
    if (g_buildcaption)     CaptionBuilder::AssetToolCheck(g_gamebin);
    if (g_buildmap)         MapBuilder::AssetToolCheck(g_gamebin);
    if (g_buildvpk)         VpkBuilder::AssetToolCheck(g_gamebin);
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &sucesfullprocess_color : &done_color, "Done in %.2f seconds. All sub systems checks passed!\n", Plat_FloatTime() - start);

    Msg("Finalizing build list... \n");
    std::size_t uiCountAsset = CountAllAssets();
    Msg("%llu assets need to be built\n\n", uiCountAsset);
    
    // Sanity check
    if(uiCountAsset == 0)
    {
        Shared::qWarning("AssetSystem -> No assets to %s!\n", g_infocontent ? "print" : "build");
        exit(0);
    }

    // asset report!
    if (g_infocontent)
    {
        const char* FolderList[] = { MATERIALS_DIR ,MODELS_DIR ,SOUNDS_DIR ,SCENE_DIR ,CAPTION_DIR ,MAPS_DIR };
        const char* FolderListSrc[] = { MATERIALSRC_DIR ,MODELSRC_DIR ,SOUNDSRC_DIR ,SCENESRC_DIR ,CAPTIONSRC_DIR ,MAPSRC_DIR };
        char szAssetReportSource[MAX_PATH] = "", szAssetReportCompiled[MAX_PATH] = "", szTemp[MAX_PATH] = "";
        
        // asset source report
        start = Plat_FloatTime();
        V_snprintf(szAssetReportSource, sizeof(szAssetReportSource), "%s\\%s", g_contentbuilderPath, CONTENTBUILDER_ASSERT_R_SRC);
        remove(szAssetReportSource);
        Msg("AssetSystem -> Generating asset source report at: \"%s\"... ", szAssetReportSource);
        for (const char* folder : FolderListSrc)
        {
            V_snprintf(szTemp, sizeof(szTemp), "%s\\%s", gamedir, folder);
            qprintf("AssetSystemVerbose -> \"%s\"\n", szTemp);
            Shared::ScanFolderSaveContents(folder, szAssetReportSource, "Content Report");
        }
        ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);

        // asset compiled report
        start = Plat_FloatTime();
        V_snprintf(szAssetReportCompiled, sizeof(szAssetReportCompiled), "%s\\%s", g_contentbuilderPath, CONTENTBUILDER_ASSERT_R_COM);
        remove(szAssetReportCompiled);
        Msg("AssetSystem -> Generating asset compiled report at: \"%s\"... ", szAssetReportCompiled);
        for (const char* folder : FolderList)
        {
            V_snprintf(szTemp, sizeof(szTemp), "%s\\%s", gamedir, folder);
            qprintf("AssetSystemVerbose -> \"%s\"\n", szTemp);
            Shared::ScanFolderSaveContents(folder, szAssetReportCompiled, "Content Report");
        }
        ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Print contentbuilder usage
//-----------------------------------------------------------------------------
void PrintUsage(int argc, char* argv[])
{
    Msg("\nUsage: contentbuilder.exe [options] -game <path>\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " General Options:\n");
    Msg("   -b:                    Build the content.\n"
        "   -lb:                   Build only out-of-date content.\n"
        "   -pause:                After building the content, pause the program.\n"
        "   -info:                 Only prints all the assets to be compiled (no compile).\n"
        "   -forcedirty:           Do not remove temp content.\n"
        "   -nolog:                Disables log generation.\n"
        "   -nosteam:              Dont use steam api mount funtions. Use when \'-game\' path is outside the game root path.\n"
        "                          Useful for no steam mods builds, addon builds outside the game (steam) path.\n"
        "   -game <path>:          Specify the folder of the gameinfo.txt file.\n"
        "   -vproject <path>:      Same as \'-game\'.\n"
        "   -steamgamedir <path>:  if \'-nosteam\' is enabled, loads the steam game path,\n"
        "                          (e.g: \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Half-life 2\")\n"
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Building Options:\n");
    Msg("   -addonbuild:           Builds only maps, materials, models and sounds.\n"
        "   -mp_addon:             If enabled, it will compile the addon and pack all the contents into the defined .bsp file\n"
        "                          Useful for addons intended for release on the Workshop. (\'-addonbuild\' needs to be enabled to work)\n"
        "   -skipmaterial:         Skips texture (\\%s) compile.\n"
        "   -skipmodel:            Skips model (\\%s) compile.\n"
        "   -skipsound:            Skips sound (\\%s) compile.\n"
        "   -skipscene:            Skips scene (\\%s) compile.\n"
        "   -skipcaption:          Skips caption (\\%s) compile.\n"
        "   -skipmap:              Skips maps (\\%s) compile.\n"
        "   -vpk:                  Generate vpk files. (Not avaible for \'-addonbuild\').\n"
        "   -steambuild:           Generates a build for steam release content, (Not avaible for addon build, only for game builds)\n"
        "\n"
        , MATERIALSRC_DIR, MODELSRC_DIR, SOUNDSRC_DIR, SCENESRC_DIR, CAPTIONSRC_DIR, MAPSRC_DIR);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Spew Options:\n");
    Msg("   -v or -verbose:        Enables verbose.\n"
        "   -quiet:                Prints minimal text. (Note: Disables \'-verbose\' and \'-spewallcommands\')\n"
        "   -spewallcommands:                     \n"
        "   -compileverbose:       Enable verbose for tools. (Prints a LOT of text)\n"
        "   -spewallverbose:       Same as \'-v -spewallcommands -compileverbose\'\n"      
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Advanced Build Options:\n");
    Msg("   -toolsforce32bits:     Force contentbuilder to use 32 bits tools.\n"
        "   -toolsforce64bits:     Force contentbuilder to use 64 bits tools.\n"
        "   -proc n or -threads n: Max simultaneous compile subsystems. (Not avaible for %s, %s and %s).\n"
        "   -ignoreerrors:         Ignores errors unless is strictly necesary. (This WILL lead to undefined behaviour, BE CAREFUL!!)\n"
        "                          (NOT recomended for steam builds, only for testing and fast builds)\n"
        "\n", MAPBUILDER_KV, VPKBUILDER_KV, ADDONBUILDER_KV);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Other Options:\n");
    Msg("   -FullMinidumps:        Write large minidumps on crash.\n"
        "\n");

    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(1);
}


//-----------------------------------------------------------------------------
// Purpose:   Prints the header
//-----------------------------------------------------------------------------
void PrintHeader()
{
    Msg("\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, "//------------------------------------------------------------\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, "// "); Msg("Content builder (Build: %s %s)\n" ,__DATE__, __TIME__);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, "//------------------------------------------------------------\n");
}


//-----------------------------------------------------------------------------
// Purpose:   Parse command line
//-----------------------------------------------------------------------------
void ParseCommandline(int argc, char* argv[])
{
    ColorSpewMessage(SPEW_MESSAGE, &header_color, "\nCommand Line:\n\t");
    
    if(argc == 1 || argc == 2)
    {
        PrintUsage(argc, argv);
    }

    for (std::size_t i = 1; i < argc; ++i)
    {
        Msg("%s ", argv[i]);
    }

    for (int i = 1; i < argc; ++i)
    {
        if (!V_stricmp(argv[i], "-?") || !V_stricmp(argv[i], "-help") || argc == 1)
        {
            PrintUsage(argc, argv);
        }
        else if (!V_stricmp(argv[i], "-v") || !V_stricmp(argv[i], "-verbose"))
        {
            verbose = true;
        }  
        else if (!V_stricmp(argv[i], "-threads") || !V_stricmp(argv[i], "-proc"))
        {
            //threads = true; //TODO!
        }
        else if (!V_stricmp(argv[i], "-b"))
        {
            g_buildcontent = true;
        }        
        else if (!V_stricmp(argv[i], "-lb"))
        {
            g_buildcontent = true;
            g_buildoutofdatecontent = true;
        }
        else if (!V_stricmp(argv[i], "-pause"))
        {
            g_pause = true;
        }
        else if (!V_stricmp(argv[i], "-info"))
        {
            g_infocontent = true;
            g_buildcontent = true;
        }
        else if (!V_stricmp(argv[i], "-forcedirty"))
        {
            g_cleanuptempcontent = false;
        }       
        else if (!V_stricmp(argv[i], "-nolog"))
        {
            g_createlog = false;
        }
        else if (!Q_stricmp(argv[i], "-FullMinidumps"))
        {
            EnableFullMinidumps(true);
        }
        else if (!V_stricmp(argv[i], "-toolsforce32bits"))
        {
            g_force32bits = true;
            g_force64bits = false;
        }
        else if (!V_stricmp(argv[i], "-toolsforce64bits"))
        {
            g_force32bits = false;
            g_force64bits = true;
        }
        else if (!V_stricmp(argv[i], "-ignoreerrors"))
        {
            g_ignoreerrors = true;
        }
        else if (!V_stricmp(argv[i], "-nosteam"))
        {
            g_nosteam = true;
        }
        else if (!V_stricmp(argv[i], "-spewallcommands"))
        {
            g_spewallcommands = true;
        }          
        else if (!V_stricmp(argv[i], "-compileverbose"))
        {
            g_compileverbose = true;
        }  
        else if (!V_stricmp(argv[i], "-spewallverbose"))
        {
            verbose = true;
            g_spewallcommands = true;
            g_compileverbose = true;
        }       
        else if (!V_stricmp(argv[i], "-quiet"))
        {
            g_quiet = true;
            verbose = false;
            g_spewallcommands = false;
            g_infocontent = false;
        }
        else if (!V_stricmp(argv[i], "-addonbuild"))
        {
            g_addonbuild = true;
            g_buildcaption = false;
            g_buildscene = false;
            g_buildvpk = false;
        }      
#ifdef MP_ADDON_SUPPORT
        else if (!V_stricmp(argv[i], "-mp_addon"))
        {
            g_mp_addon = true;
        }
#endif // MP_ADDON_SUPPORT
        else if (!V_stricmp(argv[i], "-skipmaterial"))
        {
            g_buildmaterials = false;
        }
        else if (!V_stricmp(argv[i], "-skipmodel"))
        {
            g_buildmodels = false;
        }
        else if (!V_stricmp(argv[i], "-skipsound"))
        {
            g_buildsounds = false;
        }
        else if (!V_stricmp(argv[i], "-skipscene"))
        {
            g_buildscene = false;
        }
        else if (!V_stricmp(argv[i], "-skipcaption"))
        {
            g_buildcaption = false;
        }
        else if (!V_stricmp(argv[i], "-skipmap"))
        {
            g_buildmap = false;
        }
        else if (!V_stricmp(argv[i], "-vpk"))
        {
            g_buildvpk = true;
        }
        else if (!V_stricmp(argv[i], "-game") || !V_stricmp(argv[i], "-vproject"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* gamePath = argv[i];
                if (!gamePath)
                {
                    Error("Error: \'-game\' requires a valid path argument. NULL path\n");
                }
                V_strcpy(gamedir, gamePath);
            }
            else
            {
                Error("Error: \'-game\' requires a valid path argument.\n");
            }
        }
        else if (!V_stricmp(argv[i], "-steamgamedir"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                const char* gamePath = argv[i];
                if (!gamePath)
                {
                    Error("\nError: \'-steamgamedir\' requires a valid path argument. NULL path\n");
                }
                V_strcpy(g_steamdir, gamePath);
            }
            else
            {
                Error("\nError: \'-steamgamedir\' requires a valid path argument.\n");
            }
        }
        else
        {
            Shared::qWarning("\nWarning Unknown option \'%s\'\n", argv[i]);
            PrintUsage(argc, argv);
        }
    }
    Msg("\n");
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    float start = Plat_FloatTime();

    SetupDefaultToolsMinidumpHandler();
    CommandLine()->CreateCmdLine(argc, argv);
    InstallSpewFunction();
    PrintHeader();
	ParseCommandline(argc, argv);
    CmdLib_InitFileSystem(gamedir);
    PreInit();
    Init_AssetTools();

    if (g_buildcontent || g_addonbuild)
    {
        if (g_buildmaterials)
        {
            MaterialBuilder::MaterialCompile();
        }
        if (g_buildmodels)
        {
            ModelBuilder::ModelCompile();
        }
        if (g_buildsounds)
        {
            SoundBuilder::SoundCompile();
        }
        if (g_buildscene)
        {
            SceneBuilder::SceneCompile();
        }
        if (g_buildcaption)
        {
            CaptionBuilder::CaptionCompile();
        }
        if (g_buildmap)
        {
            MapBuilder::MapCompile();
        }
        if (g_buildvpk)
        {
            VpkBuilder::VpkCompile();
        }
#ifdef MP_ADDON_SUPPORT
        if(g_mp_addon && g_addonbuild && !g_infocontent)
        {
            AddonBuilder::AddonCompile();
        }
#endif // MP_ADDON_SUPPORT
        Msg("\n-------------------------------------------------------------------------------------------\n");
        Msg(" AssetCompile -> Done in %s | ", Shared::TimeStamp());
        ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "Completed: %llu,     ", g_process_completed);
        ColorSpewMessage(SPEW_MESSAGE, &red, "Error: %llu,     ", g_process_error);
        ColorSpewMessage(SPEW_MESSAGE, &yellow, "Skipped: %llu         ", GetSkippedAssets());
        Msg("\n-------------------------------------------------------------------------------------------\n");
        Msg("\n");
    }

    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(1);

    HitKeyToContinue();
	
    return 0;
}