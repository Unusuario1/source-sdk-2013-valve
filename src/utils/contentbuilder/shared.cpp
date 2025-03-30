#include <io.h>
#include <windows.h>
#include <shlobj.h>
#include <cwchar>

#include "filesystem_init.h"
#include "KeyValues.h"
#include "Color.h"
#include "cmdlib.h"

#include "contentbuilder.h"
#include "shared.h"


namespace Shared
{
    //-----------------------------------------------------------------------------
    // Purpose:   Ignore errors (no terminating call) if they -ignoreerros is enabled
    //-----------------------------------------------------------------------------
    void qError(const char* format, ...)
    {
        Color red = { 255,0,0 };
        va_list argptr;
        va_start(argptr, format);

        char str[2048];
        Q_vsnprintf(str, sizeof(str), format, argptr);

        if (g_ignoreerrors)
        {
            ColorSpewMessage(SPEW_MESSAGE, &red, "%s", str);
        }
        else
        {
            Error("%s", str);
        }

        va_end(argptr);
    }

    //-----------------------------------------------------------------------------
    // Purpose:   Given a outdir, srcdir and a extension it will copy all the files of srcdir to outdir 
    //-----------------------------------------------------------------------------
    void CopyFilesRecursively(const char* srcDir, const char* outDir, const char* extension) 
    {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind;
        char searchPath[MAX_PATH];
        
        V_snprintf(searchPath, sizeof(searchPath), "%s\\*", srcDir);

        hFind = FindFirstFileA(searchPath, &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) 
        {
            Shared::qError("Error: Cannot open directory: \"%s\"\n", srcDir);
            return;
        }

        do 
        {
            const char* fileName = findFileData.cFileName;

            // Skip "." and ".."
            if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) 
            {
                continue;
            }

            char fullPath[MAX_PATH];
            char outPath[MAX_PATH];

            V_snprintf(fullPath, sizeof(fullPath), "%s\\%s", srcDir, fileName);
            V_snprintf(outPath, sizeof(outPath), "%s\\%s", outDir, fileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                // Recursively process subdirectories
                CopyFilesRecursively(fullPath, outDir, extension);
            }
            else 
            {
                // Get file extension
                const char* fileExt = strrchr(fileName, '.');
                if (fileExt && strcmp(fileExt, extension) == 0) 
                {

                    // Ensure the output directory exists
                    CreateDirectoryA(outDir, NULL);

                    // Copy file to outDir
                    if (!CopyFileA(fullPath, outPath, FALSE)) 
                    {
                        Shared::qError("Failed to copy: %s to %s\n", fullPath, outPath);
                    }
                    else 
                    {
                        qprintf("Copied: %s -> %s\n", fullPath, outPath);
                    }
                }
            }
        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Counts how many of a given asset are in a dir
    //-----------------------------------------------------------------------------
    std::size_t CountAssets(const char* directory, const char* asset_type) 
    {
        char searchPath[MAX_PATH];
        int count = 0;
        size_t extLen = strlen(asset_type);

        V_snprintf(searchPath, MAX_PATH, "%s\\*", directory); // Search for all files and folders

        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile(searchPath, &findData);

        if (hFind == INVALID_HANDLE_VALUE) 
        {
            Shared::qError("\tAssetSystem -> Failed to open directory: %s\n", directory);
        }

        do
        {
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) 
            { 
                continue; // Skip special entries
            } 

            char fullPath[MAX_PATH];
            snprintf(fullPath, MAX_PATH, "%s\\%s", directory, findData.cFileName);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                // If it's a directory, recursively scan it
                count += CountAssets(fullPath, asset_type);
            }
            else 
            {
                // Check if the file ends with asset_type
                size_t len = strlen(findData.cFileName);
                if (len > extLen && strcmp(findData.cFileName + len - extLen, asset_type) == 0)
                {
                    count++;
                }
            }
        } while (FindNextFile(hFind, &findData) != 0);

        FindClose(hFind);
        return count;
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Creates asset compile dirs 
    //-----------------------------------------------------------------------------
    void CreateAssetSystemGamePath(const char* gamedir, const char* asset_dir)
    {
        char _temp[MAX_PATH];
        V_snprintf(_temp, sizeof(_temp), "%s\\%s", gamedir, asset_dir);

        if (SHCreateDirectoryEx(NULL, _temp, NULL) == ERROR_SUCCESS)
        {
            Msg("\tAssetSystem -> Creating directory: \"%s\"\n", _temp);
        }
        else if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            Msg("\tAssetSystem -> Directory: \"%s\", already exists!\n", _temp);
        }
        else
        {
            Shared::qError("AssetSystem -> Cound not create: \"%s\"\n"
                           "AssetSystem -> Exit code (%i)\n",
                           _temp, GetLastError()
            );
        }
    }


    bool DirectoryExists(const char* path) 
    {
        DWORD attr = GetFileAttributesA(path);
        return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Check if the tool exists
    //-----------------------------------------------------------------------------
    void AssetToolCheck(const char* gamebin, const char* tool_name, const char* sub_system)
    {
        char tool_path[MAX_PATH];
        V_snprintf(tool_path, sizeof(tool_path), " %s\\%s ", gamebin, tool_name);

        Msg("\t%s: \"%s\"\n", sub_system, tool_path);

        if (Shared::CheckIfFileExist(tool_path))
        {
            Shared::qError("%s doesnt not exist in %s !\n", tool_name, gamebin);
            exit(-1);
        }
    }


    //----------------------------------------------------------------------------
    // Purpose: Starts .exe tools
    //----------------------------------------------------------------------------
    void StartExe(const char* gamebin, std::size_t bufferSize, const char* type_asset, const char* tool_name, 
                            const char* Keyvalues, std::size_t &complete, std::size_t &error, bool quietmode = false)
    {
        char _gametoolpath[MAX_PATH];
        float start, end;

        start = Plat_FloatTime();

        if (!quietmode) 
        {
            Msg("\n====== Building %s ======\n", type_asset);
        }

        V_snprintf(_gametoolpath, sizeof(_gametoolpath), "\"%s\\%s\" %s", gamebin, tool_name, Keyvalues);

        if (!quietmode)
        {
            Msg("AssetTools -> Starting: %s\n\n", _gametoolpath);
        }

        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);

        // Create process
        if (!CreateProcess(NULL, _gametoolpath, NULL, NULL, false, 0, NULL, NULL, &si, &pi))
        {
            Shared::qError("%s could not start!\n", _gametoolpath);
            error++;
        }

        // Wait until child process exits
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Retrieve exit code
        DWORD exitCode = 0;
        if (GetExitCodeProcess(pi.hProcess, &exitCode))
        {
            if (exitCode > 0)
            {
                Shared::qError("%s compile failed: %d!\n", _gametoolpath, exitCode);
                error++;
            }
            else
            {
                if (!quietmode) 
                {
                    Msg("%s compile complete!\n", _gametoolpath);
                }
                complete++;
            }
        }
        else
        {
            Shared::qError("GetExitCodeProcess() failed!\n");
        }

        // Close process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        end = Plat_FloatTime();
        
        if (!quietmode)
        {
            Msg("\nAssetTools -> Done building %s in %f seconds.\n", type_asset, end - start);
        }
    }


    //-----------------------------------------------------------------------------
    // Purpose:     Char to wide character type, REMENBER TO FREE! free(output), 
    //-----------------------------------------------------------------------------
    wchar_t* CtoWc(char* input, wchar_t* output,std::size_t size)
    {
        output = (wchar_t*)malloc(size * sizeof(wchar_t));
        
        if (!output)
        {
            return nullptr;
        }
        
        mbstowcs(output, input, (size * sizeof(wchar_t)));
        return output;
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Checks if file exist
    //-----------------------------------------------------------------------------
    FORCEINLINE bool CheckIfFileExist(const char *path)
    {
        return (_access(path, 0) == 0);
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Do we target 32 or 64 bits tools?
    //-----------------------------------------------------------------------------
    bool TargetPlatform()
    {
        bool _temp = PLATFORM_64BITS ? true : false;
        
        if (g_force32bits) 
        {
            _temp = true;
        }
        if (g_force64bits)
        {
            _temp = true;
        }

        return _temp;
    }


    //-----------------------------------------------------------------------------
    // Purpose:     Gets the bin path where the tools are located
    //-----------------------------------------------------------------------------
    void SetUpBinDir(char* string, size_t bufferSize)
    {
        char temp[MAX_PATH];

        if (!g_nosteam) 
        {
            FileSystem_GetAppInstallDir(temp, sizeof(temp));
        }
        else
        {
            V_snprintf(temp, sizeof(temp), "%s", g_steamdir);
        }

        V_snprintf(string, bufferSize, "%s\\%s", temp, TargetPlatform() ? TOOLS_PATH_64BITS : TOOLS_PATH_32BITS);
    }

    //-----------------------------------------------------------------------------
    // Purpose:     Loads specific tools KeyValues, we asume that we are inside ContentBuilder KV 
    //-----------------------------------------------------------------------------
    void LoadGameInfoKv(const char* ToolKeyValue, char * output_argv, std::size_t bufferSize)
    {
        KeyValues* GameInfoKVCubemap = ReadKeyValuesFile(g_gameinfodir);
        
        if (!GameInfoKVCubemap)
        {
            Shared::qError("Could not get KeyValues from %s!\n", g_gameinfodir);
        }

        KeyValues* ContentBuilderKV = GameInfoKVCubemap->FindKey(CONTENTBUILDER_KV, false);

        if (!ContentBuilderKV)
        {
            Shared::qError("Could not get \'%s\' KeyValues from %s!\n", CONTENTBUILDER_KV, g_gameinfodir);
        }

        KeyValues* ToolKV = ContentBuilderKV->FindKey(ToolKeyValue, false);

        if (!ToolKV)
        {
            Shared::qError("Could not get \'%s\' KeyValues from %s!\n", ToolKeyValue, g_gameinfodir);
        }
        
        for (KeyValues* subKey = ToolKV->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
        {
            V_snprintf(output_argv, bufferSize, "%s %s %s", output_argv, subKey->GetName(), subKey->GetString());
        }
    }
}