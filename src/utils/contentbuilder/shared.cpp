#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <cwchar>
#include <wchar.h>

#include "filesystem_init.h"
#include "KeyValues.h"
#include "Color.h"
#include "cmdlib.h"

#include "contentbuilder.h"
#include "colorscheme.h"
#include "shared.h"


namespace Shared
{
    //-----------------------------------------------------------------------------
    // Purpose:   Ignore errors (no terminating call) if g_ignoreerrors = true
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
    void CopyFilesRecursively(const char* srcDir, const char* srcfolder, const char* gamefolder, const char* extension) 
    {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind;
        char searchPath[MAX_PATH];
        
        V_snprintf(searchPath, sizeof(searchPath), "%s\\*", srcDir);

        hFind = FindFirstFileA(searchPath, &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) 
        {
            Error("Error: Cannot open directory: \"%s\"\n", srcDir);
        }

        do 
        {
            char fullPath[MAX_PATH] = "", *outPath;
            const char* fileName = findFileData.cFileName;

            // Skip "." and ".."
            if (V_strcmp(fileName, ".") == 0 || V_strcmp(fileName, "..") == 0) 
            {
                continue;
            }

            V_snprintf(fullPath, sizeof(fullPath), "%s\\%s", srcDir, fileName);
            outPath = Shared::ReplaceSubstring(fullPath, srcfolder, gamefolder);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                // Recursively process subdirectories
                free(outPath);
                CopyFilesRecursively(fullPath, srcfolder, gamefolder, extension);
            }
            else 
            {
                const char* fileExt = V_strrchr(fileName, '.');
                if (fileExt && V_strcmp(fileExt, extension) == 0) 
                {
                    char* createDir = Shared::ReplaceSubstring(srcDir, srcfolder, gamefolder);
                    CreateDirectoryA(createDir, NULL);
                    free(createDir);

                    if (!CopyFileA(fullPath, outPath, FALSE)) 
                    {
                        Shared::qError("Failed to copy: %s to %s\n", fullPath, outPath);
                        free(outPath);
                    }
                    else 
                    {
                        qprintf("\nCopied: %s -> %s\n", fullPath, outPath);
                        free(outPath);
                    }
                }
            }
        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
    }


    //-----------------------------------------------------------------------------
    // Purpose: This is only used for tools that support batch compile!
    //-----------------------------------------------------------------------------
    void AssetInfoBuild(const char* folder, const char* extension)
    {
        char searchPath[MAX_PATH];
        V_snprintf(searchPath, sizeof(searchPath), "%s\\*", folder);

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(searchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
            return;

        do 
        {
            const char* name = findData.cFileName;

            if (V_strcmp(name, ".") == 0 || V_strcmp(name, "..") == 0)
                continue;

            char fullPath[MAX_PATH];
            V_snprintf(fullPath, MAX_PATH, "%s\\%s", folder, name);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                AssetInfoBuild(fullPath, extension); // Recurse into subdirectory
            }
            else 
            {
                const char* dot = V_strrchr(name, '.');
                if (dot && V_stricmp(dot, extension) == 0) 
                {
                    ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "OK");
                    Msg(" - %s - ", Shared::TimeStampAsset());
                    ColorSpewMessage(SPEW_MESSAGE, &path_color, "%s\n", fullPath);
                }
            }

        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }


    //-----------------------------------------------------------------------------
    // Purpose: Print asset time stamp
    //-----------------------------------------------------------------------------
    void AssetInfoBuild(WIN32_FIND_DATAA findData)
    {
        ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "OK");
        Msg(" - %s - ", Shared::TimeStampAsset());
        ColorSpewMessage(SPEW_MESSAGE, &path_color, "%s\n", findData.cFileName);
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
            Warning("\tAssetSystem -> Failed to open directory: %s\n", directory);
        }

        do
        {
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) 
            { 
                continue; // Skip special entries
            } 

            char fullPath[MAX_PATH];
            V_snprintf(fullPath, MAX_PATH, "%s\\%s", directory, findData.cFileName);

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
    // Purpose: Given a string, sub-string, and string to replace it replaces the 
    //          sub-string with the string to replace
    // NOTE: remeber to free the memory! (e.g: free(foo);)
    //-----------------------------------------------------------------------------
    char* ReplaceSubstring(const char* str, const char* old_sub, const char* new_sub) 
    {
        if (!str || !old_sub || !new_sub) 
            return NULL;

        size_t str_len = strlen(str);
        size_t old_len = strlen(old_sub);
        size_t new_len = strlen(new_sub);

        // First count how many times old_sub appears
        int count = 0;
        const char* tmp = str;
        while ((tmp = strstr(tmp, old_sub))) 
        {
            count++;
            tmp += old_len;
        }

        // Allocate memory for the new string
        size_t new_str_len = str_len + count * (new_len - old_len) + 1;
        char* result = (char*)malloc(new_str_len);
        if (!result) return NULL;

        char* dst = result;
        const char* src = str;

        while ((tmp = strstr(src, old_sub))) 
        {
            size_t bytes_to_copy = tmp - src;
            memcpy(dst, src, bytes_to_copy);
            dst += bytes_to_copy;
            memcpy(dst, new_sub, new_len);
            dst += new_len;
            src = tmp + old_len;
        }
        strcpy(dst, src); // Copy the rest

        return result;
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
            if (g_spewallcommands) 
            {
                Msg("\tAssetSystem -> Creating directory: ");   ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", _temp);
            }
        }
        else if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            if (g_spewallcommands)
            {
                Msg("\tAssetSystem -> Directory:");
                ColorSpewMessage(SPEW_MESSAGE, &path_color, " \"%s\" ", _temp);
                Msg(", already exists!\n");
            }
        }
        else
        {
            Shared::qError("AssetSystem -> Cound not create: \"%s\"\n"
                           "AssetSystem -> Exit code (%i)\n",
                           _temp, GetLastError()
            );
        }
    }

    const char* TimeStampAsset()
    {
        float actualTime = Plat_FloatTime();
        float deltaTime = g_timer - actualTime;

        if (deltaTime < 0) deltaTime = 0;

        std::size_t mins = static_cast<std::size_t>(deltaTime) / 60;
        std::size_t secs = static_cast<std::size_t>(deltaTime) % 60;

        static char szTime[128];
        V_snprintf(szTime, sizeof(szTime), "%.2llum:%.2llus", mins, secs);

        return szTime;
    }


    //-----------------------------------------------------------------------------
    // Purpose: does the string have the extension? "foo.bsp" ".bsp" -> true
    //-----------------------------------------------------------------------------
    bool HasExtension(const char* filename, const char* extension)
    {
        const char* dot = strrchr(filename, '.');
        return (dot && _stricmp(dot, extension) == 0);
    }


    //-----------------------------------------------------------------------------
    // Purpose:
    //-----------------------------------------------------------------------------
    bool FindFileRecursive(const char* baseDir, const char* extension)
    {
        char searchPath[MAX_PATH];
        V_snprintf(searchPath, MAX_PATH, "%s\\*", baseDir);

        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA(searchPath, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE)
            return false;

        do
        {
            if (V_strcmp(findFileData.cFileName, ".") == 0 || V_strcmp(findFileData.cFileName, "..") == 0)
                continue;

            char fullPath[MAX_PATH];
            V_snprintf(fullPath, MAX_PATH, "%s\\%s", baseDir, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (FindFileRecursive(fullPath, extension))
                {
                    FindClose(hFind);
                    return true;
                }
            }
            else
            {
                if (HasExtension(findFileData.cFileName, extension))
                {
                    FindClose(hFind);
                    return true;
                }
            }

        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
        return false;
    }


    //-----------------------------------------------------------------------------
    // Purpose: Checks a if a dir has at least ONE asset type
    //-----------------------------------------------------------------------------
    bool DirectoryAssetTypeExist(const char* directoryPath, const char* extension, const char* asset_type)
    {
        if (FindFileRecursive(directoryPath, extension))
        {
            return true;
        }
        else 
        {
            Warning("AssetsSystem -> No files with extension \"%s\" found in \"%s\"\n"
                    "AssetsSystem -> Skipping %s (%s) compile!\n",
                    extension, directoryPath, asset_type, extension);
            return false;
        }
    }


    //-----------------------------------------------------------------------------
    // Purpose:
    //-----------------------------------------------------------------------------
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
        V_snprintf(tool_path, sizeof(tool_path), "%s\\%s", gamebin, tool_name);

        //qprintf("Verbose | Tool path: %s\n", tool_path);

        if (g_spewallcommands)
        {
            Msg("\t%s: ", sub_system);
            ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", tool_path);
        }

        if (!Shared::CheckIfFileExist(tool_path))
        {
            Shared::qError("\t\n%s doesnt not exist in %s !\n", tool_name, gamebin);
        }
    }


    //----------------------------------------------------------------------------
    // Purpose: Starts .exe tools
    //----------------------------------------------------------------------------
    void StartExe(const char* gamebin, std::size_t bufferSize, const char* type_asset, const char* tool_name, 
                            const char* Keyvalues, std::size_t &complete, std::size_t &error, bool quietmode = false)
    {
        char _gametoolpath[8192];
        float start = Plat_FloatTime();

//  if (!quietmode) 
//  {
//      Msg("\n\n====== Building %s ======\n", type_asset);
//  }

        V_snprintf(_gametoolpath, sizeof(_gametoolpath), "\"%s\\%s\" %s", gamebin, tool_name, Keyvalues);

        if (!quietmode)
        {
            Msg("AssetTools -> Starting: ");    ColorSpewMessage(SPEW_MESSAGE, &path_color, "%s\n\n", _gametoolpath);
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
                complete++;
            }
        }
        else
        {
            Shared::qError("GetExitCodeProcess() failed!\n");
            error++;
        }

        // Close process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        if (!quietmode)
        {
            ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "\nAssetTools -> Done building %s in %f seconds.\n\n", type_asset, Plat_FloatTime() - start);
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
    // Purpose:   Do we target 32 or 64 bits tools? if true x64, false x86
    //-----------------------------------------------------------------------------
    bool TargetPlatform()
    {
        bool _temp = PLATFORM_64BITS ? true : false; // Default value
        
        if (g_force32bits) 
        {
            _temp = false;
        }
        else if (g_force64bits)
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
    void LoadGameInfoKv(const char* ToolKeyValue, char* output_argv, std::size_t bufferSize)
    {
        float start = Plat_FloatTime();

        KeyValues* GameInfoKVCubemap = ReadKeyValuesFile(g_gameinfodir);

        qprintf("Loading Keyvalues from: %s ... ", ToolKeyValue);

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

        verbose ? ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start) : void();
    }
}