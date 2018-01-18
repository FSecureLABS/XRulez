#pragma once

// Module dependencies.
#include "CppTools/CppToolsModule.h"

// Module's precompiled header pack inclusion.
#if !defined(ENVIROMODULE_HH_INCLUDED)
#	include "EnviroModule.hh"
#endif

/// Asserts.
#define CHECK(x) // ASSERT(x)

#include <iostream>
/// @brief Program build and run environment.
namespace Enviro
{
#	if defined PLATFORM_WINDOWS && PLATFORM_WINDOWS == YES
	extern const bool IsDllBuild;												///< Indicates if the code is (being) compiled as an executable or a dynamic link library.
	extern const bool IsUnicodeBuild;											///< Indicates if the code is (being) compiled in UNICODE.

	// Valid only when running from a DLL (m_IsRunningInDll = true), otherwise nullptr's will be exposed.
	// They are not between #ifdef TARGET_BINARY_IMAGE_TYPE_DLL to allow having them in source code without, again, putting every call between #ifdef TARGET_BINARY_IMAGE_TYPE_DLL.
	DWORD*& AccessDllLoadReason();												///< Pointer to a handle to the DLL module. @see https://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx.
	HINSTANCE*& AccessDllInstance();											///< Pointer to a the reason code that indicates why the DLL entry-point function is being called. @see https://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx.
	LPVOID*& AccessDllReservedParam();											///< It's meaning depend on DllInputLoadReason. @see https://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx.
#	endif

	/// Valid only when running from an EXE (m_IsRunningInDll = false), otherwise values assigned below will be exposed.
	std::vector<std::tstring>& AccessCommandLineParams();

	// Just simple redirectors, however note that we can't use anything other than std::wcout after calling _setmode(_fileno(stdout), _O_U16TEXT);.
	extern decltype(std::wcout)& cout;
	extern decltype(std::wcout)& wcout;
	extern decltype(std::wcout)& tcout;
	extern decltype(std::wcout)& cerr;
	extern decltype(std::wcout)& wcerr;
	extern decltype(std::wcout)& tcerr;

	extern decltype(std::cin)& cin;
	extern decltype(std::wcin)& wcin;
	
#	ifdef _UNICODE
		extern decltype(std::wcin)& tcin;
#	else
		extern decltype(std::cin)& tcin;
#	endif
}

/// Real entry point declaration. You must define this global function in order to use Enviro Module.
extern int Main();
