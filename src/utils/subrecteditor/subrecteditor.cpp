#include "mainwindow.hpp"
#include <QApplication>
#include "coresubrecteditor.hpp"


//-----------------------------------------------------------------------------
// Purpose:   Parse command line
//-----------------------------------------------------------------------------
static void ParseCommandline(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (!V_stricmp(argv[i], "-FullMinidumps"))
        {
            EnableFullMinidumps(true);
        }
        else if (!V_stricmp(argv[i], "-game") || !V_stricmp(argv[i], "-vproject"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* gamePath = argv[i];
                if (!gamePath)
                {
                    Error("\nError: \'-game\' requires a valid path argument. NULL path\n");
                }
                V_strcpy_safe(gamedir, gamePath);
            }
            else
            {
                Error("\nError: \'-game\' requires a valid path argument.\n");
            }
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static void Init(int argc, char* argv[])
{
    SetupDefaultToolsMinidumpHandler();
    CommandLine()->CreateCmdLine(argc, argv);
    InstallSpewFunction();
    ParseCommandline(argc, argv);

    CmdLib_InitFileSystem(gamedir);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static void Destroy(int argc, char* argv[])
{
    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    Init(argc, argv);

    QApplication App(argc, argv);
    MainWindow Window;
    Window.setWindowTitle("Sub Rect Editor");
    Window.show();

    const int iRet = App.exec();
    Destroy(argc, argv);
    return iRet;
}

