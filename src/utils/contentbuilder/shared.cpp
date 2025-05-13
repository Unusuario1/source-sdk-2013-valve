//========= --------------------------------------------------- ============//
//
// Purpose: Shared funtion across the subsystems
//
// $NoKeywords: $
//=============================================================================//
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

        char str[8192], szErrorFile[MAX_PATH] = "";
        Q_vsnprintf(str, sizeof(str), format, argptr);

        if (g_ignoreerrors)
        {
            V_snprintf(szErrorFile, sizeof(szErrorFile), "%s\\%s\\%s", gamedir, CONTENTBUILDER_OUTPATH, CONTENTBUILDER_ERROR_MSG);
            FILE* file = fopen(szErrorFile, "a");
            
            ColorSpewMessage(SPEW_MESSAGE, &red, "%s", str);

            if (!file)
            {
                Warning("AssetSystem -> Cannot save Error msg!!\n");
            }
            else
            {
                fprintf(file, "%s", str);
                fclose(file);
            }
        }
        else
        {
            Error("%s", str);
        }

        va_end(argptr);
    }    
    
    
    void qWarning(const char* format, ...)
    {
        Color yellow = { 255,255,0 , 255 };
        va_list argptr;
        va_start(argptr, format);

        char str[8192], szWarnignFile[MAX_PATH] = "";
        Q_vsnprintf(str, sizeof(str), format, argptr);
        V_snprintf(szWarnignFile, sizeof(szWarnignFile), "%s\\%s\\%s", gamedir, CONTENTBUILDER_OUTPATH, CONTENTBUILDER_WARNING_MSG);
        FILE* file = fopen(szWarnignFile, "a");

        ColorSpewMessage(SPEW_MESSAGE, &yellow, "%s", str);

        if (!file)
        {
            Warning("AssetSystem -> Cannot save Warning msg!!\n");
        }
        else
        {
            fprintf(file, "%s", str);
            fclose(file);
        }

        va_end(argptr);
    }

    // TODO: FIX THIS: we are calling this mf in every SINGLE ITERATION!!! this should be called ONLY ONE time in the lifespam of contentbuilder!!
    bool ExcludeDirOrFile(const char* assetpath)
    {
        KeyValues* pContentBuilderKV_Read = ReadKeyValuesFile(g_contentbuilderdir);
        
        if(!Shared::CheckIfPathOrFileExist(g_contentbuilderdir))
        {
            Error("NO");
        }
        
        if (!pContentBuilderKV_Read) 
        {
            return false; 
        }

        KeyValues* pContentBuilderKV = pContentBuilderKV_Read->FindKey(CONTENTBUILDER_KV, false);
        if (!pContentBuilderKV) 
        { 
            return false; 
        }

        KeyValues* pAssetSystemKV = pContentBuilderKV->FindKey(EXCLUDE_KV, false);        
        if (!pAssetSystemKV)
        {
            return false;
        }

        // Get how many paths are in Exclude KV
        std::size_t uiExcludePath = 0;
        for (KeyValues* subKey = pAssetSystemKV->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
        {
            uiExcludePath++;
        }

        // terminate the list
        std::size_t j = 0;
        char** folderExcludeList = new char* [uiExcludePath];
        for(KeyValues* subKey = pAssetSystemKV->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
        {
            folderExcludeList[j] = new char[MAX_PATH];

            V_snprintf(folderExcludeList[j],sizeof(folderExcludeList[j]), "%s", subKey->GetString(EXCLUDESTRING_KV, NULL));
            
            j++;
        }

        //Check if the file or folder is in the path
        for(std::size_t i = 0; i < uiExcludePath;i++)
        {
            if(V_strstr(assetpath, folderExcludeList[i]))
            {
                // Msg("\n\n\n\t%s\n\n", folderExcludeList[i]);
                // free the memory
                for (std::size_t i = 0; i < uiExcludePath; i++)
                {
                    delete[] folderExcludeList[i];
                }
                delete[] folderExcludeList;

                pContentBuilderKV_Read->deleteThis();
                
                return true;
            }
        }

        // free the memory
        for(std::size_t i = 0; i< uiExcludePath;i++)
        {
            delete[] folderExcludeList[i];
        }
        delete[] folderExcludeList;

        pContentBuilderKV_Read->deleteThis();

        return false;
    }


    bool CreateDirectoryRecursive(const char* path) 
    {
        char szTemp[MAX_PATH];

        V_strncpy(szTemp, path, MAX_PATH);
        V_StripTrailingSlash(szTemp);

        for (char* p = szTemp + 3; *p; p++)
        {
            if (*p == '\\') 
            {
                *p = '\0';
                CreateDirectoryA(szTemp, NULL); // ignore errors
                *p = '\\';
            }
        }

        return CreateDirectoryA(szTemp, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
    }


    bool CopyDirectoryContents(const char* srcPath, const char* dstPath, const char *pExtension)
    {
        char szSrcSearch[MAX_PATH];
        V_snprintf(szSrcSearch, sizeof(szSrcSearch), "%s\\*%s", srcPath, pExtension);

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(szSrcSearch, &findData);

        if (hFind == INVALID_HANDLE_VALUE) 
        {
            Shared::qError("AssetSystem -> Failed to open source directory: %s\n", srcPath);
            return false;
        }

        // Create the destination directory if it doesn't exist
        CreateDirectoryA(dstPath, NULL);

        do 
        {
            // Skip "." and ".."
            if (V_strcmp(findData.cFileName, ".") == 0 || V_strcmp(findData.cFileName, "..") == 0)
                continue;

            char szFullSrc[MAX_PATH];
            char szFullDst[MAX_PATH];
            V_snprintf(szFullSrc, sizeof(szFullSrc), "%s\\%s", srcPath, findData.cFileName);
            V_snprintf(szFullDst, sizeof(szFullDst), "%s\\%s", dstPath, findData.cFileName);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                // Recursively copy subdirectories
                CopyDirectoryContents(szFullSrc, szFullSrc, pExtension);
            }
            else 
            {
                // Copy the file
                if (!CopyFileA(szFullDst, szFullDst, FALSE))
                {
                    Shared::qError("AssetSystem -> Failed to copy file: %s to %s (Error: %lu)\n", szFullSrc, szFullDst, GetLastError());
                }
            }
        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
        return true;
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Given a outdir, srcdir and a pExtension it will copy all the files of srcdir to outdir 
    //-----------------------------------------------------------------------------
    void CopyFilesRecursivelyGame(const char* srcDir, const char* srcfolder, const char* gamefolder, const char* pExtension) 
    {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind;
        char szSearchPath[MAX_PATH];
        
        V_snprintf(szSearchPath, sizeof(szSearchPath), "%s\\*", srcDir);

        hFind = FindFirstFileA(szSearchPath, &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) 
        {
            Error("Error: Cannot open directory: \"%s\"\n", srcDir);
        }

        do 
        {
            char szFullPath[MAX_PATH] = "", *outPath;
            const char* fileName = findFileData.cFileName;

            // Skip "." and ".."
            if (V_strcmp(fileName, ".") == 0 || V_strcmp(fileName, "..") == 0) 
            {
                continue;
            }

            V_snprintf(szFullPath, sizeof(szFullPath), "%s\\%s", srcDir, fileName);
            outPath = Shared::ReplaceSubstring(szFullPath, srcfolder, gamefolder);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                // Recursively process subdirectories
                free(outPath);
                CopyFilesRecursivelyGame(szFullPath, srcfolder, gamefolder, pExtension);
            }
            else 
            {
                const char* szFileExt = V_strrchr(fileName, '.');

                if (szFileExt && V_strcmp(szFileExt, pExtension) == 0)
                {
                    char* szCreateDir = Shared::ReplaceSubstring(srcDir, srcfolder, gamefolder);
                    CreateDirectory(szCreateDir, NULL);
                    free(szCreateDir);

                    if (!CopyFileA(szFullPath, outPath, FALSE))
                    {
                        Shared::qError("Failed to copy: %s to %s\n", szFullPath, outPath);
                        free(outPath);
                    }
                    else 
                    {
                        qprintf("\n AssetSystemVerbose -> Copied: %s -> %s\n", szFullPath, outPath);
                        free(outPath);
                    }
                }
            }
        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
    }


    void DeleteFolderWithContents(const char* folderPath) 
    {
        char szPath[MAX_PATH];
        V_snprintf(szPath, sizeof(szPath), "%s\\", folderPath); // trailing slash required
        szPath[V_strlen(szPath) + 1] = '\0';                    // double null-terminated string

        SHFILEOPSTRUCTA fileOp = { 0 };
        fileOp.wFunc = FO_DELETE;
        fileOp.pFrom = szPath;
        fileOp.fFlags = FOF_NO_UI; // No confirmation dialogs

        int result = SHFileOperationA(&fileOp);
        if (result != 0) 
        {
            Shared::qWarning("AssetSystem -> Failed to delete: \"%s\". SHFileOperation error: %d\n", szPath, result);
        }
    }


    //-----------------------------------------------------------------------------
    // Purpose: This is only used for tools that support batch compile!
    //-----------------------------------------------------------------------------
    void AssetInfoBuild(const char* folder, const char* pExtension)
    {
        char szSearchPath[MAX_PATH];

        if(!g_infocontent)
        {
            return;
        }

        V_snprintf(szSearchPath, sizeof(szSearchPath), "%s\\*", folder);

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(szSearchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
            return;

        do 
        {
            const char* szName = findData.cFileName;

            if (V_strcmp(szName, ".") == 0 || V_strcmp(szName, "..") == 0)
                continue;

            char szFullPath[MAX_PATH];
            V_snprintf(szFullPath, sizeof(szFullPath), "%s\\%s", folder, szName);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                AssetInfoBuild(szFullPath, pExtension); // Recurse into subdirectory
            }
            else 
            {
                const char* szDot = V_strrchr(szName, '.');
                if (szDot && V_stricmp(szDot, pExtension) == 0)
                {     
                    ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "OK");
                    Msg(" - %s - ", Shared::TimeStamp());
                    ColorSpewMessage(SPEW_MESSAGE, &path_color, "%s\n", szFullPath);
                }
            }

        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }


    char* MergeString(const char* string, const char* sub1)
    {
        static char szInternal[MAX_PATH] = "";
        V_snprintf(szInternal, sizeof(szInternal), "%s\\%s", string, sub1);
        return szInternal;
    }


    static bool ScanFolderSaveContentsInternal(const char* asset_folder, FILE* file, const char* pExtension)
    {
        char szSearchPath[MAX_PATH];
        V_snprintf(szSearchPath, sizeof(szSearchPath), "%s\\*", asset_folder);

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(szSearchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
            return false;

        do
        {
            const char* szName = findData.cFileName;

            if (V_strcmp(szName, ".") == 0 || V_strcmp(szName, "..") == 0)
                continue;

            char szFullPath[MAX_PATH];
            V_snprintf(szFullPath, sizeof(szFullPath), "%s\\%s", asset_folder, szName);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                ScanFolderSaveContentsInternal(szFullPath, file, pExtension); // Recurse into subdirectory
            }
            else
            {
                const char* szDot = V_strrchr(szName, '.');
                if (szDot && V_stricmp(szDot, pExtension) == 0)
                {
                    fprintf(file, "%s\n", szName);
                }
            }

        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
        return true;
    }


    bool ScanFolderSaveContents(const char* asset_folder, const char* outputfile, const char* pExtension, const char* pSubSystem)
    {
        FILE* file = fopen(outputfile, "a");
        if (!file)
        {
            Shared::qError("AssetSystem -> Error opening file: \"%s\"\n", outputfile);
            return false;
        }

        fprintf(file, "//-- %s -- auto generated file for %s system --//\n", CONTENTBUILDER_KV, pSubSystem);

        bool bResult = ScanFolderSaveContentsInternal(asset_folder, file, pExtension);
        
        if(bResult)
        {
            qWarning("AssetSystem -> Cound not generate report at: \"%s\"\n", outputfile);
        }

        fprintf(file, "\n\n");
        fclose(file);

        return bResult;
    }

    

    //-----------------------------------------------------------------------------
    // Purpose: Print asset time stamp
    //-----------------------------------------------------------------------------
    void AssetInfoBuild(WIN32_FIND_DATAA findData)
    {
        ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "OK");
        Msg(" - %s - ", Shared::TimeStamp());
        ColorSpewMessage(SPEW_MESSAGE, &path_color, "%s\n", findData.cFileName);
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Counts how many of a given asset are in a dir
    //-----------------------------------------------------------------------------
    std::size_t CountAssets(const char* directory,const char* asset_type) 
    {
        char szSearchPath[MAX_PATH];
        int count = 0;

        size_t extLen = V_strlen(asset_type);

        V_snprintf(szSearchPath, sizeof(szSearchPath), "%s\\*", directory); // Search for all files and folders

        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile(szSearchPath, &findData);
        //if (hFind == INVALID_HANDLE_VALUE) 

        do
        {
            if (V_strcmp(findData.cFileName, ".") == 0 || V_strcmp(findData.cFileName, "..") == 0) 
            { 
                continue; // Skip special entries
            } 

            char szFullPath[MAX_PATH];
            V_snprintf(szFullPath, sizeof(szFullPath), "%s\\%s", directory, findData.cFileName);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
            {
                // If it's a directory, recursively scan it
                count += CountAssets(szFullPath, asset_type);
            }
            else 
            {
                // Check if the file ends with asset_type
                size_t len = V_strlen(findData.cFileName);
                if (len > extLen && V_strcmp(findData.cFileName + len - extLen, asset_type) == 0)
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

        size_t str_len = V_strlen(str);
        size_t old_len = V_strlen(old_sub);
        size_t new_len = V_strlen(new_sub);

        // First count how many times old_sub appears
        int count = 0;
        const char* tmp = str;
        while ((tmp = V_strstr(tmp, old_sub))) 
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

        while ((tmp = V_strstr(src, old_sub))) 
        {
            size_t bytes_to_copy = tmp - src;
            memcpy(dst, src, bytes_to_copy);
            dst += bytes_to_copy;
            memcpy(dst, new_sub, new_len);
            dst += new_len;
            src = tmp + old_len;
        }
        V_strcpy(dst, src); // Copy the rest

        return result;
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Creates asset compile dirs 
    //-----------------------------------------------------------------------------
    void CreateAssetSystemGamePath(const char* gamedir, const char* asset_dir)
    {
        char szTemp[MAX_PATH];
        V_snprintf(szTemp, sizeof(szTemp), "%s\\%s", gamedir, asset_dir);

        if (SHCreateDirectoryEx(NULL, szTemp, NULL) == ERROR_SUCCESS)
        {
            if (g_spewallcommands) 
            {
                Msg("\tAssetSystem -> Creating directory: ");   ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", szTemp);
            }
        }
        else if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            if (g_spewallcommands)
            {
                Msg("\tAssetSystem -> Directory:");
                ColorSpewMessage(SPEW_MESSAGE, &path_color, " \"%s\" ", szTemp);
                Msg(", already exists!\n");
            }
        }
        else
        {
            Shared::qError("AssetSystem -> Cound not create: \"%s\"\n"
                           "AssetSystem -> Exit code (%i)\n",
                            szTemp, GetLastError()
            );
        }
    }


    const char* TimeStamp()
    {
        float actualTime = Plat_FloatTime();
        float deltaTime = actualTime - g_timer;

        if (deltaTime < 0) deltaTime = 0;

        std::size_t mins = static_cast<std::size_t>(deltaTime) / 60;
        std::size_t secs = static_cast<std::size_t>(deltaTime) % 60;

        static char szTime[128];
        V_snprintf(szTime, sizeof(szTime), "%.2llum:%.2llus", mins, secs);

        return szTime;
    }


    //-----------------------------------------------------------------------------
    // Purpose: does the string have the pExtension? "foo.bsp" ".bsp" -> true
    //-----------------------------------------------------------------------------
    bool HasExtension(const char* filename, const char* pExtension)
    {
        const char* szDot = V_strrchr(filename, '.');
        return (szDot && V_stricmp(szDot, pExtension) == 0);
    }


    void PrintHeaderCompileType(const char* compile_type)
    {
        ColorSpewMessage(SPEW_MESSAGE, &header_color, "\n====== %s %s ======\n", g_infocontent ? "Printing" : "Building", compile_type);
    }


    //-----------------------------------------------------------------------------
    // Purpose:     If g_buildoutofdatecontent = true, we check if the compiled
    //              asset is out of date in comparasion to the src, if it is,
    //              return true, if not false
    //-----------------------------------------------------------------------------
    bool PartialBuildAsset(const char* asset_path, const char* asset_src_folder, const char* asset_folder)
    {
        if (!g_buildoutofdatecontent)
        {
            return true;
        }

        char* szAssetSrcPath = V_strdup(asset_path);
        char* szAssetPath = ReplaceSubstring(szAssetSrcPath, asset_src_folder, asset_folder);

        // Convert char* to wchar_t*
        wchar_t wCompiled[MAX_PATH];
        wchar_t wSource[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, szAssetSrcPath, -1, wSource, MAX_PATH);
        MultiByteToWideChar(CP_UTF8, 0, szAssetPath, -1, wCompiled, MAX_PATH);

        FILETIME ftCompiled, ftSource;

        // Open both files
        HANDLE hCompiled = CreateFileW(wCompiled, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE hSource = CreateFileW(wSource, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hCompiled == INVALID_HANDLE_VALUE || hSource == INVALID_HANDLE_VALUE) 
        {
            // If compiled asset doesn't exist, it's definitely out of date
            if (hCompiled == INVALID_HANDLE_VALUE) 
            {
                if (hSource != INVALID_HANDLE_VALUE) 
                {
                    CloseHandle(hSource);
                }

                // avoid memory leaks
                free(szAssetSrcPath);
                free(szAssetPath);
                return true;
            }

            // Otherwise, something else failed
            if (hCompiled != INVALID_HANDLE_VALUE) 
            {
                CloseHandle(hCompiled);
            }
            if (hSource != INVALID_HANDLE_VALUE) 
            {
                CloseHandle(hSource);
            }

            Shared::qWarning("AssetSystem -> Failed to open files. Force-build asset.\n");

            // avoid memory leaks
            free(szAssetSrcPath);
            free(szAssetPath);
            return false;
        }

        // Get last write times
        GetFileTime(hCompiled, NULL, NULL, &ftCompiled);
        GetFileTime(hSource, NULL, NULL, &ftSource);

        CloseHandle(hCompiled);
        CloseHandle(hSource);

        free(szAssetSrcPath);
        free(szAssetPath);

        // Compare timestamps
        return CompareFileTime(&ftSource, &ftCompiled) > 0;
    }


    //-----------------------------------------------------------------------------
    // Purpose:
    //-----------------------------------------------------------------------------
    static bool FindFileRecursive(const char* baseDir, const char* pExtension)
    {
        char szSearchPath[MAX_PATH];
        V_snprintf(szSearchPath, sizeof(szSearchPath), "%s\\*", baseDir);

        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA(szSearchPath, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE)
            return false;

        do
        {
            if (V_strcmp(findFileData.cFileName, ".") == 0 || V_strcmp(findFileData.cFileName, "..") == 0)
                continue;

            char szFullPath[MAX_PATH];
            V_snprintf(szFullPath, sizeof(szFullPath), "%s\\%s", baseDir, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (FindFileRecursive(szFullPath, pExtension))
                {
                    FindClose(hFind);
                    return true;
                }
            }
            else
            {
                if (HasExtension(findFileData.cFileName, pExtension))
                {
                    FindClose(hFind);
                    return true;
                }
            }

        } while (FindNextFileA(hFind, &findFileData));

        FindClose(hFind);
        return false;
    }

    // TODO: maybe remove this!
    //-----------------------------------------------------------------------------
    // Purpose: Checks a if a dir has at least ONE asset type
    //-----------------------------------------------------------------------------
    bool DirectoryAssetTypeExist(const char* directoryPath, const char* pExtension, const char* asset_type)
    {
        if (FindFileRecursive(directoryPath, pExtension))
        {
            return true;
        }
        else 
        {
            Shared::qWarning(   "AssetSystem -> No files with pExtension \"%s\" found in \"%s\"\n"
                                "AssetSystem -> Skipping %s (%s) compile!\n",
                                pExtension, directoryPath, asset_type, pExtension);
            return false;
        }
    }


    //TODO: remove this!!
    //-----------------------------------------------------------------------------
    // Purpose: Does a dir exist? 
    //-----------------------------------------------------------------------------
    bool DirectoryExists(const char* path)
    {
        DWORD attr = GetFileAttributesA(path);
        return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Check if the tool exists
    //-----------------------------------------------------------------------------
    void AssetToolCheck(const char* pGameBin, const char* tool_name, const char* sub_system)
    {
        char szPath[MAX_PATH];

        V_snprintf(szPath, sizeof(szPath), "%s\\%s", pGameBin, tool_name);

        if (g_spewallcommands)
        {
            Msg("\t%s: ", sub_system);
            ColorSpewMessage(SPEW_MESSAGE, &path_color, "\"%s\"\n", szPath);
        }

        if (!Shared::CheckIfPathOrFileExist(szPath))
        {
            Shared::qError("\n\t%s doesnt not exist in %s !\n", tool_name, pGameBin);
        }
    }


    //----------------------------------------------------------------------------
    // Purpose: Starts .exe tools
    //----------------------------------------------------------------------------
    void StartExe( const char* type_asset, const char* tool_name, const char* Keyvalues)
    {
        char szGameToolPath[8192];
        float start = Plat_FloatTime();

        V_snprintf(szGameToolPath, sizeof(szGameToolPath), "\"%s\\%s\" %s", g_gamebin, tool_name, Keyvalues);

        if (!g_quiet)
        {
            Msg("AssetTools -> Starting: ");    ColorSpewMessage(SPEW_MESSAGE, &path_color, "%s\n\n", szGameToolPath);
        }

        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);

        // Create process
        if (!CreateProcess(NULL, szGameToolPath, NULL, NULL, false, 0, NULL, NULL, &si, &pi))
        {
            Shared::qError("AssetSystem -> %s could not start!\n", szGameToolPath);
            g_process_error++;
            return;
        }

        // Wait until child process exits
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Retrieve exit code
        DWORD exitCode = 0;
        if (GetExitCodeProcess(pi.hProcess, &exitCode))
        {
            if (exitCode > 0)
            {
                Shared::qWarning("AssetSystem -> %s compile failed: %d!\n", szGameToolPath, exitCode);
                g_process_error++;
                return;
            }
            else
            {
                g_process_completed++;
            }
        }
        else
        {
            Shared::qError("GetExitCodeProcess() failed!\n");
            g_process_error++;
            return;
        }

        // Close process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        if (!g_quiet)
        {
            ColorSpewMessage(SPEW_MESSAGE, &sucesfullprocess_color, "\nAssetTools -> Done building %s in %f seconds.\n\n", type_asset, Plat_FloatTime() - start);
        }
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Checks if path or file exists
    //-----------------------------------------------------------------------------
    bool CheckIfPathOrFileExist(const char *path)
    {
        return (_access(path, 0) == 0);
    }


    //-----------------------------------------------------------------------------
    // Purpose:   Do we target 32 or 64 bits tools? if true x64, false x86
    //-----------------------------------------------------------------------------
    bool TargetPlatform()
    {
        bool szTemp = PLATFORM_64BITS ? true : false; // Default value
        
        if (g_force32bits) 
        {
            szTemp = false;
        }
        else if (g_force64bits)
        {
            szTemp = true;
        }

        return szTemp;
    }


    //-----------------------------------------------------------------------------
    // Purpose:     Gets the bin path where the tools are located
    //-----------------------------------------------------------------------------
    void SetUpBinDir(char* string, size_t uiBufferSize)
    {
        char szTemp[MAX_PATH];

        if (!g_nosteam) 
        {
            FileSystem_GetAppInstallDir(szTemp, sizeof(szTemp));
        }
        else
        {
            V_snprintf(szTemp, sizeof(szTemp), "%s", g_steamdir);
        }

        V_snprintf(string, uiBufferSize, "%s\\%s", szTemp, TargetPlatform() ? TOOLS_PATH_64BITS : TOOLS_PATH_32BITS);
    }

    //TODO: rework this to pass childer KeyValues!!
    //-----------------------------------------------------------------------------
    // Purpose:     Loads specific tools KeyValues, we asume that we are inside ContentBuilder KV 
    //-----------------------------------------------------------------------------
    void LoadGameInfoKv(const char* ToolKeyValue, char* output_argv, std::size_t uiBufferSize)
    {
        float start = Plat_FloatTime();

        KeyValues* ContentBuilderKV_Read = ReadKeyValuesFile(g_contentbuilderdir);
        KeyValues* ContentBuilderKV = ContentBuilderKV_Read->FindKey(CONTENTBUILDER_KV, false);

        qprintf("AssetSystemVerbose -> Loading Keyvalues from: \'%s\'... ", ToolKeyValue);

        if (!ContentBuilderKV_Read || !ContentBuilderKV)
        {
            Shared::qWarning("AssetSystem -> Could not get \'%s\' KeyValues from \"%s\"!\n", CONTENTBUILDER_KV, g_contentbuilderdir);
            Shared::qWarning("AssetSystem -> Using default values for asset compile, this might not be ideal!\n");
            return;
        }

        KeyValues* ToolKV = ContentBuilderKV->FindKey(ToolKeyValue, false);

        if (!ToolKV)
        {
            Shared::qWarning("AssetSystem -> Could not get \'%s\' KeyValues from \"%s\"!\n", ToolKeyValue, g_contentbuilderdir);
            Shared::qWarning("AssetSystem -> Using default values for asset compile, this might not be ideal!\n");
            return;
        }

        for (KeyValues* subKey = ToolKV->GetFirstSubKey(); subKey; subKey = subKey->GetNextKey())
        {
            const char* argv_tool = subKey->GetString(BUILDPARAM, NULL);
            if (argv_tool) 
            {
                V_snprintf(output_argv, uiBufferSize, " %s %s ", output_argv, argv_tool);
            }
        }

        ContentBuilderKV_Read->deleteThis();

        if (verbose) 
        { 
            ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);
        }
    }
}