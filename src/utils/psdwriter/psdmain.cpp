//========== ------------------------------------------------- ============//
//
// Purpose: Converts .png, .jpg, etc.. to .psd for vtex compile
//
// $NoKeywords: $
//=============================================================================//
#include <windows.h>
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "loadcmdline.h"
#include "cmdlib.h"
#include "filesystem_init.h"
#include "filesystem_tools.h"
#include "color.h"
#include "psdwriter.h"


// TODO list:
//	-Add a exclusion command smth like -tga
//	-Add a command that manages the compression of the .psd file
//	-Also add the template albedo, normal, etc.. texture template
//  -Add an option to remove the old source files


#define MATERIALSRC_DIR		"materialsrc"


//-----------------------------------------------------------------------------
// Purpose: Global vars, helpers for writing psd files
//-----------------------------------------------------------------------------
//TODO: MAKE A SANTIY check the iamge cannot be greater than 4096x4096
bool			g_bQuiet = false;
bool            g_bSignature = false;
bool            g_bIsSingleFile = true;
bool            g_bTempalteGeneration = false;
char			g_szGameMaterialSrcDir[MAX_PATH] = "";
char			g_szSingleInputFile[MAX_PATH] = "";
char			g_szExcludeExt[16][16] = { "" };
const char* g_rgpAssetConvertList[10] = {   ".jpg",         //0
                                            ".jpeg",        //1
                                            ".png",         //2
                                            ".tga",         //3
                                            ".bmp",         //4
                                            ".gif",         //5
                                            ".hdr",         //6
                                            ".pic",         //7
                                            ".ppm",         //9
                                            ".pgm"          //10
};
const char* g_rgpEndFileName[12] = {          "_color",         //https://developer.valvesoftware.com/wiki/$basetexture
                                            "_normal",        //https://developer.valvesoftware.com/wiki/$bumpmap
                                            "_ao",            //https://developer.valvesoftware.com/wiki/$ambientoccltexture
                                            "_detail",        //https://developer.valvesoftware.com/wiki/$detail
                                            "_envmap",        //https://developer.valvesoftware.com/wiki/$envmap
                                            "_envmapmask",    //https://developer.valvesoftware.com/wiki/$envmapmask
                                            "_trans",         //
                                            "_specular"       //https://developer.valvesoftware.com/wiki/$phongexponenttexture                             
                                            "_flowmap",       //https://developer.valvesoftware.com/wiki/Water_(shader)#Flowing_water
                                            "_selfilum",      //https://developer.valvesoftware.com/wiki/Glowing_textures_(Source)#$selfillum
                                            "_blendmodulate", //https://developer.valvesoftware.com/wiki/$blendmodulatetexture
                                            "_lightwarp",     //https://developer.valvesoftware.com/wiki/$lightwarptexture
                                            "_dudvmap"        //https://developer.valvesoftware.com/wiki/Water_(shader)
                                            "_ramp"           //https://developer.valvesoftware.com/wiki/$ramptexture
#if 0
                                            "_tintmask",      //this is only in post found in csgo branch
                                            "_flowmapnoise",  //https://developer.valvesoftware.com/wiki/Water_(shader)#Flowing_water
#endif     
};
uint16_t		g_uiImageWidth;
uint16_t		g_uiImageHeight;
Color			header_color(0, 255, 255, 255);
Color			successful_color(0, 255, 0, 255);
Color			failed(255, 0, 0, 255);


//-----------------------------------------------------------------------------
// Purpose:   
//-----------------------------------------------------------------------------
void ProcessDirAndConvertContents()
{
	char szWildCard[MAX_PATH] = "";

	for (const char* pExtension : g_rgpAssetConvertList)
	{
		for (uint8_t i = 0; i < 8; i++) //TODO: fix this!
		{
			if (pExtension != g_szExcludeExt[i]) 
			{
				V_snprintf(szWildCard, sizeof(szWildCard), "%s\\*%s", g_szGameMaterialSrcDir, pExtension);

				// TODO: make here the implementation 
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:   
//-----------------------------------------------------------------------------
void Init()
{
	V_StripTrailingSlash(gamedir);
	V_snprintf(g_szGameMaterialSrcDir, sizeof(g_szGameMaterialSrcDir), "%s\\%s", gamedir, MATERIALSRC_DIR);
}


//-----------------------------------------------------------------------------
// Purpose:   Prints the header
//-----------------------------------------------------------------------------
void PrintHeader()
{
	ColorSpewMessage(SPEW_MESSAGE, &header_color, "\n----- -------- - psdwriter.exe (Build: %s %s)\n", __DATE__, __TIME__);
}


//-----------------------------------------------------------------------------
// Purpose:   Print psdwriter usage
//-----------------------------------------------------------------------------
void PrintUsage(int argc, char* argv[])
{
    char szTemp[256] = "";

    for(const char* pExt: g_rgpAssetConvertList)
    {
        V_snprintf(szTemp, sizeof(szTemp), "%s %s ", szTemp == "" ? "" : szTemp, pExt);
    }

    Msg("Usage: psdwriter.exe [options] -game <path>\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " General Options:\n");
    Msg("   -i <file>:                      \n"
        "   -recc:                          \n"  
        "   -psdtemplategeneration:         \n"
        "   -exclusionformat <extension>:   Suported formats: %s\n"
        "   -signature <studio> <mod>:      (e.g: -signature myfirststudio myfirstsourcemod)\n"
		"   -game <path>:                   Specify the folder of the gameinfo.txt file.\n"
        "   -vproject <path>:               Same as \'-game\'.\n"
        "\n", szTemp);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Spew Options:\n");
    Msg("   -v or -verbose:                 Enables verbose.\n"
        "   -quiet:                         Prints minimal text. (Note: Disables \'-verbose\')\n"
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Other Options:\n");
    Msg("   -FullMinidumps:                 Write large minidumps on crash.\n"
        "   -forcebittype <n>:              8/16/32 applied to all .psd files\n" 
        "   -psdcompresion <n>:            \n"
        "\n");

    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(1);
}


//-----------------------------------------------------------------------------
// Purpose:   Parse command line
//-----------------------------------------------------------------------------
void ParseCommandline(int argc, char* argv[])
{
    if (argc == 1)
    {
        PrintUsage(argc, argv);
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
			g_bQuiet = false;
        }
        else if (!V_stricmp(argv[i], "-quiet"))
        {
			verbose	 = false;
            g_bQuiet = true;
        }
        else if (!V_stricmp(argv[i], "-FullMinidumps"))
        {
			EnableFullMinidumps(true);
		}
        else if (!V_stricmp(argv[i], "-psdtemplategeneration"))
        {
            g_bTempalteGeneration = true;
		}
        else if (!V_stricmp(argv[i], "-i"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* pInputPath = argv[i];
                if (!pInputPath)
                {
                    Error("Error: \'-i\' requires a valid path argument. NULL path\n");
                }
                g_bIsSingleFile = true;
                V_strcpy(g_szSingleInputFile, pInputPath);
            }
            else
            {
                Error("Error: \'-i\' requires a valid path argument.\n");
            }
        }
        else if (!V_stricmp(argv[i], "-game") || !V_stricmp(argv[i], "-vproject"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* pGamePath = argv[i];
                if (!pGamePath)
                {
                    Error("Error: \'-game\' requires a valid path argument. NULL path\n");
                }
                V_strcpy(gamedir, pGamePath);
            }
            else
            {
                Error("Error: \'-game\' requires a valid path argument.\n");
            }
        }
        else
        {
            Error("\Error Unknown option \'%s\'\n", argv[i]);
            PrintUsage(argc, argv);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	SetupDefaultToolsMinidumpHandler();
	CommandLine()->CreateCmdLine(argc, argv);
	InstallSpewFunction();
	PrintHeader();
	ParseCommandline(argc, argv);
	CmdLib_InitFileSystem(gamedir); // remove this!
	Init();
	
    if (g_bIsSingleFile)
    {
        CheckAndExportToPsd(g_szSingleInputFile);
    }
    else 
    {
        ProcessDirAndConvertContents();
    }

	DeleteCmdLine(argc, argv);
	CmdLib_Cleanup();
	CmdLib_Exit(1);
	return 0;
}
