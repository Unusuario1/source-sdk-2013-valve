#ifndef CONTENTBUILDER_H
#define CONTENTBUILDER_H
#include <cstddef>
#include "basetypes.h"

extern std::size_t g_timer;
extern qboolean verbose;
extern bool g_infocontent;
extern bool g_nosteam;
extern bool g_ignoreerrors;
extern bool g_force32bits;
extern bool g_force64bits;
extern bool g_spewallcommands;
extern bool g_cleanuptempcontent;
extern char g_gameinfodir[MAX_PATH];
extern char g_gamedir[MAX_PATH];
extern char g_steamdir[MAX_PATH];

#endif // CONTENTBUILDER_H