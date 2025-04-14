#include "appframework/AppFramework.h"
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "filesystem_init.h"
#include "filesystem_tools.h"
#include "filesystem.h"
#include "loadcmdline.h"
#include "scriplib.h"
#include "utlbuffer.h"
#include "cmdlib.h"
#include "sceneimage.h"
#include "choreoevent.h"

#include "sceneimagebuilder.h"


//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
bool g_builvcd  = false;
bool g_pause    = true;
bool g_quiet    = false;
bool g_log      = false;


void HitKeyToContinue()
{
    if (g_pause)
    {
        system("pause");
    }
}


void PrintHeader()
{
    Msg("sceneimagebuilder.exe (Build: %s %s)\n", __DATE__, __TIME__);
}


void PrintUsage()
{
    Msg("Usage: sceneimagebuilder.exe [options] -game <path>\n"
        "   -game <paht>:    Path of the game folder to \'gameinfo.txt\'. (e.g: C:\\Half Life 2\\hl2)\n"
        "   -? or -help:     Prints help.\n"
        "   -v or -verbose:  Turn on verbose output.\n"
        "   -l:              log to file log.txt\n"
        "   -nopause:        Dont pause at end of processing.\n"
        "   -quiet:          Prints only escensial messagues.\n"
        "\n"
    );
    HitKeyToContinue();
    exit(-1);
}


void ParseCommandline()
{
    int i;
    for (i = 1; i < CommandLine()->ParmCount(); ++i)
    {
        const char* parm = CommandLine()->GetParm(i);

        if (parm[0] == '-')
        {
            switch (parm[1])
            {
            case 'h' || '?': // -help
                PrintUsage();
                break;

            case 'v': // -v or -verbose
                if (!Q_stricmp(parm, "-v") || !Q_stricmp(parm, "-verbose"))
                {
                    qprintf("Verbose mode enabled\n");
                    verbose = true;
                    g_quiet = false;
                }
                break;

            case 'n': // -nopause
                if (!Q_stricmp(parm, "-nopause"))
                {
                    qprintf("No pause enabled.\n");
                    g_pause = false;
                }
                break;
            case 'l':
                if (!Q_stricmp(parm, "-l"))
                {
                    qprintf("log mode enabled.\n");
                    g_log = true;
                }
            case 'q': // -quiet
                if (!Q_stricmp(parm, "-quiet"))
                {
                    qprintf("Quiet mode enabled.\n");
                    verbose = false;
                    g_quiet = true;
                }
                break;

            case 'g': // -game
                if (!Q_stricmp(parm, "-game"))
                {
                    ++i;
                    if (i < CommandLine()->ParmCount())
                    {
                        const char* gamePath = CommandLine()->GetParm(i);
                        if (gamePath[0] == '-')
                        {
                            Error("Error: -game requires a valid path argument.\n");
                        }
                        else
                        {
                            V_strcpy(gamedir, gamePath);
                        }
                    }
                    else
                    {
                        Error("Error: -game requires a valid path argument.\n");
                    }
                }
                break;

            default:
                Warning("Warning: Unknown option '%s'\n", parm);
                PrintUsage();
            }
        }
        else
        {
            Warning("Warning: Unknown non-option argument '%s'\n", parm);
            PrintUsage();
        }
    }

    if (CommandLine()->ParmCount() < 2 || (i != CommandLine()->ParmCount()))
    {
        PrintUsage();
    }
}


//-----------------------------------------------------------------------------
// Purpose: Sets up the game path
//-----------------------------------------------------------------------------
bool CSceneImageBuilderApp::SetupSearchPaths()
{
    if (!BaseClass::SetupSearchPaths(NULL, false, true))
        return false;

    // Set gamedir.
    V_AppendSlash(gamedir, sizeof(gamedir));

    return true;
}


//-----------------------------------------------------------------------------
// Purpose: Contructor
//-----------------------------------------------------------------------------
bool CSceneImageBuilderApp::Create()
{
    AppSystemInfo_t appSystems[] =
    {
        { "", "" }	// Required to terminate the list
    };

    return AddSystems(appSystems);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
void CSceneImageBuilderApp::Destroy()
{
}


bool CSceneImageBuilderApp::PreInit()
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


void CSceneImageBuilderApp::PostShutdown()
{
    g_pFileSystem = NULL;
    BaseClass::PostShutdown();
}


bool CSceneImageBuilderApp::CreateSceneImageFile(CUtlBuffer& targetBuffer, char const* pchModPath, bool bLittleEndian, bool bQuiet, ISceneCompileStatus* Status)
{
    bool bSuccess = sceneImage.CreateSceneImageFile(targetBuffer, pchModPath, bLittleEndian, bQuiet, Status);
    return bSuccess;
}


void CSceneImageBuilderApp::SceneBuild()
{
    CUtlBuffer	targetBuffer;
    char sceneCompiledPath[MAX_PATH], _gamedir[1024]; //gamedir without the blackslash
    const char* SceneFile = "scenes\\scenes.image";

    V_strcpy(_gamedir, gamedir);
    V_StripTrailingSlash(_gamedir);

    V_snprintf(sceneCompiledPath, sizeof(sceneCompiledPath), "%s\\%s", _gamedir, SceneFile);

    qprintf("Game path: %s\n", _gamedir);
    qprintf("Scene source (.vcd): %s\\%s\\*.vcd\n", _gamedir, "scenes");
    qprintf("Scene compiled (.image): %s\n", sceneCompiledPath);

    if (g_pSceneImage->CreateSceneImageFile(targetBuffer, _gamedir, true, g_quiet, NULL))
    {
        Msg("Writting compiled Scene file: %s... ", sceneCompiledPath);

        if (scriptlib->WriteBufferToFile(sceneCompiledPath, targetBuffer, WRITE_TO_DISK_ALWAYS))
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
        Error("\nScene Compile failed!\n");
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int CSceneImageBuilderApp::Main()
{
    float start = Plat_FloatTime();

    SetupDefaultToolsMinidumpHandler();
    InstallSpewFunction();
    PrintHeader();

    ParseCommandline();

    Create();
    PreInit();

    SceneBuild();

    Destroy();
    PostShutdown();

    Msg("--> Done in %.2f seconds.\n", Plat_FloatTime() - start);
    HitKeyToContinue();

    return 0;
}


//-----------------------------------------------------------------------------
// Purpose: Main entry point 
//-----------------------------------------------------------------------------
DEFINE_CONSOLE_STEAM_APPLICATION_OBJECT(CSceneImageBuilderApp)