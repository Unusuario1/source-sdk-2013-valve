#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "materialbuilder.h"
#include "shared.h"


/*
		MaterialCompile
		{

		}
*/
namespace MaterialBuilder 
{
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_MATERIAL_TOOL, "MaterialBuilder");
	}


	void LoadGameInfoKv(char *tool_argv, std::size_t bufferSize)
	{
		//by default we start vtex with: -nopause -deducepath -crcvalidate
		const char _argv1[] = " -nopause -deducepath -crcvalidate ";
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(MATERIALBUILDER_KV, _argv2, sizeof(_argv2));
		
		V_snprintf(tool_argv, bufferSize, " %s %s ", _argv1, _argv2);
	}


	void MaterialCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		char tool_commands[4096] = "";
		//Copy the vmt files from materialsrc to materials

		MaterialBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));
		Shared::StartExe(gamebin, bufferSize, "Materials", NAME_MATERIAL_TOOL, tool_commands, complete, error);
	}
}

