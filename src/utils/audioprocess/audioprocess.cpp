//==== AudioProcess -> Written by Unusuario2, https://github.com/Unusuario2  ===//
//
// Purpose: 
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
#include <windows.h>
#include <tier1/strtools.h>
#include <tier0/icommandline.h>
#include <tools_minidump.h>
#include <loadcmdline.h>
#include <cmdlib.h>
#include <filesystem_init.h>
#include <filesystem_tools.h>
#include <resourcecopy/cresourcecopy.hpp>
#include <colorschemetools.h>
#include <pipeline_shareddefs.h>
extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/error.h>
    #include <libavutil/mem.h>
}

#pragma warning(disable : 4238)

#define MAX_SAMPLE_RATE 44100u

//-----------------------------------------------------------------------------
// Purpose: Global vars 
//-----------------------------------------------------------------------------
float           g_flStartTime           = 0;
int             g_uiCompletedOperations = 0;
int             g_uiSkippedOperations   = 0;
int             g_uiFailedOperations    = 0;
bool            g_bQuiet                = false;
bool            g_bPathMode             = true;
bool            g_bShallowMode          = false;
ContainerList*  g_pVAudioFormats        = nullptr;
CResourceCopy*  g_pResourceCopy         = nullptr;
ContainerList*  g_pContainerSubStrings  = nullptr;
SpewMode        g_eSpewMode             = SpewMode::k_Normal;


//-----------------------------------------------------------------------------
// Purpose:   
//-----------------------------------------------------------------------------
static bool IsAValidAudioFile(const char* pFileName)
{
    for (int i = 0; i < g_pVAudioFormats->size(); ++i)
    {
        if(V_strstr(V_strrchr(pFileName, '.'), g_pVAudioFormats->at(i).data()))
            return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
// Purpose:   
//-----------------------------------------------------------------------------
static void ConvertAudioFileToWav(const char* pFileName, const char* pNewDir)
{
    if (!IsAValidAudioFile(pFileName))
    {
        Warning("AudioProcess -> The file:");
        ColorSpewMessage(SPEW_MESSAGE, &ColorPath, " %s ", pFileName);
        Msg("does not have a supported file format for audio conversion!\n");
        g_uiFailedOperations++;
        return;
    }

    if (!g_pResourceCopy->CreateDir(pNewDir))
    {
        Warning("AudioProcess -> Could not create the new dir in: "); ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "%s\n", pNewDir);
        return;
    }

    Msg("\nAudioProcess -> Converting audio file: %s...\n", V_strrchr(pFileName, '\\') + 1);

    AVFormatContext* formatctx = nullptr;
    if (avformat_open_input(&formatctx, pFileName, nullptr, nullptr) < 0)
    {
        Warning("AudioProcess -> Could not open file: %s!\n", pFileName);
        g_uiFailedOperations++;
        return;
    }

    if (avformat_find_stream_info(formatctx, nullptr) < 0)
    {
        Warning("AudioProcess -> Could not find stream info: %s!\n", pFileName);
        avformat_close_input(&formatctx);
        g_uiFailedOperations++;
        return;
    }

    int iIndex = av_find_best_stream(formatctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (iIndex < 0)
    {
        Warning("AudioProcess -> No audio stream found: %s!\n", pFileName);
        avformat_close_input(&formatctx);
        g_uiFailedOperations++;
        return;
    }

    const AVCodec* decoder = avcodec_find_decoder(formatctx->streams[iIndex]->codecpar->codec_id);
    if (!decoder)
    {
        Warning("AudioProcess -> No decoder found for file: %s!\n", pFileName);
        avformat_close_input(&formatctx);
        g_uiFailedOperations++;
        return;
    }

    AVCodecContext* codecctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(codecctx, formatctx->streams[iIndex]->codecpar);

    if (avcodec_open2(codecctx, decoder, nullptr) < 0)
    {
        Warning("AudioProcess -> Failed to open codec: %s!\n", pFileName);
        avcodec_free_context(&codecctx);
        avformat_close_input(&formatctx);
        g_uiFailedOperations++;
        return;
    }

    if (g_eSpewMode != SpewMode::k_Quiet)
    {
        Msg("---- Information Audio File: %s ----\n", V_strrchr(pFileName, '\\') + 1);
        Msg("Codec:             %s\n", decoder ? decoder->long_name : "Unknown");
        Msg("Sample Rate:       %d\n", formatctx->streams[iIndex]->codecpar->sample_rate);
        Msg("Channels:          %d\n", formatctx->streams[iIndex]->codecpar->ch_layout.nb_channels);
        Msg("Bits Per Sample:   %d\n",
            av_get_bytes_per_sample((AVSampleFormat)formatctx->streams[iIndex]->codecpar->format) * 8);
        Msg("Duration:          %.2f seconds\n", (double)formatctx->duration / AV_TIME_BASE);
        Msg("Bitrate:           %lld kb/s\n", (long long)(formatctx->bit_rate / 1000));
        Msg("Channel Layout:    ");
        char szLayoutStr[128];
        av_channel_layout_describe(&formatctx->streams[iIndex]->codecpar->ch_layout, szLayoutStr, sizeof(szLayoutStr));
        Msg("%s\n", szLayoutStr);
        Msg("---------------------------------------------\n");
    }

    // Resample
    SwrContext* swr = swr_alloc();
    if (!swr)
    {
        Warning("AudioProcess -> Failed to allocate resampler!\n");
        avcodec_free_context(&codecctx);
        avformat_close_input(&formatctx);
        g_uiFailedOperations++;
        return;
    }

    AVChannelLayout in_ch_layout = codecctx->ch_layout;
    AVChannelLayout out_ch_layout;
    av_channel_layout_copy(&out_ch_layout, &codecctx->ch_layout);

    uint16_t outChannels = out_ch_layout.nb_channels;

    av_opt_set_chlayout(swr, "in_chlayout", &in_ch_layout, 0);
    av_opt_set_int(swr, "in_sample_rate", codecctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", codecctx->sample_fmt, 0);

    av_opt_set_chlayout(swr, "out_chlayout", &out_ch_layout, 0);
    av_opt_set_int(swr, "out_sample_rate", MAX_SAMPLE_RATE, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    if (swr_init(swr) < 0)
    {
        Warning("AudioProcess -> Failed to initialize resampler!\n");
        swr_free(&swr);
        avcodec_free_context(&codecctx);
        avformat_close_input(&formatctx);
        g_uiFailedOperations++;
        return;
    }

    // Output file 
    char pOutputPath[MAX_PATH];
    V_snprintf(pOutputPath, sizeof(pOutputPath), "%s\\%s", pNewDir, V_strrchr(pFileName, '\\') + 1);
    char* pDot = V_strrchr(pOutputPath, '.');
    if (pDot) *pDot = '\0';
    V_strcat_safe(pOutputPath, ".wav");

    FILE* pOut = fopen(pOutputPath, "wb");
    if (!pOut)
    {
        Warning("AudioProcess -> Failed to open output: %s!\n", pOutputPath);
        swr_free(&swr);
        avcodec_free_context(&codecctx);
        avformat_close_input(&formatctx);
        g_uiFailedOperations++;
        return;
    }

    uint8_t wavHeader[44] = { 0 };
    fwrite(wavHeader, 1, sizeof(wavHeader), pOut);

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    uint64_t totalSamples = 0;

    // Decode and convert
    while (av_read_frame(formatctx, pkt) >= 0)
    {
        if (pkt->stream_index == iIndex)
        {
            if (avcodec_send_packet(codecctx, pkt) == 0)
            {
                while (avcodec_receive_frame(codecctx, frame) == 0)
                {
                    int maxOutSamples = av_rescale_rnd(
                        swr_get_delay(swr, codecctx->sample_rate) + frame->nb_samples,
                        codecctx->sample_rate, codecctx->sample_rate, AV_ROUND_UP);

                    uint8_t* outData = nullptr;
                    int outLineSize = 0;

                    av_samples_alloc(&outData, &outLineSize, outChannels, maxOutSamples, AV_SAMPLE_FMT_S16, 0);

                    int converted = swr_convert(swr, &outData, maxOutSamples,
                        (const uint8_t**)frame->data, frame->nb_samples);

                    int bytes = converted * outChannels * 2;
                    fwrite(outData, 1, bytes, pOut);
                    totalSamples += converted;

                    av_freep(&outData);
                }
            }
        }
        av_packet_unref(pkt);
    }

    // Wav header
    uint32_t dataChunkSize  = totalSamples * outChannels * 2;
    uint32_t fileSize       = 36 + dataChunkSize;
    uint32_t sampleRate     = MAX_SAMPLE_RATE;
    uint32_t byteRate       = sampleRate * outChannels * 2;
    uint16_t blockAlign     = outChannels * 2;
    uint16_t bitsPerSample  = 16;
    uint16_t audioFormat    = 1;

    fseek(pOut,         0, SEEK_SET);
    fwrite("RIFF",      1, 4, pOut);
    fwrite(&fileSize,   4, 1, pOut);
    fwrite("WAVEfmt ",  1, 8, pOut);

    uint32_t fmtChunkSize = 16;
    fwrite(&fmtChunkSize,   4, 1, pOut);
    fwrite(&audioFormat,    2, 1, pOut);
    fwrite(&outChannels,    2, 1, pOut);
    fwrite(&sampleRate,     4, 1, pOut);
    fwrite(&byteRate,       4, 1, pOut);
    fwrite(&blockAlign,     2, 1, pOut);
    fwrite(&bitsPerSample,  2, 1, pOut);
    fwrite("data",          1, 4, pOut);
    fwrite(&dataChunkSize,  4, 1, pOut);

    fclose(pOut);

    if (g_eSpewMode != SpewMode::k_Quiet)
    {
        Msg("\n");
        Msg("---- Writing Audio File: %s ----\n", V_strrchr(pOutputPath, '\\') + 1);
        Msg("File Type:         WAV (PCM)\n");
        Msg("Audio Format:      PCM\n");
        Msg("Codec (Source):    %s\n", decoder ? decoder->long_name : "Unknown");
        Msg("Sample Rate:       %u Hz\n", sampleRate);
        Msg("Channels:          %u\n", outChannels);
        Msg("Bits Per Sample:   %u\n", bitsPerSample);
        Msg("Byte Rate:         %u bytes/s\n", byteRate);
        Msg("Block Align:       %u bytes\n", blockAlign);
        Msg("Duration:          %.2f seconds\n", (double)totalSamples / (double)sampleRate);
        Msg("Bitrate:           %.1f kb/s\n", (double)(byteRate * 8) / 1000.0);
        Msg("Channel Layout:    ");
        char layoutStr[128];
        av_channel_layout_describe(&out_ch_layout, layoutStr, sizeof(layoutStr));
        Msg("%s\n", layoutStr);
        Msg("---------------------------------------------\n\n");
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
    swr_free(&swr);
    av_channel_layout_uninit(&out_ch_layout);
    avcodec_free_context(&codecctx);
    avformat_close_input(&formatctx);

    Msg("AudioProcess -> Conversion complete: ");
    ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "%s\n", pOutputPath);
    g_uiCompletedOperations++;
}



//-----------------------------------------------------------------------------
// Purpose:   Print usage
//-----------------------------------------------------------------------------
static void PrintUsage(int argc, char* argv[])
{
    Msg("\nUsage: audioprocess.exe [options] <file> <path>\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, " General Options:\n");
    Msg("   -help or -?:           Print usage.\n"
        "   -s:                    If path mode is activaded, make a shallow scan.\n"
        "   -skip <substring>:     If a path has this substring skip the process. (Note: This can be use to exclude certain types of files like '.mp3')\n"
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, " Spew Options:\n");
    Msg("   -v or -verbose:        Enables verbose.\n"
        "   -q or -quiet:          Prints minimal text.\n"    
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, " Other Options:\n");
    Msg("   -FullMinidumps:        Write large minidumps on crash.\n"
        "\n");

    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(-1);
}


//-----------------------------------------------------------------------------
// Purpose:   Parse command line
//-----------------------------------------------------------------------------
static void ParseCommandline(int argc, char* argv[])
{
    if(argc == 1 || argc == 2)
    {
        PrintUsage(argc, argv);
    }

    for (int i = 1; i < argc; ++i)
    {
        V_FixSlashes(argv[i]);

        if (!V_stricmp(argv[i], "-?") || !V_stricmp(argv[i], "-help"))
        {
            PrintUsage(argc, argv);
        }
        else if (!V_stricmp(argv[i], "-v") || !V_stricmp(argv[i], "-verbose"))
        {
            verbose     = true;
            g_bQuiet    = !verbose;
        }       
        else if (!V_stricmp(argv[i], "-q") || !V_stricmp(argv[i], "-quiet"))
        {
            g_bQuiet    = true;
            verbose     = !g_bQuiet;
        }  
        else if (!V_stricmp(argv[i], "-FullMinidumps"))
        {
            EnableFullMinidumps(true);
        }
        else if (!V_stricmp(argv[i], "-skip"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* pGamePath = argv[i];
                if (!pGamePath)
                    Error("\nError: \'-skip\' requires a valid path argument. NULL path\n");
               
                ContainerString temp;
                V_strncpy(temp.data(), pGamePath, temp.size());
                g_pContainerSubStrings->push_back(temp);
                temp.data()[0] = '\0';
            }
            else
            {
                Error("\nError: \'-skip\' requires a valid path argument.\n");
            }
        }
        else
        {
            if (!(i == argc - 2 || i == argc - 1))
            {
                Warning("\nWarning Unknown option \'%s\'\n", argv[i]);
                PrintUsage(argc, argv);
            }
        }
    }
    
    // Check if the last argv is a path or a file.
    {
        const char* pFile = V_strrchr(argv[argc - 2], '\\') + 1;
        if (V_strstr(pFile, "."))
            g_bPathMode = false;
        else
            g_bPathMode = true;

    }

    if (!g_pResourceCopy->FileExist(argv[argc - 2]))
        Error("\nError: Null path, enter a valid path to process the audio files. (e.g: \"C:\\Users\\audiofiles\\test.wav\")\n");

    if (!g_pResourceCopy->FileExist(argv[argc - 1]))
        if (!g_pResourceCopy->CreateDir(argv[argc - 1]))
            Error("AudioProcess -> Could not create dir in: %s\n", argv[argc - 1]);
}


//-----------------------------------------------------------------------------
// Purpose:   Prints the header
//-----------------------------------------------------------------------------
static void PrintHeader()
{
    ColorSpewMessage(SPEW_MESSAGE, &ColorHeader, "Unusuario2 - AudioProcess (Build: %s %s)\n", __DATE__, __TIME__);
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

    // Load the file system
    {
        CSysModule* pFileSystem = Sys_LoadModule("filesystem_stdio.dll");
        if (!pFileSystem)
            Error("Failed to load filesystem_stdio.dll!\n");

        CreateInterfaceFn factory = Sys_GetFactory(pFileSystem);
        if (!factory)
            Error("Failed to create filesystem_stdio.dll interface!\n");

        g_pFileSystem = (IBaseFileSystem*)factory(BASEFILESYSTEM_INTERFACE_VERSION, NULL);
        g_pFullFileSystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL);
    }

    g_pResourceCopy = new CResourceCopy();
    g_pContainerSubStrings = new ContainerList();

    ParseCommandline(argc, argv);

    if (verbose)
        g_eSpewMode = SpewMode::k_Verbose;
    else if (g_bQuiet)
        g_eSpewMode = SpewMode::k_Quiet;
    else
        g_eSpewMode = SpewMode::k_Normal;

    g_pVAudioFormats = new ContainerList();
    {
        const ContainerList VTempList =
        {
            ContainerString{".mp3"},
            ContainerString{".aac"},
            ContainerString{".m4a"},
            ContainerString{".flac"},
            ContainerString{".wav"},
            ContainerString{".ogg"},
            ContainerString{".opus"},
            ContainerString{".wma"},
            ContainerString{".alac"},
            ContainerString{".ac3"},
            ContainerString{".aiff"},
            ContainerString{".aif"},
            ContainerString{".amr"},
            ContainerString{".mp2"},
            ContainerString{".ape"},
            ContainerString{".tta"},
            ContainerString{".tak"},
            ContainerString{".wv"},
            ContainerString{".mlp"},
            ContainerString{".alaw"},
            ContainerString{".ulaw"},
            ContainerString{".gsm"},
            ContainerString{".qcp"},
            ContainerString{".mod"},
            ContainerString{".xm"},
            ContainerString{".it"},
            ContainerString{".s3m"}
        };

        for (const auto& Ext : VTempList)
            g_pVAudioFormats->push_back(Ext);
    }

    // Unusuario2 - Sanity check! Does the FFmpeg dlls exist?? (avoid crashes)
    // Unusuario2 - If you update the ffmpeg version update also the dlls!!
    const char* rgpDllDepencies[] = { "swresample-6.dll", "avutil-60.dll", "avformat-62.dll", "avcodec-62.dll" };

    char szBinPath[MAX_PATH];
    GetModuleFileNameA(NULL, szBinPath, MAX_PATH);
    *(V_strrchr(szBinPath, '\\')) = '\0';

    for (const char* pDll : rgpDllDepencies)
    {
        char szDllFullPath[MAX_PATH];
        V_sprintf_safe(szDllFullPath, "%s\\%s", szBinPath, pDll);
        if (!g_pResourceCopy->FileExist(szDllFullPath)) {
            Error("AudioProcess -> Missing .dll module: %s!\n"
                "AudioProcess -> Expected .dll path: %s\n",
                szDllFullPath, pDll);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static void Destroy(int argc, char* argv[])
{
    if(g_eSpewMode > SpewMode::k_Quiet)
    {
        Msg("-------------------------------------------------------------------------------------------\n");
        Msg("| AudioProcess -> Done in %.2f seconds. | ",   Plat_FloatTime() - g_flStartTime);
        ColorSpewMessage(SPEW_MESSAGE, &ColorSucesfull,     "Completed: %i,     ",  g_uiCompletedOperations);
        ColorSpewMessage(SPEW_MESSAGE, &ColorUnSucesfull,   "Error: %i,     ",      g_uiFailedOperations);
        ColorSpewMessage(SPEW_MESSAGE, &ColorWarning,       "Skipped: %i         ", g_uiSkippedOperations);
        Msg("\n");
        Msg("-------------------------------------------------------------------------------------------\n");
    }

    delete g_pResourceCopy;
    delete g_pContainerSubStrings;
    delete g_pVAudioFormats;
    
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

    if (g_bPathMode)
    {
        ContainerList* VAudioFilesToConvert = nullptr;
        VAudioFilesToConvert = new ContainerList();
        VAudioFilesToConvert->reserve(1000); // Avoid some realloc

        for (int i = 0; i < g_pVAudioFormats->size(); ++i)
        {
            float start = Plat_FloatTime();
            char szWildCard[MAX_PATH];
            V_sprintf_safe(szWildCard, "%s\\*%s", argv[argc - 2], g_pVAudioFormats->at(i).data());

            if (g_eSpewMode != SpewMode::k_Quiet) { Msg("AudioProcess -> Scaning dir: "); ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "%s", szWildCard); Msg("... "); }

            ContainerList VTemp = g_pResourceCopy->ScanDirectoryRecursive(szWildCard);
            VAudioFilesToConvert->insert(VAudioFilesToConvert->end(), VTemp.begin(), VTemp.end());
            
            if (g_eSpewMode != SpewMode::k_Quiet) { Msg("done(%.2f)\n", Plat_FloatTime() - start); }
        }

        if (VAudioFilesToConvert->size() == 0)
        {
            Warning("AudioProcess -> No files to convert!\n");
            Destroy(argc, argv);
            return 0;
        }
        else
        {
            Msg("AudioProcess -> Files to convert: %llu\n\n", VAudioFilesToConvert->size());
        }

        for (int i = 0; i < VAudioFilesToConvert->size(); ++i) 
        {
            char szNewDir[MAX_PATH];
            {
                const char* pBasePath = argv[argc - 2];
                const char* pFileFullPath = VAudioFilesToConvert->at(i).data();
                const char* pRelativePath = &pFileFullPath[V_strlen(pBasePath) + 1];
                char* pDir = V_strdup(pRelativePath);
                char* pFileName = V_strrchr(pDir, '\\');
                if (pFileName) 
                {
                    *(pFileName) = '\0';
                    V_sprintf_safe(szNewDir, "%s\\%s", argv[argc - 1], pDir);
                }
                else 
                {
                    V_sprintf_safe(szNewDir, "%s", argv[argc - 1]);
                }
                delete[] pDir;
            }
            ConvertAudioFileToWav(VAudioFilesToConvert->at(i).data(), szNewDir);
        }

        delete VAudioFilesToConvert;
    }
    else
    {
        ConvertAudioFileToWav(argv[argc - 2], argv[argc - 1]);
    }

    Destroy(argc, argv);
    return 0;
}

