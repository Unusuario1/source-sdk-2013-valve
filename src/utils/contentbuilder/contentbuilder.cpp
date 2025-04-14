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
#include "colorscheme.h"
#include "materialbuilder.h"
#include "modelbuilder.h"
#include "soundbuilder.h"
#include "scenebuilder.h"
#include "captionbuilder.h"
#include "mapbuilder.h"
#include "vpkbuilder.h"
#include "shared.h"


// TODO: setup a log funtion also a warning, error, smth like _build/build(date time).log
//                                                            _build/warning.contentlist
//                                                            _build/asset_report.contentlist

//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
std::size_t g_process_completed = 0;
std::size_t g_process_error = 0;
std::size_t g_timer         = 0;        // global timer for assets!
bool g_force32bits          = PLATFORM_64BITS ? true : false;
bool g_force64bits          = PLATFORM_64BITS ? true : false;
bool g_cleanuptempcontent   = true;     //Do we cleanup temp content?
bool g_infocontent          = false;    //Dont build only print assets
bool g_nosteam              = false;    //No steam funtions
bool g_addonbuild           = false;    //Build the -game dir as addon instead of a full game
bool g_buildcontent         = false;    //Build content?
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
bool g_quiet                = false;
bool g_createlog            = true;
char g_gamebin[MAX_PATH]    = "";       //  game/bin or game/bin/x64
char g_steamdir[MAX_PATH]   = "";       //  game
char g_gameinfodir[MAX_PATH] = "";      //  game/mod/gameinfo.txt
char g_contentbuilderPath[MAX_PATH] = ""; // game/mod/_build


//-----------------------------------------------------------------------------
// Purpose:   Wait until the user wants to exit the program
//-----------------------------------------------------------------------------
void HitKeyToContinue()
{
    g_pause ? system("pause") : void();
}


//-----------------------------------------------------------------------------
// Purpose:  Count how many assets we have to compile
//-----------------------------------------------------------------------------
std::size_t CountAllAssets()
{
    std::size_t i = 0;
    if (g_buildmaterials)
    {
        i += Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION1);
        i += Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION2);
        i += Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION3);
        i += Shared::CountAssets(gamedir, MATERIALS_EXTENSION);
    }
    if (g_buildmodels)
    {
        i += Shared::CountAssets(gamedir, MODELSRC_EXTENSION);
    }
    if (g_buildsounds)
    {
        i += Shared::CountAssets(gamedir, SOUNDSRC_EXTENSION);
    }
    if (g_buildscene)
    {
        i += Shared::CountAssets(gamedir, SCENESRC_EXTENSION);
    }
    if (g_buildcaption)
    {
        i += Shared::CountAssets(gamedir, CAPTIONSRC_EXTENSION);
    }
    if (g_buildmap)
    {
        i += Shared::CountAssets(gamedir, MAPSRC_EXTENSION);
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
        i += Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION1);
        i += Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION2);
        i += Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION3);
        i += Shared::CountAssets(gamedir, MATERIALS_EXTENSION);
    }
    if (!g_buildmodels)
    {
        i += Shared::CountAssets(gamedir, MODELSRC_EXTENSION);
    }
    if (!g_buildsounds)
    {
        i += Shared::CountAssets(gamedir, SOUNDSRC_EXTENSION);
    }
    if (!g_buildscene)
    {
        i += Shared::CountAssets(gamedir, SCENESRC_EXTENSION);
    }
    if (!g_buildcaption)
    {
        i += Shared::CountAssets(gamedir, CAPTIONSRC_EXTENSION);
    }
    if (!g_buildmap)
    {
        i += Shared::CountAssets(gamedir, MAPSRC_EXTENSION);
    }
    if (!g_buildvpk)
    {
        i += 1;
    }
    return i;
}


//-----------------------------------------------------------------------------
// Purpose:     Add MOD search path to the filesystem
//-----------------------------------------------------------------------------
void PreInit()
{
    // We dont care about compiled assets or search paths
    g_pFullFileSystem->RemoveAllSearchPaths();
    g_pFullFileSystem->AddSearchPath("MOD", gamedir);

    // Note: Even though it's standard to add a '\' at the end of the string,
    // we remove it here because it makes managing paths much easier across the tool.
    V_StripTrailingSlash(gamedir);
}


//-----------------------------------------------------------------------------
// Purpose:   Create the log file   //TODO: do the same for warning an errors!!
//-----------------------------------------------------------------------------
void SetUpLogFile()
{
    char logFile[MAX_PATH];

    V_snprintf(g_contentbuilderPath, sizeof(g_contentbuilderPath), "%s\\%s", gamedir, CONTENTBUILDER_OUTPATH);

    // Remove old files
    if (Shared::CheckIfFileExist(g_contentbuilderPath) && g_cleanuptempcontent)
    {
        if (remove(g_contentbuilderPath) != 0)
        {
            Warning("\nCould not remove old build files in \"%s\"!\n", g_contentbuilderPath);
        }
    }

    if(!Shared::CreateDirectoryRecursive(g_contentbuilderPath))
    {
      DWORD err = GetLastError();
      if (err != ERROR_ALREADY_EXISTS)
      {
          Shared::qError("\nCould not create temporary directory at: \"%s\" (Error code: %lu)\n", g_contentbuilderPath, err);
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

    if(g_infocontent)
    {
        // asset report!

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
        if (g_steamdir == NULL) 
        {
            Error("AssetSystem -> steam game dir is NULL! Check \'-steamgamedir\' command!\n");
        }
    }

    assert(gamedir == NULL ? Error("AssetSystem -> gamedir is NULL! Check \'-game\' command!\n") : NULL);

    Shared::SetUpBinDir(g_gamebin, sizeof(g_gamebin));
    V_snprintf(g_gameinfodir, sizeof(g_gameinfodir), "%s\\%s", gamedir, GAMEINFO);

    SetUpLogFile();

    // Basic info
    Msg("\n");
    Msg("Working Directory:   %s\n", g_steamdir);
    Msg("Mod:                 %s\n", gamedir);
    Msg("Operation:           %s\n", g_buildcontent && !g_buildoutofdatecontent ? "Force-building content" : "Partial-building content");
    Msg("Verbosity:           %s %s\n", (verbose ? "High" : (g_quiet ? "Quiet" : "Standard")), (g_spewallcommands ? "(showing paths)" : ""));
    Msg("Building:            %s\n", g_addonbuild ? "Partial (addon)" : (g_buildmaterials && g_buildmodels && g_buildsounds && 
                                                                         g_buildscene && g_buildcaption && g_buildmap &&
                                                                         g_buildvpk) ? "Full (game)" : "Partial (game)"); // lol...
    
    if (!g_buildmaterials)   V_strcat(szExclude, "(materials) ", sizeof(szExclude));
    if (!g_buildmodels)      V_strcat(szExclude, "(models) ", sizeof(szExclude));
    if (!g_buildsounds)      V_strcat(szExclude, "(sounds) ", sizeof(szExclude));
    if (!g_buildscene)       V_strcat(szExclude, "(scene) ", sizeof(szExclude));
    if (!g_buildcaption)     V_strcat(szExclude, "(caption) ", sizeof(szExclude));
    if (!g_buildmap)         V_strcat(szExclude, "(maps) ", sizeof(szExclude));
    if (!g_buildvpk)         V_strcat(szExclude, "(vpk) ", sizeof(szExclude));
    else                     V_strcat(szExclude, "None ", sizeof(szExclude));
    Msg("Excluding:           %s\n\n", szExclude);

    if (g_spewallcommands)
    {
        //General paths
        ColorSpewMessage(SPEW_MESSAGE, &header_color, "\nGeneral paths:\n");
        Msg("\tPath - GameInfo.txt: ");             ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gameinfodir);
        Msg("\tPath - Game binary (tools): ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gamebin);

        //Game/Mod source assets (src) paths
        ColorSpewMessage(SPEW_MESSAGE, &header_color, "Game/Mod Source/Compiled assets paths:\n");
        Msg("\tContent Source - Materials: ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MATERIALSRC_DIR);
        Msg("\tContent Source - Models: ");         ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MODELSRC_DIR);
        Msg("\tContent Source - Sounds: ");         ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SOUNDSRC_DIR);
        Msg("\tContent Source - Scene: ");          ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SCENE_DIR);
        Msg("\tContent Source - Caption: ");        ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, CAPTIONSRC_DIR);
        Msg("\tContent Source - Maps: ");           ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MAPSRC_DIR);
        Msg("\tContent Source - Valve Pack File (vpk): ");  ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", gamedir);
        Msg("\tContent Compiled - Materials: ");    ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MATERIALS_DIR);
        Msg("\tContent Compiled - Models: ");       ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MODELS_DIR);
        Msg("\tContent Compiled - Sounds: ");       ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SOUNDS_DIR);
        Msg("\tContent Compiled - Scene: ");        ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, SCENE_DIR);
        Msg("\tContent Compiled - Caption: ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, CAPTION_DIR);
        Msg("\tContent Compiled - Maps: ");         ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", gamedir, MAPS_DIR);
        Msg("\tContent Compiled - Valve Pack File (vpk): ");  ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", gamedir);
    }

    //Create asset compiled dir
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &header_color : &white, "Initializing AssetSystem... %s", g_spewallcommands ? "\n" : "");
    Shared::CreateAssetSystemGamePath(gamedir, MATERIALS_DIR);
    Shared::CreateAssetSystemGamePath(gamedir, MODELS_DIR);
    Shared::CreateAssetSystemGamePath(gamedir, SOUNDS_DIR);
    Shared::CreateAssetSystemGamePath(gamedir, SCENE_DIR);
    Shared::CreateAssetSystemGamePath(gamedir, CAPTION_DIR);
    Shared::CreateAssetSystemGamePath(gamedir, MAPS_DIR);
    if (g_spewallcommands)
    {
        //scan source assets
        Msg("\tAssetSystem -> Materials: %llu count\n", Shared::CountAssets(gamedir, MATERIALS_EXTENSION));
        Msg("\tAssetSystem -> Textures:  %llu count\n", Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION1) +
                                                        Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION2) +
                                                        Shared::CountAssets(gamedir, TEXTURESRC_EXTENSION3));
        Msg("\tAssetSystem -> Models:    %llu count\n", Shared::CountAssets(gamedir, MODELSRC_EXTENSION));
        Msg("\tAssetSystem -> Sounds:    %llu count\n", Shared::CountAssets(gamedir, SOUNDSRC_EXTENSION));
        Msg("\tAssetSystem -> Scene:     %llu count\n", Shared::CountAssets(gamedir, SCENESRC_EXTENSION));
        Msg("\tAssetSystem -> Caption:   %llu count\n", Shared::CountAssets(gamedir, CAPTIONSRC_EXTENSION));
        Msg("\tAssetSystem -> Maps       %llu count\n", Shared::CountAssets(gamedir, MAPSRC_EXTENSION));
    }
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &sucesfullprocess_color : &done_color, "Done in %.2f seconds.\n", Plat_FloatTime() - start);

    //Tools paths
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &header_color : &white, "Initializing AssetTools systems (%s)... %s", Shared::TargetPlatform() ? "64 bits" : "32 bits", g_spewallcommands ? "\n" : ""); // I know....
    MaterialBuilder::AssetToolCheck(g_gamebin);
    ModelBuilder::AssetToolCheck(g_gamebin);
    SoundBuilder::AssetToolCheck(g_gamebin);
    SceneBuilder::AssetToolCheck(g_gamebin);
    CaptionBuilder::AssetToolCheck(g_gamebin);
    MapBuilder::AssetToolCheck(g_gamebin);
    VpkBuilder::AssetToolCheck(g_gamebin);
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &sucesfullprocess_color : &done_color, "Done in %.2f seconds. All sub systems checks passed!\n", Plat_FloatTime() - start);

    Msg("Finalizing build list... \n");
    std::size_t uCountAsset = CountAllAssets();
    Msg("%llu assets need to be built\n\n", uCountAsset);
    
    // Sanity check
    if(uCountAsset == 0)
    {
        Warning("No assets to %s!\n", g_infocontent ? "print" : "build");
        exit(0);
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Print contentbuilder usage
//-----------------------------------------------------------------------------
void PrintUsage(int argc, char* argv[])
{
    // Add somewhere a -steamBuild
    Msg("\nUsage: contentbuilder.exe [options] -game <path>\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " General Options:\n");
    Msg("   -b:                    Build the content.\n"
        "   -lb:                   Build only out-of-date content.\n"
        "   -pause:                After building the content, pause the program.\n"
        "   -info:                 Only prints all the assets to be compiled (no compile).\n"
        "   -forcedirty:           Do not remove temp content.\n"
        "   -ignoreerrors:         Ignores errors unless is strictly necesary.\n"
        "                          (NOT recomended for steam builds, only for testing and fast builds)\n"
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
        "   -skipmaterial:         Skips texture (\\%s) compile.\n"
        "   -skipmodel:            Skips model (\\%s) compile.\n"
        "   -skipsound:            Skips sound (\\%s) compile.\n"
        "   -skipscene:            Skips scene (\\%s) compile.\n"
        "   -skipcaption:          Skips caption (\\%s) compile.\n"
        "   -skipmap:              Skips maps (\\%s) compile.\n"
        "   -vpk:                  Generate vpk files (Not avaible for \'-addonbuild\').\n"
        "\n"
        , MATERIALSRC_DIR, MODELSRC_DIR, SOUNDSRC_DIR, SCENESRC_DIR, CAPTIONSRC_DIR, MAPSRC_DIR);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Spew Options:\n");
    Msg("   -v or -verbose:        Enables verbose.\n"
        "   -quiet:                Prints minimal text. (Note: Disables \'-verbose\' and \'spewallcommands\')\n"
        "   -spewallcommands:                     \n"
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Advanced Build Options:\n");
    Msg("   -toolsforce32bits:     Force contentbuilder to use 32 bits tools.\n"
        "   -toolsforce64bits:     Force contentbuilder to use 64 bits tools.\n"
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Other Options:\n");
    Msg("   -FullMinidumps:        Write large minidumps on crash.\n"
        "\n");

    DeleteCmdLine(argc, argv);
    CmdLib_Exit(1);

    /*
        Msg(" Building Options:\n");
    Msg(
        "  -b:                   build the content\n"
        "  -checkup:             clean files (remove forcedirty) and perform content checkup.\n\n"
        "  -contentcleanup       cleans up the game/mod from compile files!\n" 
    );
    Msg(" Spew Options:\n");
    Msg(
        "  -spewallcommands:     print the command line and working directory for each compile.\n"
    );
    Msg(" General Options:\n");
    Msg(
        "  -path <path>:          only build content in the specified path.\n"
        "  -ignoreerrors:         Ignores errors and doesnt stop content build.\n"
        "  -v:                    high level of verbosity\n"
        "  -include <ext>:        consider only specific resources for the operation.\n"
        "  -szExclude <ext>:        szExclude resources of a certain type, and their children\n"
        "  -sharedrefsonly <ext>: restrict -lsr behavior to only consider references from the specified type.\n"
    );
    Msg(" Lists (*.contentlist):\n");
    Msg(
        "  -greylist <file>:     failure to compile any of this content is a warning, not an error.\n"
        "  -includelist <file>:  add the assets in this file to the build list.\n"
    );
    Msg(" Advanced Build Options:\n");
    Msg(
        "  -proc <num>:          max simultaneous compile processes (default: 8). (Thread number) (this is disaled for maps and vpk)\n"
    );
    */
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
        else if (!V_stricmp(argv[i], "-quiet"))
        {
            g_quiet = true;
            verbose = false;
            g_spewallcommands = false;
        }
        else if (!V_stricmp(argv[i], "-addonbuild"))
        {
            g_addonbuild = true;
            g_buildcaption = false;
            g_buildvpk = false;
            g_buildscene = true;
        }
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
            Warning("Warning: Unknown option \'%s\'\n", argv[i]);
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

    // TODO: add here multi thread bull shit for 4x compile! mat, mod, sound, sce, capt
    if (g_buildcontent || g_addonbuild)
    {
        if (g_buildmaterials)
        {
            MaterialBuilder::MaterialCompile(g_gamebin, sizeof(g_gamebin), g_process_completed, g_process_error);
        }
        if (g_buildmodels)
        {
            ModelBuilder::ModelCompile(g_gamebin, sizeof(g_gamebin), g_process_completed, g_process_error);
        }
        if (g_buildsounds)
        {
            SoundBuilder::SoundCompile(g_gamebin, sizeof(g_gamebin), g_process_completed, g_process_error);
        }
        if (g_buildscene)
        {
            SceneBuilder::SceneCompile(g_gamebin, sizeof(g_gamebin), g_process_completed, g_process_error);
        }
        if (g_buildcaption)
        {
            CaptionBuilder::CaptionCompile(g_gamebin, sizeof(g_gamebin), g_process_completed, g_process_error);
        }
        if (g_buildmap)
        {
            MapBuilder::MapCompile(g_gamebin, sizeof(g_gamebin), g_process_completed, g_process_error);
        }
        if (g_buildvpk)
        {
            VpkBuilder::VpkCompile(g_gamebin, sizeof(g_gamebin), g_process_completed, g_process_error);
        }

        Msg("\n-------------------------------------------------------------------------------------------\n");
        Msg("  AssetCompile -> Done in %s | ", Shared::TimeStamp());
        ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "Completed: %llu,     ", g_process_completed);
        ColorSpewMessage(SPEW_MESSAGE, &red, "Error: %llu,     ", g_process_error);
        ColorSpewMessage(SPEW_MESSAGE, &yellow, "Skipped: %llu         ", GetSkippedAssets());
        Msg("\n-------------------------------------------------------------------------------------------\n\n");
    }

    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();

    HitKeyToContinue();
	return 0;
}