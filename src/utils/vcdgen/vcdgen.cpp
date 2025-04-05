#include "appframework/AppFramework.h"
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "filesystem_init.h"
#include "filesystem.h"
#include "loadcmdline.h"
#include "scriplib.h"
#include "utlbuffer.h"
#include "cmdlib.h"
#include "../game/shared/sceneimage.h"
#include "../game/shared/choreoevent.h"

#include "vcdgen.h"


//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
bool g_builvcd = false;
bool g_pause = true;
bool g_quiet = false;


void HitKeyToContinue()
{
    if (g_pause)
    {
        system("pause");
    }
}


void PrintHeader()
{
    Msg("Valve Choreography Data Generator (vcdgen.exe)(Build: %s %s)\n", __DATE__, __TIME__);
}


void PrintUsage()
{
    Msg("Usage: Generates scene.image file from vcd folder.\n"
        "   -game : path of the game folder (e.g: C:\\Half Life 2\\hl2)\n"
        "   -?/-help : Prints help\n"
        "   -v/-verbose : Turn on verbose output.\n"
        "   -nopause :  pause at end of processing.\n"
        "   -quiet :  pause at end of processing.\n"
    );
}


void ParseCommandline()
{
    if (CommandLine()->ParmCount() == 0)
    {
        Error("Must specify exactly one of -game, -? || -help or -v || -verbose\n");
        HitKeyToContinue();
    }
    if (CommandLine()->ParmCount() == 1 || CommandLine()->FindParm("-?") != NULL || CommandLine()->FindParm("-help") != NULL)
    {
        PrintUsage();
        HitKeyToContinue();
    }
    if (CommandLine()->FindParm("-v") != NULL || CommandLine()->FindParm("-verbose") != NULL)
    {
        qprintf("Verbose mode enabled\n");
        verbose = true;
        g_quiet = false;
    }
    if (CommandLine()->FindParm("-nopause") != NULL)
    {
        qprintf("No pause enabled.\n");
        g_pause = false;
    }
    if (CommandLine()->FindParm("-quiet") != NULL)
    {
        qprintf("Quiet mode enabled.\n");
        verbose = false;
        g_quiet = true;
    }
    if (CommandLine()->FindParm("-game") != NULL)
    {   
        const char* gamePath = CommandLine()->ParmValue("-game");

        if (!gamePath || gamePath[0] == '-')
        {
            Error("Error: -game requires a valid path argument.\n");
        }
        else
        {
            V_snprintf(gamedir, sizeof(gamedir), "%s", gamePath);
        }
    }
}


bool CVcdGenApp::SetupSearchPaths()
{
    return CTier3SteamApp::SetupSearchPaths(NULL, false, true) ? true : false;
}


bool CVcdGenApp::Create()
{
    AppSystemInfo_t appSystems[] =
    {
        { "", "" }	// Required to terminate the list
    };

    return AddSystems(appSystems);
}


void CVcdGenApp::Destroy()
{
}


bool CVcdGenApp::PreInit()
{
    if (!CTier3SteamApp::PreInit())
        return false;

    g_pFileSystem = g_pFullFileSystem;
    if (!g_pFileSystem)
    {
        Error("Unable to load required library interface! (FileSystem)\n");
        return false;
    }

    g_pFullFileSystem->SetWarningFunc(Warning);

    // Add paths...
    if (!SetupSearchPaths())
        return false;

    return true;
}

void CVcdGenApp::PostShutdown()
{
    g_pFileSystem = NULL;
    BaseClass::PostShutdown();
}

bool CVcdGenApp::CreateSceneImageFile(CUtlBuffer& targetBuffer, char const* pchModPath, bool bLittleEndian, bool bQuiet, ISceneCompileStatus* Status)
{
    bool bs = sceneImage.CreateSceneImageFile(targetBuffer, pchModPath, bLittleEndian, bQuiet, Status);
    return bs;
}


void CVcdGenApp::SceneBuild()
{
    Create();
    PreInit();
    CUtlBuffer	targetBuffer;
    char _tempvargamedir[MAX_PATH];
    const char* SceneFile = "scenes\\scenes.image";

    V_strncpy(_tempvargamedir, gamedir, sizeof(_tempvargamedir));
    V_StripTrailingSlash(_tempvargamedir);
    V_snprintf(_tempvargamedir, sizeof(_tempvargamedir), "%s\\%s", _tempvargamedir, SceneFile);

    qprintf("Game path: %s\n", gamedir);
    qprintf("Scene source (.vcd): %s\\%s\\*.vcd\n", gamedir, "scene");
    qprintf("Scene compiled (.image): %s\n", _tempvargamedir);

    if (g_pSceneImage->CreateSceneImageFile(targetBuffer, gamedir, true, g_quiet, NULL))
    {
        Msg("Writting compiled Scene file: %s... ", _tempvargamedir);

        if (scriptlib->WriteBufferToFile(_tempvargamedir, targetBuffer, WRITE_TO_DISK_ALWAYS))
        {
            Msg("done\n");
        }
        else
        {
            Error("FAILED!!\n");
        }
    }
    else
    {
        Msg("Scene Compile failed!\n");
    }
    Destroy();
    PostShutdown();
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int CVcdGenApp::Main()
{
    float start, end;
    start = Plat_FloatTime();

    PrintHeader();
    InstallSpewFunction();
    ParseCommandline();

    SceneBuild();

    end = Plat_FloatTime();
    Msg("--> Done in %f seconds.\n", end - start);
    HitKeyToContinue();

    return 0;
}


//-----------------------------------------------------------------------------
// Purpose: Main entry point 
//-----------------------------------------------------------------------------
DEFINE_CONSOLE_STEAM_APPLICATION_OBJECT(CVcdGenApp)