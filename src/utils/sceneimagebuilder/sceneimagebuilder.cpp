//========= --------------------------------------------------- ============//
//
// Purpose: 
//
//=====================================================================================//
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
bool g_bBuilvcd  = false;
bool g_bPause    = true;
bool g_bQuiet    = false;
bool g_bLog      = false;


void HitKeyToContinue()
{
    if (g_bPause)
    {
        system("pause");
    }
}


void PrintHeader()
{
    //Msg("Valve Software - sceneimagebuilder.exe (Build: %s %s)\n", __DATE__, __TIME__);
    Msg("-------------- - sceneimagebuilder.exe (Build: %s %s)\n", __DATE__, __TIME__);
}


void PrintUsage()
{
    Msg("\n"
        "Usage: sceneimagebuilder.exe [options] -game <path>\n"
        "   -game <path>:    Path of the game folder to \'gameinfo.txt\'. (e.g: C:\\Half Life 2\\hl2)\n"
        "   -? or -help:     Prints help.\n"
        "   -v or -verbose:  Turn on verbose output.\n"
        "   -l:              log to file sceneimagebuilder.log\n"
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
        const char* pParm = CommandLine()->GetParm(i);

        if (pParm[0] == '-')
        {
            switch (pParm[1])
            {
            case 'h' || '?': // -help
                PrintUsage();
                break;

            case 'v': // -v or -verbose
                if (!Q_stricmp(pParm, "-v") || !Q_stricmp(pParm, "-verbose"))
                {
                    qprintf("Verbose mode enabled\n");
                    verbose = true;
                    g_bQuiet = false;
                }
                break;

            case 'n': // -nopause
                if (!Q_stricmp(pParm, "-nopause"))
                {
                    qprintf("No pause enabled.\n");
                    g_bPause = false;
                }
                break;
            case 'l':
                if (!Q_stricmp(pParm, "-l"))
                {
                    qprintf("log mode enabled.\n");
                    g_bLog = true;
                }
            case 'q': // -quiet
                if (!Q_stricmp(pParm, "-quiet"))
                {
                    qprintf("Quiet mode enabled.\n");
                    verbose = false;
                    g_bQuiet = true;
                }
                break;

            case 'g': // -game
                if (!Q_stricmp(pParm, "-game"))
                {
                    ++i;
                    if (i < CommandLine()->ParmCount())
                    {
                        const char* pGamePath = CommandLine()->GetParm(i);
                        if (pGamePath[0] == '-')
                        {
                            Error("Error: -game requires a valid path argument.\n");
                        }
                        else
                        {
                            V_strcpy(gamedir, pGamePath);
                        }
                    }
                    else
                    {
                        Error("Error: -game requires a valid path argument.\n");
                    }
                }
                break;

            default:
                Warning("Warning: Unknown option '%s'\n", pParm);
                PrintUsage();
            }
        }
        else
        {
            Warning("Warning: Unknown non-option argument '%s'\n", pParm);
            PrintUsage();
        }
    }

    if (CommandLine()->ParmCount() < 2 || (i != CommandLine()->ParmCount()))
    {
        PrintUsage();
    }
}


bool CSceneImageBuilderApp::SetupSearchPaths()
{
    if (!BaseClass::SetupSearchPaths(NULL, false, true))
        return false;

    // Set gamedir.
    V_AppendSlash(gamedir, sizeof(gamedir));

    return true;
}


bool CSceneImageBuilderApp::Create()
{
    AppSystemInfo_t appSystems[] =
    {
        { "", "" }	// Required to terminate the list
    };

    return AddSystems(appSystems);
}


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
    {
        return false;
    }
    
    if(g_bLog)
    {
        char logFile[512];
        V_snprintf(logFile, sizeof(logFile), "%s\%s\\%s.log", gamedir, "scenes", "sceneimagebuilder");
        remove(logFile);
        SetSpewFunctionLogFile(logFile);
    }

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
    char szSceneCompiledPath[MAX_PATH], szGameDir[1024]; //gamedir without the blackslash
    const char* SceneFile = "scenes\\scenes.image";

    V_strcpy(szGameDir, gamedir);
    V_StripTrailingSlash(szGameDir);

    V_snprintf(szSceneCompiledPath, sizeof(szSceneCompiledPath), "%s\\%s", szGameDir, SceneFile);

    qprintf("Game path: %s\n", szGameDir);
    qprintf("Scene source (.vcd): %s\\%s\\*.vcd\n", szGameDir, "scenes");
    qprintf("Scene compiled (.image): %s\n", szSceneCompiledPath);

    if (g_pSceneImage->CreateSceneImageFile(targetBuffer, szGameDir, true, g_bQuiet, NULL))
    {
        Msg("Writting compiled Scene file: %s... ", szSceneCompiledPath);

        if (scriptlib->WriteBufferToFile(szSceneCompiledPath, targetBuffer, WRITE_TO_DISK_ALWAYS))
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

    InstallSpewFunction();
    SetupDefaultToolsMinidumpHandler();
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