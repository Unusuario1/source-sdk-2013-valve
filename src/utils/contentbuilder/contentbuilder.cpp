#include <windows.h>
#include "stdlib.h"
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "loadcmdline.h"
#include "cmdlib.h"
#include "filesystem_init.h"

#include "contentbuilder.h"
#include "materialbuilder.h"
#include "modelbuilder.h"
#include "soundbuilder.h"
#include "scenebuilder.h"
#include "captionbuilder.h"
#include "mapbuilder.h"
#include "vpkbuilder.h"
#include "shared.h"


//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
std::size_t process_completed = 0;
std::size_t process_error = 0;
bool g_force32bits = PLATFORM_64BITS ? true : false;
bool g_force64bits = PLATFORM_64BITS ? true : false;
bool g_nosteam = false;
bool g_addonbuild = false;
bool g_buildcontent = false;
bool g_buildmaterials = true;
bool g_buildmodels = true;
bool g_buildsounds = true;
bool g_buildscene = true;
bool g_buildcaption = true;
bool g_buildmap = true;
bool g_buildvpk = true;
bool g_ignoreerrors = false;
bool g_pause = false;
char g_gamedir[MAX_PATH] = "";      //  game/mod "-game"
char g_gamebin[MAX_PATH] = "";      //  game/bin or game/bin/x64
char g_steamdir[MAX_PATH] = "";     //  game
char g_gameinfodir[MAX_PATH] = "";  //  game/mod/gameinfo.txt


void HitKeyToContinue()
{
    if (g_pause)
    {
        system("pause");
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Get how many skip build we got
//-----------------------------------------------------------------------------
std::size_t GetSkippedAssets()
{
    std::size_t i = 0;
    if (!g_buildmaterials)
    {
        i += Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION);
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
    return i;
}


//-----------------------------------------------------------------------------
// Purpose:   Check if the tools are in bin or bin/x64 ADD MAPBUILDER!!!
//-----------------------------------------------------------------------------
void Init_AssetTools()
{
    float start, end;

    start = Plat_FloatTime();

    //Basic setup
    if (!g_nosteam) 
    {
        FileSystem_GetAppInstallDir(g_steamdir, sizeof(g_steamdir));
    }

    Shared::SetUpBinDir(g_gamebin, sizeof(g_gamebin));
    V_snprintf(g_gameinfodir, sizeof(g_gamedir), "%s\\%s", g_gamedir, GAMEINFO);

    if(!Shared::DirectoryExists(g_gamedir))
    {
        Shared::qError("Game dir does not exist, type a valid dir in \'-game\'!");
        exit(-1);
    }

    //General paths
    Msg("\nGeneral paths:\n");
    Msg("\tPath - Install directory: \"%s\"\n", g_steamdir);
    Msg("\tPath - Game directory: \"%s\"\n", g_gamedir);
    Msg("\tPath - GameInfo.txt: \"%s\"\n", g_gameinfodir);
    Msg("\tPath - Game binary (tools): \"%s\"\n", g_gamebin);

    //Game/Mod source assets (src) paths
    Msg("Game/Mod Source/Compiled assets paths:\n");
    Msg("\tContent Source - Materials: \"%s\\%s\"\n", g_gamedir, MATERIALSRC_DIR);
    Msg("\tContent Source - Models: \"%s\\%s\"\n", g_gamedir, MODELSRC_DIR);
    Msg("\tContent Source - Sounds: \"%s\\%s\"\n", g_gamedir, SOUNDSRC_DIR);
    Msg("\tContent Source - Scene: \"%s\\%s\"\n", g_gamedir, SCENE_DIR);
    Msg("\tContent Source - Caption: \"%s\\%s\"\n", g_gamedir, CAPTIONSRC_DIR);
    Msg("\tContent Source - Maps: \"%s\\%s\"\n", g_gamedir, MAPSRC_DIR);
    Msg("\tContent Source - Valve Pack File (vpk): \"%s\"\n", g_gamedir);
    Msg("\tContent Compiled - Models: \"%s\\%s\"\n", g_gamedir, MODELS_DIR);
    Msg("\tContent Compiled - Sounds: \"%s\\%s\"\n", g_gamedir, SOUNDS_DIR);
    Msg("\tContent Compiled - Scene: \"%s\\%s\"\n", g_gamedir, SCENE_DIR);
    Msg("\tContent Compiled - Caption: \"%s\\%s\"\n", g_gamedir, CAPTION_DIR);
    Msg("\tContent Compiled - Maps: \"%s\\%s\"\n", g_gamedir, MAPS_DIR);
    Msg("\tContent Compiled - Valve Pack File (vpk): \"%s\"\n", g_gamedir);

    //Create asset compiled dir
    Msg("Initializing AssetSystem:\n");
    Shared::CreateAssetSystemGamePath(g_gamedir, MATERIALS_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, MODELS_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, SOUNDS_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, SCENE_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, CAPTION_DIR);
    Shared::CreateAssetSystemGamePath(g_gamedir, MAPS_DIR);
    
    //scan source assets
    Msg("\tAssetSystem -> Materials: %llu count\n", Shared::CountAssets(g_gamedir, MATERIALS_EXTENSION));
    Msg("\tAssetSystem -> Textures: %llu count\n", Shared::CountAssets(g_gamedir, TEXTURESRC_EXTENSION));
    Msg("\tAssetSystem -> Models: %llu count\n", Shared::CountAssets(g_gamedir, MODELSRC_EXTENSION));
    Msg("\tAssetSystem -> Sounds: %llu count\n", Shared::CountAssets(g_gamedir, SOUNDSRC_EXTENSION));
    Msg("\tAssetSystem -> Scene: %llu count\n", Shared::CountAssets(g_gamedir, SCENESRC_EXTENSION));
    Msg("\tAssetSystem -> Caption: %llu count\n", Shared::CountAssets(g_gamedir, CAPTIONSRC_EXTENSION));
    Msg("\tAssetSystem -> Maps: %llu count\n", Shared::CountAssets(g_gamedir, MAPSRC_EXTENSION));
    end = Plat_FloatTime();
    Msg("Done in %f seconds.\n", end - start);

    //Tools paths
    Msg("\nInitializing AssetTools systems (%s):\n", Shared::TargetPlatform() ? "64 bits" : "32 bits");
    MaterialBuilder::AssetToolCheck(g_gamebin);
    ModelBuilder::AssetToolCheck(g_gamebin);
    SoundBuilder::AssetToolCheck(g_gamebin);
    SceneBuilder::AssetToolCheck(g_gamebin);
    CaptionBuilder::AssetToolCheck(g_gamebin);
    //MapBuilder::AssetToolCheck(g_gamebin);
    VpkBuilder::AssetToolCheck(g_gamebin);
    end = Plat_FloatTime();
    Msg("Done in %f seconds. All sub systems checks passed!\n", end - start);
}


//-----------------------------------------------------------------------------
// Purpose:   Self explanatory
//-----------------------------------------------------------------------------
void PrintUsage()
{
    //Add somewhere a -steamBuild, -addonbuild, -nosteam 
    printf("\nUsage:");
    printf(" contentbuilder[options]\n\n");
    printf(" Building Options:\n");
    printf(
        "  -b:                   build the content\n"
        "  -checkup:             clean files (remove forcedirty) and perform content checkup.\n\n"
    );
//    printf(" Info Options:\n");
//    printf("  -info <substring>:    print info about all assets matching the specified substring (don't build)\n\n");
    printf(" Spew Options:\n");
    printf(
        "  -spewallcommands:     print the command line and working directory for each compile.\n"
        "  -spewallcompiles:     print the stdout from each compile (this is a LOT of text).\n"
        "  -compileverbose:      run each compile in verbose mode (implies -spewallcompiles).\n"
        "  -mergeverbose:        print details when merging from temp folders into game dir.\n\n"
    );
    printf(" General Options:\n");
    printf(
        "  -path <path>:          only build content in the specified path.\n"
        "  -ignoreerrors:         Ignores errors and doesnt stop content build.\n"
        "  -v:                    high level of verbosity\n"
//        "  -noask:                don't prompt the user (not recommended)\n"
        "  -include <ext>:        consider only specific resources for the operation.\n"
        "  -exclude <ext>:        exclude resources of a certain type, and their children\n"
//        "  -lsc <ext>:            lists assets with multiple parents.\n"
//        "  -lsr <ext>:            lists assets with multiple references.\n"
        "  -sharedrefsonly <ext>: restrict -lsr behavior to only consider references from the specified type.\n"
//        "  -restricted_write_include <ext>: Only write files with the given extension to disk.\n\n"
    );
    printf(" Lists (*.contentlist):\n");
    printf(
        "  -greylist <file>:     failure to compile any of this content is a warning, not an error.\n"
        "  -includelist <file>:  add the assets in this file to the build list.\n"
//        "  -whitelist <file>:    only consider this content and its children/referencees. UNSUPPORTED!!!.\n\n"
    );
    printf(" Orphans Options:\n");
    printf(
        "  -o:                   delete orphaned content\n"
        "  -lo:                  list orphaned content (don't delete)\n"
        "  -vpk:                 pack all compiled content into a vpk\n\n"
    );
    printf(" Advanced Build Options:\n");
    printf(
        "  -timeout <num>:       time (in seconds) to stop building content.\n"
        "  -proc <num>:          max simultaneous compile processes (default: 8).\n"
        "  -background:          run in 'background', lowest priority mode.\n"
        "  -repeat <num>:        number of times to build each asset. (default 1)\n"
        "  -failListFile <path>: append failed asset file paths to the specified text file.\n"
        "  -rsub <num>:          build a randomly chosen <num> sized subset of the assets.\n"
        "  -rsubseed <num>:      random number seed for -rsub, defaults to time-based seed.\n"
        "  -skipreverts:         skip the 'revert unchanged' step at the end of the build\n"
        "  -allowbulkdelete:     automatically allow large orphan deletions (USE WITH CAUTION)\n"
//        "  -dontchaincompileparents: don't chain compiles up to parents of the included type\n"
//        "                            (ie. if you run with -include vtex, this suppresses vmats)\n"
//        "  -dontcommit:          don't actually commit built content - just drop it on the floor.\n"
    );
    exit(-1);
}


//-----------------------------------------------------------------------------
// Purpose:   Self explanatory
//-----------------------------------------------------------------------------
void ParseCommandline(int argc, char* argv[])
{
    printf("Command line: %s\n", *CommandLine()->GetParms());

    if(argc == 0)
	{
        Msg("Must specify exactly one of -b, -lb, -o, -lo, -info, -lsc, -checkup, or -vpk\n");
        exit(-1);
    }     
    if(argc == 1 || CommandLine()->FindParm("-?") != NULL || CommandLine()->FindParm("-help") != NULL)
	{
        PrintUsage();
	}    
    if(CommandLine()->FindParm("-v") != NULL || CommandLine()->FindParm("-verbose") != NULL)
    {
        verbose = true;
    }
    if(CommandLine()->FindParm("-b") != NULL)
    {
        g_buildcontent = true;
    }       
    if(CommandLine()->FindParm("-pause") != NULL)
    {
        g_pause = true;
    }     
    if(CommandLine()->FindParm("-toolsforce32bits") != NULL)
    {
        g_force32bits = true;
        g_force64bits = false;
    }       
    if(CommandLine()->FindParm("-toolsforce64bits") != NULL)
    {
        g_force32bits = false;
        g_force64bits = true;
    }    
    if(CommandLine()->FindParm("-ignoreerrors") != NULL)
    {
        g_ignoreerrors = true;
    }     
    if(CommandLine()->FindParm("-nosteam") != NULL)
    {
        g_nosteam = true;
    }    
    if(CommandLine()->FindParm("-addonbuild") != NULL)
    {
        g_addonbuild = true;

        //For addon maps we dont build these assets types
        g_buildcaption = false;
        g_buildvpk = false;
        g_buildscene = false;
    }    
    if(CommandLine()->FindParm("-skipmaterial") != NULL)
    {
        g_buildmaterials = false;
    }    
    if(CommandLine()->FindParm("-skipmodel") != NULL)
    {
        g_buildmodels = false;
    }    
    if(CommandLine()->FindParm("-skipsound") != NULL)
    {
        g_buildsounds = false;
    }    
    if(CommandLine()->FindParm("-skipscene") != NULL)
    {
        g_buildscene = false;
    }
    if (CommandLine()->FindParm("-skipcaption") != NULL)
    {
        g_buildcaption = false;
    }   
    if (CommandLine()->FindParm("-skipmap") != NULL)
    {
        g_buildmap = false;
    }   
    if (CommandLine()->FindParm("-skipvpk") != NULL)
    {
        g_buildvpk = false;
    }
    if (CommandLine()->FindParm("-game") != NULL || CommandLine()->FindParm("-vproject") != NULL)
    {  
        int index = CommandLine()->FindParm("-game");
        if (index > 0 && index + 1 < argc)
        {
            const char* gamePath = argv[index + 1];

            // Check if the next argument is another command-line switch (e.g., "-b")
            if (gamePath[0] == '-')
            {
                Shared::qError("Error: -game requires a valid path argument.\n");
                exit(-1);
            }

            if(gamePath == NULL)
            {
                Shared::qError("Error: -game requires a valid path argument. NULL path\n");
                exit(-1);
            }

            V_snprintf(g_gamedir, sizeof(g_gamedir), "%s", gamePath);
        }
    }    
    if (CommandLine()->FindParm("-steamgamedir") != NULL)
    {  
        int index = CommandLine()->FindParm("-steamgamedir");
        if (index > 0 && index + 1 < argc)
        {
            const char* gamePath = argv[index + 1];

            // Check if the next argument is another command-line switch (e.g., "-b")
            if (gamePath[0] == '-')
            {
                Shared::qError("Error: -steamgamedir requires a valid path argument.\n");
            }

            if (gamePath == NULL)
            {
                Shared::qError("Error: -steamgamedir requires a valid path argument. NULL path\n");
                exit(-1);
            }

            V_snprintf(g_steamdir, sizeof(g_steamdir), "%s", gamePath);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    float start, end;

    start = Plat_FloatTime();

    Msg("Content builder (Build:%s %s)\n", __DATE__, __TIME__);

	CommandLine()->CreateCmdLine(argc, argv);
    InstallSpewFunction();

	ParseCommandline(argc, argv);

    Init_AssetTools();

    if (g_buildcontent)
    {
        if (g_buildmaterials)
        {
            MaterialBuilder::MaterialCompile(g_gamebin, sizeof(g_gamebin), process_completed, process_error);
        }
        if (g_buildmodels)
        {
            ModelBuilder::ModelCompile(g_gamebin, sizeof(g_gamebin), process_completed, process_error);
        }
        if (g_buildsounds)
        {
            SoundBuilder::SoundCompile(g_gamebin, sizeof(g_gamebin), process_completed, process_error);
        }
        if (g_buildscene)
        {
            SceneBuilder::SceneCompile(g_gamebin, sizeof(g_gamebin), process_completed, process_error);
        }
        if (g_buildcaption)
        {
            CaptionBuilder::CaptionCompile(g_gamebin, sizeof(g_gamebin), process_completed, process_error);
        }
        if (g_buildmap)
        {
            //MapBuilder::MapCompile(g_gamedir, sizeof(g_gamedir));
        }
        if (g_buildvpk)
        {
            VpkBuilder::VpkCompile(g_gamebin, sizeof(g_gamebin), process_completed, process_error);
        }
        end = Plat_FloatTime();

        Msg("\n"
            "-------------------------------------------------------------------------------------------\n"
            "| AssetCompile -> Done in %d seconds | Completed: %i     Error: %i     Skipped: %i         \n"
            "-------------------------------------------------------------------------------------------\n"
            "\n",
            (int)(end - start), process_completed, process_error, GetSkippedAssets()
        );
    }

    HitKeyToContinue();

	return 0;
}