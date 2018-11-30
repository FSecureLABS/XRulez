// Module's precompiled header pack.
#pragma once
#include "PreModule.hpp"
#define WINTOOLSMODULE_HH_INCLUDED

#if !defined(INSIDE_PRECOMPILED_HEADER)
#	pragma message (COMPOSE_FILE_LINE_AND_TEXT("Warning: WinTools Module's precompiled header pack is included outside the precompiled header. It will have a negative impact on build performance."))
#endif

// Preprocessor symbols.
#define _WIN32_DCOM

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other includes.
#include <Windows.h>															//< Windows as a target platform.
#include <WinError.h>															//< WinApi error code definitions and macros like SUCCEEDED.
#include <ShlObj.h>																//< For SHGetFolderPath and SHGetKnownFolderPath functions.
#include <TlHelp32.h>															//< PROCESSENTRY32, etc.
#include <ComDef.h>																//< For _com_error class.
#include <TaskSchd.h>															//< Microsoft Windows Task Scheduler DCOM.
#include "Shlwapi.h"															//< For PathFileExists.

// Static libraries.
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "Shlwapi.lib")
