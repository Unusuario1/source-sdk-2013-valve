//==== TemplateTool -> Written by Unusuario2, https://github.com/Unusuario2  ===//
//
// Purpose: Use this .cpp file as a template for your custom util in the source engine.
//
// License:
//        MIT License
//
//        Copyright (c) 2025 [un usuario], https://github.com/Unusuario2
//
//        Permission is hereby granted, free of charge, to any person obtaining a copy
//        of this software and associated documentation files (the "Software"), to deal
//        in the Software without restriction, including without limitation the rights
//        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//        copies of the Software, and to permit persons to whom the Software is
//        furnished to do so, subject to the following conditions:
//
//        The above copyright notice and this permission notice shall be included in all
//        copies or substantial portions of the Software.
//
//        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//        SOFTWARE.
//
// $NoKeywords: $
//==============================================================================//
#include <tier1/strtools.h>
#include <tier0/icommandline.h>
#include <tools_minidump.h>
#include <loadcmdline.h>
#include <cmdlib.h>
#include <filesystem_init.h>
#include <filesystem_tools.h>
#include <colorschemetools.h>
#include <pipeline_shareddefs.h>


//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
float               g_flStartTime       = 0;




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
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Prints the header
//-----------------------------------------------------------------------------
static void PrintHeader()
{
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, "Unusuario2 - External Console (Build: %s %s)\n", __DATE__, __TIME__);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static void Init(int argc, char* argv[])
{
    g_flStartTime = Plat_FloatTime();

    SetupDefaultToolsMinidumpHandler();
    CommandLine()->CreateCmdLine(argc, argv);
    InstallSpewFunction();
    PrintHeader();
    ParseCommandline(argc, argv);
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
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    Init(argc, argv);

    // Put here the logic of your program.


    Destroy(argc, argv);

    return 0;
}

