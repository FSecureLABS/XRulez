#include "StdAfx.h"
#include "WinToolsModule.h"

// Useful macros for DCOM/Task Scheduler related functions.
#	define PROGRESS(d,c,g) d; if (retVal = c, FAILED(retVal)) return retVal; SCOPE_GUARD{ g; };
#	define RETURN_IF_FAILED(c) if (retVal = c, FAILED(retVal)) return retVal;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LONGLONG WinTools::ConvertFileTimetoLonglong(FILETIME fileTime)
{
	// Perform scaling on the QuadPart (LONGLONG) of fileTime.
	return (LARGE_INTEGER{ fileTime.dwLowDateTime, static_cast<LONG>(fileTime.dwHighDateTime) }.QuadPart - 116444736000000000LL) / 10000000LL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::tstring WinTools::ConvertHresultToMessage(HRESULT hr)
{
	// Use temporary _com_error object to translate HRESULT to human readable message.
	return std::tstring(_com_error(hr).ErrorMessage());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::tstring WinTools::ConvertWinErrorToMessage(DWORD errorCode)
{
	TCHAR buffer[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, static_cast<DWORD>(std::size(buffer)), NULL);

	return buffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::tstring WinTools::ConvertWinErrorToMessage(HMODULE module, DWORD errorCode)
{
	// Call FormatMessage to search for error string inside provided module. If that fails, search in "system" string table.
	TCHAR buffer[1024];
	if (!FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, module, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, static_cast<DWORD>(std::size(buffer)), NULL) && ERROR_RESOURCE_TYPE_NOT_FOUND == GetLastError())
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, static_cast<DWORD>(std::size(buffer)), NULL);

	return buffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::tstring WinTools::ConvertHresultToMessageWithHresult(HRESULT hr)
{
	// Add HRESULT as a hexadecimal number and forward call to WinTools::ConvertHresultToMessage().
	return CppTools::StringConversions::NumberToHexTString(hr) + std::tstring(TEXT(" : ")) + WinTools::ConvertHresultToMessage(hr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::tstring WinTools::ConvertWinErrorToMessageWithWinError(HMODULE module, DWORD errorCode)
{
	// Add WinError as a hexadecimal number and forward call to WinTools::ConvertWinErrorToMessage().
	return std::to_tstring(errorCode) + std::tstring(TEXT(" : ")) + WinTools::ConvertWinErrorToMessage(module, errorCode);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::tstring WinTools::ConvertWinErrorToMessageWithWinError(DWORD errorCode)
{
	// Add WinError as a hexadecimal number and forward call to WinTools::ConvertWinErrorToMessage().
	return std::to_tstring(errorCode) + std::tstring(TEXT(" : ")) + WinTools::ConvertWinErrorToMessage(errorCode);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::GetAppDataFolder(OUT WCHAR returnPath[MAX_PATH])
{
	// Because we're using XP compatibility, call SHGetFolderPathW.
	return CppTools::XErrorCreator(SHGetFolderPathW(NULL, CSIDL_APPDATA, nullptr, 0, returnPath));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::filesystem::path WinTools::GetAppDataFolder()
{
	std::array<wchar_t, MAX_PATH> buffer;
	CallWinApiHr(SHGetFolderPathW(NULL, CSIDL_APPDATA, nullptr, 0, buffer.data()));
	return buffer.data();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::filesystem::path WinTools::GetTempFolder() noexcept(false)
{
	std::array<TCHAR, MAX_PATH> buffer;
	auto length = GetTempPath(static_cast<DWORD>(buffer.size()), buffer.data());
	if (!length OR length > static_cast<DWORD>(buffer.size()))
		throw WinApiHrException(GetLastError(), XEXCEPTION_PARAMS, "GetTempPath()");

	return buffer.data();
}

std::filesystem::path WinTools::GetStartupFolder() noexcept(false)
{
	PWSTR pszPath;
	CallWinApiHr(SHGetKnownFolderPath(FOLDERID_Startup, 0, NULL, &pszPath));
	std::filesystem::path location = { pszPath };
	CoTaskMemFree(static_cast<LPVOID>(pszPath));
	return location;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*CppTools::XError<HRESULT> WinTools::GetAppDataFolder(OUT WCHAR returnPath[MAX_PATH])
{
	// There was an API switch on Microsoft Windows Vista - SHGetFolderPathW should be considered deprecated on that and younger systems.
/*	if (IsWindowsVistaOrGreater())
	{
		// Call SHGetKnownFolderPath.
		PWSTR path = nullptr;
		auto xe = CppTools::XErrorCreator(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DONT_UNEXPAND, nullptr, &path));
		SCOPE_GUARD{ if (path) CoTaskMemFree(path); };

		// Copy string as the buffer pointed by path must be freed.
		auto dummy = wcscpy_s(returnPath, MAX_PATH, path);

		// Debug validation.
		CHECK(dummy);
		UNUSED_PARAM(dummy);													//< Suppress warning in release builds.

																				// Return the value (got from SHGetKnownFolderPath call).
		return xe;
	}

	// For systems prior to Microsoft Windows Vista call SHGetFolderPathW.
	return CppTools::XErrorCreator(SHGetFolderPathW(NULL, CSIDL_APPDATA, nullptr, 0, returnPath));
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring WinTools::PathAppend(const std::wstring& first, const std::wstring& second)
{
	if (first.back() == L'\\' OR first.back() == L'/')
		return first + second.substr(second.front() == L'\\' OR second.front() == L'/' ? 1 : 0);

	return first + (second.front() == L'\\' OR second.front() == L'/' ? L'\0' : L'/') + second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XErrorEnumWithHresult<WinTools::EnumerateFilesRet> WinTools::EnumerateFiles(const std::wstring& path, EnumerateFilesCallback_t callback, void* userData)
{
	// Note that independently of configuration, we are always using wide strings for paths.
	WIN32_FIND_DATAW fileData;

	// Get first entry that matches path criteria.
	auto fileHandle = FindFirstFileW(path.c_str(), &fileData);
	if (fileHandle == INVALID_HANDLE_VALUE)
		return CppTools::XErrorEnumWithHresultCreator(EnumerateFilesRet::FindFirstFileW, WinTools::SystemErrorToHresult(GetLastError()));

	SCOPE_GUARD{ FindClose(fileHandle); };

	// Call the callback function for every entity unless it returns false.
	while (callback(userData, fileData) && FindNextFileW(fileHandle, &fileData));

	// Return success if we got to the end of the file list, otherwise return error code.
	return CppTools::XErrorEnumWithHresultCreator(EnumerateFilesRet::FindNextFileW, GetLastError() == ERROR_NO_MORE_FILES ? S_OK : WinTools::SystemErrorToHresult(GetLastError()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XErrorEnumWithHresult<WinTools::EnumerateProcessesRet> WinTools::EnumerateProcesses(EnumerateProcessesCallback_t callback, void* userData)
{
	// Take a snapshot of all processes in the system.
	auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapshot == INVALID_HANDLE_VALUE)
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProcessesRet::CreateToolhelp32Snapshot, WinTools::SystemErrorToHresult(GetLastError()));

	SCOPE_GUARD{ CloseHandle(snapshot); };

	// Retrieve information about the first process.
	PROCESSENTRY32 processData { sizeof(PROCESSENTRY32) };
	if (Process32First(snapshot, &processData))
		while (callback(userData, processData) && Process32Next(snapshot, &processData));			//< Call the callback function for every entity unless it returns false.
	else
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProcessesRet::Process32First, WinTools::SystemErrorToHresult(GetLastError()));

	// Return success if we got to the end of the process list, otherwise return error code.
	return CppTools::XErrorEnumWithHresultCreator(EnumerateProcessesRet::Process32Next, GetLastError() == ERROR_NO_MORE_FILES ? S_OK : WinTools::SystemErrorToHresult(GetLastError()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::CheckIfPathExists(std::wstring const& path)
{
	return PathFileExistsW(path.c_str()) ? S_OK : XERROR_GETLASTERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::DeleteSpecifiedFile(std::wstring const& path)
{
	return ::DeleteFileW(path.c_str()) ? S_OK : XERROR_GETLASTERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::CreateLnk(const std::wstring& pathToTarget, const std::wstring& pathToLnk, const std::wstring& targetCommandLine)
{
	// Declare return value.
	HRESULT retVal;

	// Initialize COM.
	if (retVal = ::CoInitialize(nullptr), FAILED(retVal))
		return retVal;
	SCOPE_GUARD{ ::CoUninitialize(); };

	// Get a pointer to the IShellLink interface.
	IShellLinkW* lnk;
	if (retVal = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&lnk), FAILED(retVal))
		return retVal;
	SCOPE_GUARD{ lnk->Release(); };

	// Set shortcut's parameters.
	lnk->SetPath(pathToTarget.c_str());
	lnk->SetArguments(targetCommandLine.c_str());

	// Query IShellLink for the IPersistFile interface, used for saving the shortcut in persistent storage.
	IPersistFile* persistFile;
	if (retVal = lnk->QueryInterface(IID_IPersistFile, (LPVOID*)&persistFile), FAILED(retVal))
		return retVal;
	SCOPE_GUARD{ persistFile->Release(); };

	// Save the link by calling IPersistFile::Save.
	return persistFile->Save(pathToLnk.c_str(), TRUE);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::ScheduleBootTimeTask(const std::wstring& taskName, const std::wstring& pathToTarget, std::wstring const& targetCommandLine, bool overwriteIfExists)
{
	// Initialize COM.
	PROGRESS(HRESULT retVal, ::CoInitializeEx(nullptr, COINIT_MULTITHREADED), ::CoUninitialize())

	// Set general COM security levels.
	RETURN_IF_FAILED(CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr))

	// Create an instance of the Task Service.
	PROGRESS(ITaskService *service = nullptr, CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&service), service->Release())

	// Connect to the Task Service.
	RETURN_IF_FAILED(service->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t()))

	// Get the pointer to the root task folder. This folder will hold the new task that is registered.
	PROGRESS(ITaskFolder *rootFolder = nullptr, service->GetFolder(_bstr_t(L"\\"), &rootFolder), rootFolder->Release())

	if (!overwriteIfExists)
	{
		// Query for specified task exists.
		IRegisteredTask* taskCheck = nullptr;
		retVal = rootFolder->GetTask(_bstr_t(taskName.c_str()), &taskCheck);

		if (retVal != WinTools::SystemErrorToHresult(ERROR_FILE_NOT_FOUND))
			return SUCCEEDED(retVal) ? ERROR_ALREADY_EXISTS : retVal;
	}

	// Create the task builder object to create the task.
	PROGRESS(ITaskDefinition *task = nullptr, service->NewTask(0, &task), task->Release())

	// Get the registration info for setting the identification.
	PROGRESS(IRegistrationInfo *regInfo = nullptr, task->get_RegistrationInfo(&regInfo), regInfo->Release())

	// Create the settings for the task, and set its values.
	PROGRESS(ITaskSettings *settings = nullptr, task->get_Settings(&settings), settings->Release())
	RETURN_IF_FAILED(settings->put_StartWhenAvailable(VARIANT_TRUE))
	RETURN_IF_FAILED(settings->put_StopIfGoingOnBatteries(VARIANT_FALSE))
	RETURN_IF_FAILED(settings->put_DisallowStartIfOnBatteries(VARIANT_FALSE))

	// Get the trigger collection to insert the boot trigger.
	PROGRESS(ITriggerCollection *triggerCollection = nullptr, task->get_Triggers(&triggerCollection), triggerCollection->Release())

	// Add the boot trigger to the task.
	PROGRESS(ITrigger *trigger = nullptr, triggerCollection->Create(TASK_TRIGGER_BOOT, &trigger), trigger->Release())
	PROGRESS(IBootTrigger *bootTrigger = nullptr, trigger->QueryInterface(IID_IBootTrigger, (void**)&bootTrigger), bootTrigger->Release())

	// Set the task to start at a certain time. The time format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
	RETURN_IF_FAILED(bootTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00")))
	RETURN_IF_FAILED(bootTrigger->put_EndBoundary(_bstr_t(L"2044-04-04T04:44:40")))

	// Add an Action to the task. First, get the task action collection pointer.
	PROGRESS(IActionCollection *actionCollection = nullptr, task->get_Actions(&actionCollection), actionCollection->Release())

	// Create the Action, specifying it as an executable action.
	PROGRESS(IAction *action = nullptr, actionCollection->Create(TASK_ACTION_EXEC, &action), action->Release())

	// QI for the executable task pointer.
	PROGRESS(IExecAction *execAction = nullptr, action->QueryInterface(IID_IExecAction, (void**)&execAction), execAction->Release())

	// Set the path and command line args for the executable.
	RETURN_IF_FAILED(execAction->put_Path(_bstr_t(pathToTarget.c_str())))
	RETURN_IF_FAILED(execAction->put_Arguments(_bstr_t(targetCommandLine.c_str())))

	// Save the task in the root folder.
	IRegisteredTask *registeredTask = NULL;
	VARIANT password;
	password.vt = VT_EMPTY;
	return rootFolder->RegisterTaskDefinition(_bstr_t(taskName.c_str()), task, TASK_CREATE_OR_UPDATE, _variant_t(L"Local Service"), password, TASK_LOGON_SERVICE_ACCOUNT, _variant_t(L""), &registeredTask);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::GetCurrentProcessName(OUT std::wstring& name)
{
	// Create buffer long enough to contain full path.
	wchar_t buffer[MAX_PATH];
	DWORD len = MAX_PATH;

	// Try and retrieve current process image full path.
	if (auto hr = QueryFullProcessImageNameW(GetCurrentProcess(), 0, buffer, &len))
		return hr;

	// Get rid of path-part.
	name = buffer;
	auto lastSlash = name.find_last_of(L"/\\");
	if (lastSlash != std::string::npos)
		name = name.substr(lastSlash + 1);

	// Return success.
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::DeleteScheduledTask(const std::wstring& taskName)
{
	// Initialize COM.
	PROGRESS(HRESULT retVal, ::CoInitializeEx(nullptr, COINIT_MULTITHREADED), ::CoUninitialize())

	// Set general COM security levels.
	RETURN_IF_FAILED(CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr))

	// Create an instance of the Task Service.
	PROGRESS(ITaskService *service = nullptr, CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, reinterpret_cast<void**>(&service)), service->Release())

	// Connect to the Task Service.
	RETURN_IF_FAILED(service->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t()))

	// Get the pointer to the root task folder. This folder will hold the new task that is registered.
	PROGRESS(ITaskFolder *rootFolder = nullptr, service->GetFolder(_bstr_t(L"\\"), &rootFolder), rootFolder->Release())

	// If the same task exists, remove it.
	return rootFolder->DeleteTask(_bstr_t(taskName.c_str()), 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::CheckIfScheduledTaskExists(const std::wstring& taskName)
{
	// Initialize COM.
	PROGRESS(HRESULT retVal, ::CoInitializeEx(nullptr, COINIT_MULTITHREADED), ::CoUninitialize())

	// Set general COM security levels.
	RETURN_IF_FAILED(CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, 0, nullptr))

	// Create an instance of the Task Service.
	PROGRESS(ITaskService *service = nullptr, CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&service), service->Release())

	// Connect to the Task Service.
	RETURN_IF_FAILED(service->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t()))

	// Get the pointer to the root task folder. This folder will hold the new task that is registered.
	PROGRESS(ITaskFolder *rootFolder = nullptr, service->GetFolder(_bstr_t(L"\\"), &rootFolder), rootFolder->Release())

	// Query for specified task exists.
	PROGRESS(IRegisteredTask* taskCheck = nullptr, rootFolder->GetTask(_bstr_t(taskName.c_str()), &taskCheck), taskCheck->Release());

	// If we're here then specified task already exists.
	return S_OK;
}

// Undefine macros.
#	undef PROGRESS
#	undef RETURN_IF_FAILED
