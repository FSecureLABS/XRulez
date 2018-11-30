#pragma once

// Module dependencies.
#include "Enviro/EnviroModule.h"

// Module's precompiled header pack inclusion.
#if !defined(WINTOOLSMODULE_HH_INCLUDED)
#	include "WinTools/WinToolsModule.hh"
#endif

// Registry namespace.
#include "Registry.h"

// Services namespace.
#include "Services.h"

/// @brief Run-time tools for Microsoft Windows systems.
/// @note Note that independently of configuration, we are always using wide strings for paths.
namespace WinTools
{
	/// Release functor.
	struct ComReleaseFunctor { void operator() (IUnknown * pointer) const { pointer->Release(); } };

	/// Template alias for ReleaseFunktor-unique_ptr.
	template <class Interface> using UniqueComPtr = std::unique_ptr<Interface, ComReleaseFunctor>;

	/// Converts Win32 API time stamp (number of 100-nanosecond intervals since January 1, 1601 UTC) to Unix time stamp (number of seconds since Jan 1, 1970).
	/// @param fileTime WinApi time stamp to convert.
	/// @return Time in POSIX format.
	LONGLONG ConvertFileTimetoLonglong(FILETIME fileTime);

	/// Translates HRESULT to human readable message.
	/// @param hr HRESULT value to translate.
	/// @return String containing human readable message.
	/// @see ConvertHresultToMessageWithHresult.
	std::tstring ConvertHresultToMessage(HRESULT hr);

	/// Translates Windows system error code to human readable message.
	/// @param module handle to module where strings are stored.
	/// @param errorCode error number.
	/// @return String containing human readable message.
	/// @see ConvertHresultToMessageWithHresult.
	std::tstring ConvertWinErrorToMessage(HMODULE module, DWORD errorCode);

	/// Translates Windows system error code to human readable message.
	/// @param errorCode error number.
	/// @return String containing human readable message.
	/// @see ConvertHresultToMessageWithHresult.
	std::tstring ConvertWinErrorToMessage(DWORD errorCode);

	/// Translates HRESULT to human readable message that contains provided HRESULT value.
	/// @param hr HRESULT value to translate.
	/// @return String containing human readable message with provided HRESULT value.
	/// @see ConvertHresultToMessage.
	std::tstring ConvertHresultToMessageWithHresult(HRESULT hr);

	/// Translates Windows system error code to human readable message that contains provided error value.
	/// @param module handle to module where strings are stored.
	/// @param errorCode error number.
	/// @return String containing human readable message with provided WinError value.
	/// @see ConvertWinErrorToMessage.
	std::tstring ConvertWinErrorToMessageWithWinError(HMODULE module, DWORD errorCode);

	/// Translates Windows system error code to human readable message that contains provided error value.
	/// @param errorCode error number.
	/// @return String containing human readable message with provided WinError value.
	/// @see ConvertWinErrorToMessage.
	std::tstring ConvertWinErrorToMessageWithWinError(DWORD errorCode);

	/// Retrieves %APPDATA% folder.
	/// @param returnPath wide string that will contain Application Data folder URL on success.
	/// @return Forwarded value returned by SHGetKnownFolderPath (on Microsoft Windows Vista+ systems) or SHGetFolderPathW (otherwise).
	CppTools::XError<HRESULT> GetAppDataFolder(OUT WCHAR returnPath[MAX_PATH]);

	/// Retrieves %APPDATA% folder. Throws on error.
	/// @return Temporary folder location. Throws WinApiHrException as a forwarded value returned by SHGetKnownFolderPath (on Microsoft Windows Vista+ systems) or SHGetFolderPathW (otherwise).
	std::filesystem::path GetAppDataFolder() noexcept(false);

	/// Retrieves %TEMP% folder.
	/// @return Temporary folder location. Throws WinApiHrException if GetTempPath() failed.
	std::filesystem::path GetTempFolder() noexcept(false);

	/// Retrieves Startup folder.
	/// @return Startup folder location. Throws WinApiHrException on failure.
	std::filesystem::path GetStartupFolder() noexcept(false);

	/// EnumerateFilesCallback_t function prototype, used by EnumerateFiles function. This callback is called for every file that matches path criteria.
	/// @param userData pointer that was provided when calling EnumerateProfiles.
	/// @param fileData a WIN32_FIND_DATAW structure describing one file that matches provided criteria.
	/// @return If false is returned then further enumeration is not performed.
	/// @note Note that independently of configuration, we are always using wide strings for paths.
	typedef std::function<bool(void* userData, const WIN32_FIND_DATAW& processData)> EnumerateFilesCallback_t;

	/// Return type for IsOutlookRunning function. @see IsOutlookRunning.
	enum class EnumerateFilesRet
	{
		FindFirstFileW,															///< EnumerateFiles failed calling FindFirstFileW.
		FindNextFileW,															///< Corresponding HRESULT indicates if the last call to FindNextFileW was successful or not. Note that this HRESULT
																				///  value is a translation from winerror (GetLastError()) @see EnumerateFiles().
	};

	/// Enlists all files that matches provided path criteria. Accepts wild-cards.
	/// @param path path (including wild-cards) that will be used as a mask for files to find.
	/// @param callback callback function that is called for each profile.
	/// @param userData pointer that will be passed to the callback function every time it's called. The meaning of this param is user defined (f.e. it might be a handle to an output window).
	/// @return FindNextFileW/S_OK if callback function returned false at some point or if reached the end of the file list. For other cases @see EnumerateProfileFilesRet.
	CppTools::XErrorEnumWithHresult<WinTools::EnumerateFilesRet> EnumerateFiles(const std::wstring& path, EnumerateFilesCallback_t callback, void* userData);

	/// EnumerateProcessesCallback_t function prototype, used by EnumerateProcesses function. This callback is called for every running process.
	/// @param userData pointer that was provided when calling EnumerateProcesses.
	/// @param processData a PROCESSENTRY32 structure describing one process.
	/// @return If false is returned then further enumeration is not performed.
	typedef std::function<bool(void* userData, const PROCESSENTRY32& processData)> EnumerateProcessesCallback_t;

	/// Return type for EnumerateProcesses function. @see EnumerateProcesses.
	enum class EnumerateProcessesRet
	{
		CreateToolhelp32Snapshot,												///< EnumerateProcesses failed calling CreateToolhelp32Snapshot.
		Process32First,															///< EnumerateProcesses failed calling Process32First.
		Process32Next,															///< Corresponding HRESULT indicates if the last call to Process32Next was successful or not. Note that this HRESULT
																				///  value is a translation from winerror (GetLastError()) @see return section of EnumerateProcesses().
	};

	/// Enlists all processes running in the system.
	/// @param callback callback function that is called for each profile.
	/// @param userData pointer that will be passed to the callback function every time it's called. The meaning of this param is user defined (f.e. it might be a handle to an output window).
	/// @return Process32Next/S_OK if callback function returned false at some point or if reached the end of the process list. For other cases @see EnumerateProcessesRet.
	CppTools::XErrorEnumWithHresult<WinTools::EnumerateProcessesRet> EnumerateProcesses(EnumerateProcessesCallback_t callback, void* userData);

	/// Checks if specified path (to a folder or a file) exists.
	/// @param path location to check.
	/// @return S_OK if path exists, otherwise error code (including WinTools::SystemErrorToHresult(ERROR_FILE_NOT_FOUND), if file doesn't exist or
	///  WinTools::SystemErrorToHresult(ERROR_PATH_NOT_FOUND), if path doesn't exist).
	CppTools::XError<HRESULT> CheckIfPathExists(std::wstring const& path);

	/// Deletes a file. Name DeleteSpecifiedFile instead of just DeleteFile was used because of macros in <windows.h> that try to change the name with DeleteFileA/DeleteFileW.
	/// @param path file location.
	/// @return S_OK if file was deleted successfully, HRESULT error otherwise.
	CppTools::XError<HRESULT> DeleteSpecifiedFile(std::wstring const& path);

	/// Appends path string with another one. Doesn't check for path existence or validity.
	/// @param first path to append to.
	/// @param second string to be appended.
	/// @return string that is combined path.
	std::wstring PathAppend(const std::wstring& first, const std::wstring& second);

	/// Creates Windows shortcut (*.lnk file). WARNING: internally calls CoInitialize.
	/// @param pathToTarget - element that is pointed by newly created link.
	/// @param pathToLnk - where to save newly created lnk?
	/// @param targetCommandLine - arguments for target.
	/// @return HRESULT mix of internal calls (S_OK if succeeded).
	CppTools::XError<HRESULT> CreateLnk(const std::wstring& pathToTarget, const std::wstring& pathToLnk, const std::wstring& targetCommandLine = L"");

	/// Queries Microsoft Windows Task Scheduler for specified task existence.
	/// @param taskName name of the task to check.
	/// @return S_OK if no error occurred and specified task already exists. Otherwise an error code (including WinTools::SystemErrorToHresult(ERROR_FILE_NOT_FOUND) if there's no task with name specified).
	CppTools::XError<HRESULT> CheckIfScheduledTaskExists(const std::wstring& taskName);

	/// Deletes a task from Microsoft Windows Task Scheduler.
	/// @param taskName name of the task to delete.
	/// @return WinTools::SystemErrorToHresult(ERROR_FILE_NOT_FOUND) if specified task was not found.
	CppTools::XError<HRESULT> DeleteScheduledTask(const std::wstring& taskName);

	/// Creates a boot-time task in Microsoft Task Scheduler.
	/// @param taskName name of the task to create.
	/// @param pathToTarget program to run on system boot.
	/// @param targetCommandLine command-line parameters for target.
	/// @param overwriteIfExists if task with specified name already exists then this flag determines if it'll be overwritten or left without any changes.
	/// @return If you're not a administrator then you'll get a WinTools::SystemErrorToHresult(E_ACCESSDENIED). If overwriteIfExists is set and specified task already exists then
	///  WinTools::SystemErrorToHresult(ERROR_ALREADY_EXISTS) is returned.
	CppTools::XError<HRESULT> ScheduleBootTimeTask(const std::wstring& taskName, const std::wstring& pathToTarget, std::wstring const& targetCommandLine, bool overwriteIfExists);

	/// Gets current process name.
	/// @param name output string containing current process name.
	/// @return HRESULT redirected from a call to QueryFullProcessImageName.
	CppTools::XError<HRESULT> GetCurrentProcessName(OUT std::wstring& name);

	/// Constexpr version of HRESULT_FROM_WIN32. In contrast to HRESULT_FROM_WIN32, it can be used in switches.
	/// @param errorCode system error to convert.
	/// @return Converted HRESULT value.
	constexpr HRESULT SystemErrorToHresult(ULONG errorCode) { return (HRESULT)(errorCode) <= 0 ? (HRESULT)(errorCode) : (HRESULT)(((errorCode) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000); }
}

// Other module structures.
#include "WinApiHrException.h"
