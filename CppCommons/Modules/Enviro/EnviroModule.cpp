#include "StdAfx.h"

// Modules.
#include "EnviroModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Statics.
decltype(std::wcout)& Enviro::cout = std::wcout;
decltype(std::wcout)& Enviro::wcout = std::wcout;
decltype(std::wcout)& Enviro::tcout = std::wcout;
decltype(std::wcout)& Enviro::cerr = std::wcout;
decltype(std::wcout)& Enviro::wcerr = std::wcout;
decltype(std::wcout)& Enviro::tcerr = std::wcout;

decltype(std::cin)& Enviro::cin = std::cin;
decltype(std::wcin)& Enviro::wcin = std::wcin;

#ifdef _UNICODE
	decltype(std::wcin)& Enviro::tcin = std::wcin;
#else
	decltype(std::cin)& Enviro::tcin = std::cin;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined PLATFORM_WINDOWS && PLATFORM_WINDOWS == YES

	// Define IsUnicodeBuild.
#	ifdef _UNICODE
		namespace Enviro { const bool IsUnicodeBuild = true; }
#	else
		namespace Enviro { const bool IsUnicodeBuild = false; }
#	endif

	// Define IsDllBuild and appropriate application entry point.
#	if defined(TARGET_BINARY_IMAGE_TYPE) && TARGET_BINARY_IMAGE_TYPE == TARGET_BINARY_IMAGE_TYPE_DLL
		namespace Enviro { const bool IsDllBuild = true; }

		// DllMain.
		BOOL WINAPI DllMain(HINSTANCE instace, DWORD reason, LPVOID reserved)
		{
			// Copy input params.
			Enviro::AccessDllInstance() = &instace;
			Enviro::AccessDllLoadReason() = &reason;
			Enviro::AccessDllReservedParam() = &reserved;
#	else
		namespace Enviro { const bool IsDllBuild = false; }

		// Console application's main.
		int main()
		{
			// Enable >proper< UNICODE character rendering for console window. WARNING: do not use std::cout, std::cerr, std::wcerr anymore! Use Enviro::cout/wcerr/etc. instead, EVERYWHERE (even std::cout << "mbcs!" drops assertion).
			_setmode(_fileno(stdout), _O_U16TEXT);

			// Copy command line params.
#			if defined COMPILER_MSVC && COMPILER_MSVC == YES
#				ifdef UNICODE
					if (__wargv)
						Enviro::AccessCommandLineParams() = std::vector<std::tstring>(__wargv, __wargv + __argc);
					else
						for (signed i = 0; i < __argc; ++i)
							Enviro::AccessCommandLineParams().push_back(CppTools::StringConversions::Mbcs2Unicode(__argv[i]));
#				else
					if (__argv)
						Enviro::AccessCommandLineParams() = std::vector<std::tstring>(__argv, __argv + __argc);
					else
						for (signed i = 0; i < __argc; ++i)
							Enviro::AccessCommandLineParams().push_back(CppTools::StringConversions::Unicode2Mbcs(__wargv[i]));
#				endif
#			else
#				error Compilers other than Microsoft Visual Studio are currently not supported.
#			endif
#	endif

		// Call the real main function.
		return Main();
	}

	// We are not using static globals because when loaded reflectively statics are not initialized properly.
	DWORD*& Enviro::AccessDllLoadReason() { static DWORD* reason; return reason; }
	HINSTANCE*& Enviro::AccessDllInstance() { static HINSTANCE* instance; return instance; }
	LPVOID*& Enviro::AccessDllReservedParam() { static LPVOID* reserved; return reserved; }
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::tstring>& Enviro::AccessCommandLineParams()
{
	// We have to do it that way (not by just using global static vector<string>) because of stl bugs which disallows its structures to be initialized before main.
	static std::vector<std::tstring> CommandLineParams;

	return CommandLineParams;
}
