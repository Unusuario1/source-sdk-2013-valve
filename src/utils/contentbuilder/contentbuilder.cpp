#include <windows.h>
#include "stdlib.h"
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "loadcmdline.h"
#include "cmdlib.h"
#include "filesystem_init.h"

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


// TODO: setup a log funtion also a warning, error, smth like _build/build(date, time).log
//                                                            _build/warning.contentlist
//                                                            _build/asset_report.contentlist
//                                                            _build/build_manifiest.contentlist //This is internal so we dont make a full if not needed build!


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
char g_gamedir[MAX_PATH]    = "";       //  game/mod "-game"
char g_gamebin[MAX_PATH]    = "";       //  game/bin or game/bin/x64
char g_steamdir[MAX_PATH]   = "";       //  game
char g_gameinfodir[MAX_PATH] = "";      //  game/mod/gameinfo.txt


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
        i += Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION1);
        i += Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION2);
        i += Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION3);
        i += Shared::CountAssets(g_gamedir, MATERIALS_EXTENSION);
    }
    if (g_buildmodels)
    {
        i += Shared::CountAssets(g_gamedir, MODELSRC_EXTENSION);
    }
    if (g_buildsounds)
    {
        i += Shared::CountAssets(g_gamedir, SOUNDSRC_EXTENSION);
    }
    if (g_buildscene)
    {
        i += Shared::CountAssets(g_gamedir, SCENESRC_EXTENSION);
    }
    if (g_buildcaption)
    {
        i += Shared::CountAssets(g_gamedir, CAPTIONSRC_EXTENSION);
    }
    if (g_buildmap)
    {
        i += Shared::CountAssets(g_gamedir, MAPSRC_EXTENSION);
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
        i += Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION1);
        i += Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION2);
        i += Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION3);
        i += Shared::CountAssets(g_gamedir, MATERIALS_EXTENSION);
    }
    if (!g_buildmodels)
    {
        i += Shared::CountAssets(g_gamedir, MODELSRC_EXTENSION);
    }
    if (!g_buildsounds)
    {
        i += Shared::CountAssets(g_gamedir, SOUNDSRC_EXTENSION);
    }
    if (!g_buildscene)
    {
        i += Shared::CountAssets(g_gamedir, SCENESRC_EXTENSION);
    }
    if (!g_buildcaption)
    {
        i += Shared::CountAssets(g_gamedir, CAPTIONSRC_EXTENSION);
    }
    if (!g_buildmap)
    {
        i += Shared::CountAssets(g_gamedir, MAPSRC_EXTENSION);
    }
    if (!g_buildvpk)
    {
        i += 1;
    }
    return i;
}


//-----------------------------------------------------------------------------
// Purpose:   Check if the tools are in bin or bin/x64
//-----------------------------------------------------------------------------
void Init_AssetTools()
{
    float start = Plat_FloatTime();
    char exclude[128] = "";

    g_timer = Plat_FloatTime(); 

    //Basic setup
    if (!g_nosteam)
    {
        FileSystem_GetAppInstallDir(g_steamdir, sizeof(g_steamdir));
        g_steamdir == NULL ? Error("AssetSystem -> gamedir is NULL! Check \'-game\' command!\n") : NULL;
    }

    g_gamedir == NULL ? Error("AssetSystem -> gamedir is NULL! Check \'-game\' command!\n") : NULL;

    Shared::SetUpBinDir(g_gamebin, sizeof(g_gamebin));
    V_snprintf(g_gameinfodir, sizeof(g_gamedir), "%s\\%s", g_gamedir, GAMEINFO);

    // Basic info
    Msg("\n");
    Msg("Working Directory:   %s\n", g_steamdir);
    Msg("Mod:                 %s\n", g_gamedir);
    Msg("Operation:           %s\n", g_buildcontent ? "Force-building content" : "");
    Msg("Verbosity:           %s %s\n", (verbose ? "High" : "Standard"), (g_spewallcommands ? "(showing paths)" : ""));
    Msg("Building:            %s\n", g_addonbuild ? "Partial (addon)" : 
        (g_buildmaterials && g_buildmodels && g_buildsounds && g_buildscene && g_buildcaption && g_buildmap && g_buildvpk) ? "Full (game)" : "Partial (game)"); // lol...
    
    if (!g_buildmaterials)        V_strcat(exclude, "(materials) ", sizeof(exclude));
    else if (!g_buildmodels)      V_strcat(exclude, "(models) ", sizeof(exclude));
    else if (!g_buildsounds)      V_strcat(exclude, "(sounds) ", sizeof(exclude));
    else if (!g_buildscene)       V_strcat(exclude, "(scene) ", sizeof(exclude));
    else if (!g_buildcaption)     V_strcat(exclude, "(caption) ", sizeof(exclude));
    else if (!g_buildmap)         V_strcat(exclude, "(maps) ", sizeof(exclude));
    else if (!g_buildvpk)         V_strcat(exclude, "(vpk) ", sizeof(exclude));
    else                          V_strcat(exclude, "None ", sizeof(exclude));
    Msg("Excluding:           %s\n\n", exclude);

    if (g_spewallcommands)
    {
        //General paths
        ColorSpewMessage(SPEW_MESSAGE, &header_color, "\nGeneral paths:\n");
        Msg("\tPath - GameInfo.txt: ");             ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gameinfodir);
        Msg("\tPath - Game binary (tools): ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gamebin);

        //Game/Mod source assets (src) paths
        ColorSpewMessage(SPEW_MESSAGE, &header_color, "Game/Mod Source/Compiled assets paths:\n");
        Msg("\tContent Source - Materials: ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, MATERIALSRC_DIR);
        Msg("\tContent Source - Models: ");         ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, MODELSRC_DIR);
        Msg("\tContent Source - Sounds: ");         ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, SOUNDSRC_DIR);
        Msg("\tContent Source - Scene: ");          ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, SCENE_DIR);
        Msg("\tContent Source - Caption: ");        ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, CAPTIONSRC_DIR);
        Msg("\tContent Source - Maps: ");           ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, MAPSRC_DIR);
        Msg("\tContent Source - Valve Pack File (vpk): ");  ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gamedir);
        Msg("\tContent Compiled - Materials: ");    ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, MATERIALS_DIR);
        Msg("\tContent Compiled - Models: ");       ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, MODELS_DIR);
        Msg("\tContent Compiled - Sounds: ");       ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, SOUNDS_DIR);
        Msg("\tContent Compiled - Scene: ");        ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, SCENE_DIR);
        Msg("\tContent Compiled - Caption: ");      ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, CAPTION_DIR);
        Msg("\tContent Compiled - Maps: ");         ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\\%s\"\n", g_gamedir, MAPS_DIR);
        Msg("\tContent Compiled - Valve Pack File (vpk): ");  ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", g_gamedir);
    }

    //Create asset compiled dir
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &header_color : &white, "Initializing AssetSystem... %s", g_spewallcommands ? "\n" : "");
    Shared::CreateAssetSystemGamePath(g_gamedir, MATERIALS_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, MODELS_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, SOUNDS_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, SCENE_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, CAPTION_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, MAPS_DIR);
    if (g_spewallcommands)
    {
        //scan source assets
        Msg("\tAssetSystem -> Materials: %llu count\n", Shared::CountAssets(g_gamedir, MATERIALS_EXTENSION));
        Msg("\tAssetSystem -> Textures:  %llu count\n", Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION1) +
                                                        Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION2) +
                                                        Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION3));
        Msg("\tAssetSystem -> Models:    %llu count\n", Shared::CountAssets(g_gamedir, MODELSRC_EXTENSION));
        Msg("\tAssetSystem -> Sounds:    %llu count\n", Shared::CountAssets(g_gamedir, SOUNDSRC_EXTENSION));
        Msg("\tAssetSystem -> Scene:     %llu count\n", Shared::CountAssets(g_gamedir, SCENESRC_EXTENSION));
        Msg("\tAssetSystem -> Caption:   %llu count\n", Shared::CountAssets(g_gamedir, CAPTIONSRC_EXTENSION));
        Msg("\tAssetSystem -> Maps       %llu count\n", Shared::CountAssets(g_gamedir, MAPSRC_EXTENSION));
    }
    ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "Done in %.2f seconds.\n", Plat_FloatTime() - start);

    //Tools paths
    ColorSpewMessage(SPEW_MESSAGE, g_spewallcommands ? &header_color : &white, "Initializing AssetTools systems (%s)... %s", Shared::TargetPlatform() ? "64 bits" : "32 bits", g_spewallcommands ? "\n" : ""); // I know....
    MaterialBuilder::AssetToolCheck(g_gamebin);
    ModelBuilder::AssetToolCheck(g_gamebin);
    SoundBuilder::AssetToolCheck(g_gamebin);
    SceneBuilder::AssetToolCheck(g_gamebin);
    CaptionBuilder::AssetToolCheck(g_gamebin);
    MapBuilder::AssetToolCheck(g_gamebin);
    VpkBuilder::AssetToolCheck(g_gamebin);
    ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "Done in %.2f seconds. All sub systems checks passed!\n", Plat_FloatTime() - start);

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
void PrintUsage()
{
    // Add somewhere a -steamBuild
    Msg("\nUsage: contentbuilder.exe [options] -game <path>\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " General Options:\n");
    Msg("   -b:                    Build the content.\n"
        "   -pause:                After building the content, pause the program.\n"
        "   -info:                 Only prints all the assets to be compiled (no compile).\n"
        "   -forcedirty:           Do not remove temp content.\n"
        "   -toolsforce32bits:     Force contentbuilder to use 32 bits tools.\n"
        "   -toolsforce64bits:     Force contentbuilder to use 64 bits tools.\n"
        "   -ignoreerrors:         Ignores errors unless is strictly necesary.\n"
        "                          (NOT recomended for steam builds, only for testing and fast builds)\n"
        "   -nosteam:              Dont use steam api mount funtions.\n"
        "                          Use when \'-game\' path is outside the steam game path.\n"
        "                          Useful for no steam mods builds, addon builds outside the game (steam) path.\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Building Options:\n");
    Msg("   -addonbuild:           Builds only maps, materials, models and sounds\n"
        "   -skipmaterial:         Skips texture (%s) compile\n"
        "   -skipmodel:            Skips model (%s) compile\n"
        "   -skipsound:            Skips sound (%s) compile\n"
        "   -skipscene:            Skips scene (%s) compile\n"
        "   -skipcaption:          Skips caption (%s) compile\n"
        "   -skipmap:              Skips maps (%s) compile\n"
        "   -vpk:                  Generate vpk files (Not avaible for \'-addonbuild\').\n"
        "   -game <path>           Specify the folder of the gameinfo.txt file.\n"
        "   -vproject <path>:      Same as \'-game\'\n"
        "   -steamgamedir <path>:  if \'-nosteam\' is enabled, loads the steam game path,\n"
        "                          (e.g: \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Half-life 2\")\n"
        , MATERIALSRC_DIR, MODELSRC_DIR, SOUNDSRC_DIR, SCENESRC_DIR, CAPTIONSRC_DIR, MAPSRC_DIR);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Spew Options:\n");
    Msg("   -v or -verbose:        Enables verbose\n"
        "   -spewallcommands:                     \n");
    //ColorSpewMessage(SPEW_MESSAGE, &header_color, " Advanced Build Options:\n");
    exit(-1);


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
        "  -exclude <ext>:        exclude resources of a certain type, and their children\n"
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
        "  -background:          run in 'background', lowest priority mode.\n"
        "  -failListFile <path>: append failed asset file paths to the specified text file.\n"
    );
    */
}


//-----------------------------------------------------------------------------
// Purpose:   Prints the header
//-----------------------------------------------------------------------------
void PrintHeader()
{
    Msg("\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, "//------------------------------------------------------------//\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, "// "                                                               );
    ColorSpewMessage(SPEW_MESSAGE, &white,            "Content builder (Build: %s %s)\n",           __DATE__, __TIME__);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, "//------------------------------------------------------------//\n");
}


//-----------------------------------------------------------------------------
// Purpose:   Parse command line
//-----------------------------------------------------------------------------
void ParseCommandline(int argc, char* argv[])
{
    //Msg("Command line: %s\n", *CommandLine()->GetParms());

    for (int i = 1; i < argc; ++i)
    {
        if (!V_stricmp(argv[i], "-?") || !V_stricmp(argv[i], "-help") || argc == 1)
        {
            PrintUsage();
        }
        else if (!V_stricmp(argv[i], "-v") || !V_stricmp(argv[i], "-verbose"))
        {
            verbose = true;
        }
        else if (!V_stricmp(argv[i], "-b"))
        {
            g_buildcontent = true;
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
                const char* gamePath = argv[i];
                if (!gamePath)
                {
                    Error("Error: -game requires a valid path argument. NULL path\n");
                }
                V_snprintf(g_gamedir, sizeof(g_gamedir), "%s", gamePath);
            }
            else
            {
                Error("Error: -game requires a valid path argument.\n");
            }
        }
        else if (!V_stricmp(argv[i], "-steamgamedir"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                const char* gamePath = argv[i];
                if (!gamePath)
                {
                    Error("Error: -steamgamedir requires a valid path argument. NULL path\n");
                }
                V_snprintf(g_steamdir, sizeof(g_steamdir), "%s", gamePath);
            }
            else
            {
                Error("Error: -steamgamedir requires a valid path argument.\n");
            }
        }
        else
        {
            Warning("\nWarning: Unknown option '%s'\n", argv[i]);
            PrintUsage();
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    float start = Plat_FloatTime();

    InstallSpewFunction();
    PrintHeader();
	CommandLine()->CreateCmdLine(argc, argv);
	ParseCommandline(argc, argv);
    Init_AssetTools();

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

        Msg("\n+------------------------------------------------------------------------------------------\n");
        Msg("| AssetCompile -> Done in %d seconds | ", (int)(Plat_FloatTime() - start));
        ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "Completed: %llu     ", g_process_completed);
        ColorSpewMessage(SPEW_MESSAGE, &red, "Error: %llu     ", g_process_error);
        ColorSpewMessage(SPEW_MESSAGE, &yellow, "Skipped: %llu         ", GetSkippedAssets());
        Msg("\n+------------------------------------------------------------------------------------------\n\n");
    }

    HitKeyToContinue();

	return 0;
}