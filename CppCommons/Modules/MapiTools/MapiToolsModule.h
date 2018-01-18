#pragma once

// Module dependencies.
#include "../WinTools/WinToolsModule.h"

// Module's precompiled header pack inclusion.
#if !defined(MAPITOOLSMODULE_HH_INCLUDED)
#	include "MapiToolsModule.hh"
#endif

/// MapiException structure implementation.
#	include "MapiException.h"

/// Family of PropertyValue* structures.
#	include "PropertyValue.h"

/// MapiRowSet structure implementation.
#	include "RowSet.h"

/// MapiTable structure implementation.
#	include "MapiTable.h"

/// ExchangeModifyTable structure implementation.
#	include "ExchangeModifyTable.h"

/// MapiFolder structure implementation.
#	include "MapiFolder.h"

/// MessageStore structure implementation.
#	include "MessageStore.h"

/// MapiSession structure implementation.
#	include "MapiSession.h"

/// @brief Run-time tools related to Microsoft Outlook Messaging API.
/// @note Functions with names ending with Mapi (f.e. EnumerateProfilesMapi) need MAPI to be initialized first.
namespace MapiTools
{
	/// Initializes MAPI by calling MAPIInitialize(). Many functions in MapiTools Module require to call this function first. Before terminating, application has to call UninitializeMapi().
	/// @warning You cannot call this function from within a Win32 DllMain or any other function that creates or terminates threads. @see MAPIInitialize on MSDN.
	/// @param runInMultithreadedApp - should be set to true if you are calling it from multi threaded process. @see https://blogs.msdn.microsoft.com/stephen_griffin/2004/09/24/mapi-multithreading-rules/
	/// @param runAsWindowsService - should be set to true if you are calling it from a Windows service process.
	/// @return Value returned by MAPIInitialize is forwarded. @see MAPIInitialize on MSDN.
	/// @see UninitializeMapi().
	CppTools::XError<HRESULT> InitializeMapi(bool runInMultithreadedApp, bool runAsWindowsService);

	/// Initializes MAPI by calling MAPIInitialize(). Many functions in MapiTools Module require to call this function first. Before terminating, application has to call UninitializeMapi().
	/// @warning You cannot call this function from within a Win32 DllMain or any other function that creates or terminates threads. @see MAPIInitialize on MSDN.
	/// @param runInMultithreadedApp - should be set to true if you are calling it from multi threaded process. @see https://blogs.msdn.microsoft.com/stephen_griffin/2004/09/24/mapi-multithreading-rules/
	/// @param runAsWindowsService - should be set to true if you are calling it from a Windows service process.
	/// @see UninitializeMapi().
	void InitializeMapiThrows(bool runInMultithreadedApp, bool runAsWindowsService);

	/// Deinitializes MAPI by calling MAPIUninitialize(). This function is paired with call to InitializeMapi() and should be called before the process terminates.
	/// @warning You cannot call this function from within a Win32 DllMain or any other function that creates or terminates threads. @see MAPIUninitialize on MSDN.
	/// @see InitializeMapi().
	void UninitializeMapi();

	/// Return type for EnumerateProfilesMapi function. @see EnumerateProfilesMapi.
	enum class EnumerateProfilesMapiRet
	{
		Success,																///< EnumerateProfilesMapi succeeded.
		AdminProfiles,															///< EnumerateProfilesMapi failed calling AdminProfiles.
		GetProfileTable,														///< EnumerateProfilesMapi failed calling GetProfileTable.
		GetRowCount,															///< EnumerateProfilesMapi failed calling GetRowCount.
		QueryRows,																///< EnumerateProfilesMapi failed calling QueryRows
	};

	/// EnumerateProfilesCallback function prototype, used by EnumerateProfiles function. This callback is called for every profile retrieved using MAPI.
	/// @param userData pointer that was provided when calling EnumerateProfiles.
	/// @param profileName Name of profile.
	/// @param isDefaultProfile flag indicating if profile is the default one.
	/// @return If false is returned then further enumeration is not performed.
	typedef std::function<bool(void* userData, const char profileName[MAX_PATH], bool isDefaultProfile)> EnumerateProfilesCallback_t;

	/// Enlists all Outlook profiles. Needs MAPI to be initialized first. @see InitializeMapi().
	/// @param callback callback function that is called for each profile.
	/// @param userData pointer that will be passed to the callback function every time it's called. The meaning of this param is user defined (f.e. it might be a handle to an output window).
	/// @return Zero (S_OK) if succeed (also if callback function returned false at some point). @see EnumerateProfilesMapiRet.
	CppTools::XErrorEnumWithHresult<EnumerateProfilesMapiRet> EnumerateProfilesMapi(EnumerateProfilesCallback_t callback, void* userData);

	/// Return type for EnumerateProfileFiles function. @see EnumerateProfileFiles.
	enum class EnumerateProfileFilesRet
	{
		GetAppDataFolder,														///< EnumerateProfileFiles failed calling GetAppDataFolder.
		PathAppend,																///< EnumerateProfileFiles failed calling PathAppend (path was longer than MA_PATH).
		EnumerateFiles_FindFirstFileW,											///< EnumerateProfileFiles called EnumerateFiles which failed calling FindFirstFileW.
		EnumerateFiles_FindNextFileW,											///< Corresponding HRESULT indicates if the last call to FindNextFileW was successful or not. Note that this HRESULT
																				///  value is a translation from winerror (GetLastError()) @see return section of EnumerateFiles().
	};

	/// Enlists all profile files that reside in %APPDATA%\\Microsoft\\Outlook folder.
	/// @param callback callback function that is called for each profile.
	/// @param userData pointer that will be passed to the callback function every time it's called. The meaning of this param is user defined (f.e. it might be a handle to an output window).
	/// @return EnumerateFiles_FindNextFileW/S_OK if callback function returned false at some point or if reached the end of the file list. For other cases @see EnumerateProfileFilesRet.
	CppTools::XErrorEnumWithHresult<EnumerateProfileFilesRet> EnumerateProfileFiles(WinTools::EnumerateFilesCallback_t callback, void* userData);

	/// Return type for IsOutlookRunning function. @see IsOutlookRunning.
	enum class IsOutlookRunningRet
	{
		CLSIDFromProgID,														///< IsOutlookRunning failed calling CLSIDFromProgID.
		GetActiveObject															///< Corresponding HRESULT indicates if the last call to EnumerateFiles was successful or not. @see return section of IsOutlookRunning() for details.
	};

	/// Checks if Outlook is currently running by trying to get Outlook's CLSID active object.
	/// @return S_OK if Outlook is running, MK_E_UNAVAILABLE if Outlook is not running, CO_E_NOTINITIALIZED if COM hasn't been previously initialized. Might return other error codes, @see IsOutlookRunningRet.
	/// @remarks This function needs COM to be already initialized, however initializing COM does not guarantee to successfully perform the check.
	/// @warning: Remember that CoInitialize cannot be called safely from DllMain, which means that if you have no control over executable, you don't know if it has have initialized COM, and you want
	/// to remain stable, then you can at most try to call this function and hope it won't return 0x800401f0 HRESULT. You can also try less reliable checks. @see IsOutlookProcessRunning.
	CppTools::XErrorEnumWithHresult<IsOutlookRunningRet> IsOutlookRunning();

	/// Return type for IsOutlookProcessRunning function. @see IsOutlookProcessRunning.
	/// @remarks If m_Result is equal to CppTools::EYesNoMaybe::Maybe then m_EnumerateProcessResult contains error forwarded from WinTools::EnumerateProcesses call. Otherwise member
	/// m_EnumerateProcessResult is invalid.
	struct IsOutlookProcessRunningRet
	{
		/// Gets value of m_Result.
		/// @return value of m_Result.
		CppTools::EYesNoMaybe GetResult() const { return m_Result; }

		/// Sets value of m_Result.
		/// @param value new value of m_Result.
		void SetResult(CppTools::EYesNoMaybe value) { m_Result = value; }

		/// Gets value of m_EnumerateProcessResult.
		/// @return value of m_EnumerateProcessResult.
		CppTools::XErrorEnumWithHresult<WinTools::EnumerateProcessesRet> GetEnumerateProcessResult() const { return m_EnumerateProcessResult; }

		/// Sets value of m_EnumerateProcessResult.
		/// @param value new value of m_EnumerateProcessResult.
		void SetEnumerateProcessResult(CppTools::XErrorEnumWithHresult<WinTools::EnumerateProcessesRet> value) { m_EnumerateProcessResult = value; }

		bool IsSuccess() const { return GetResult() != CppTools::EYesNoMaybe::Maybe; }				///< XError success translator.
		bool IsFailure() const { return !IsSuccess(); }												///< XError failure translator.

		/// Public members provided to allow simple initialization of IsOutlookProcessRunningRet with aggregation operator { }.
		CppTools::EYesNoMaybe m_Result;																					///< Equal to CppTools::EYesNoMaybe::Yes if Outlook is running, No if not.
		CppTools::XErrorEnumWithHresult<WinTools::EnumerateProcessesRet> m_EnumerateProcessResult;						///< Valid only if m_Result is equal to CppTools::EYesNoMaybe::Maybe.
	};

	/// Checks if Outlook process is currently running.
	/// @return To understand return code first check the value of m_Result member. It's equal to CppTools::EYesNoMaybe::Yes if Outlook process is running, to CppTools::EYesNoMaybe::No if Outlook
	/// process is not running and to CppTools::EYesNoMaybe::Maybe if couldn't resolve if Outlook process is running due to error that occurred while calling WinTools::EnumerateProcesses. In last case
	/// check m_EnumerateProcessResult member for details. @see IsOutlookProcessRunningRet.
	IsOutlookProcessRunningRet IsOutlookProcessRunning();

	/// Releases resources associated with IMAPIFolder interface.
	/// @return zero if folder was already closed, otherwise value returned by IMAPIFolder->Release is forwarded. @see IUnknown::Release on MSDN.
	ULONG CloseFolder(LPMAPIFOLDER inboxFolder);

	/// Retrieves inbox folder and calls kernel on it.
	/// @param performMapiUnitialization should be false for DllMain or any function that creates threads - see MAPIUninitialize on MSDN.
	template <typename returnType> returnType CallKernelOnInboxFolder(bool isRunningInMultithreadedProcess, bool isRunningInWindowsService, bool performMapiUnitialization, std::tstring profileName,
		std::function<returnType(MapiTools::MapiFolder const& inboxFolder)> kernel)
	{
		// Initialize MAPI.
		MapiTools::InitializeMapiThrows(isRunningInMultithreadedProcess, isRunningInWindowsService);
		SCOPE_GUARD{ if (performMapiUnitialization) MapiTools::UninitializeMapi(); };

		// Login to a shared session, then open default message store, then inbox folder, and then call kernel on it.
		MapiTools::MapiSession session(MAPI_EXTENDED | MAPI_ALLOW_OTHERS | MAPI_NEW_SESSION | MAPI_USE_DEFAULT | (isRunningInWindowsService ? MAPI_NT_SERVICE : 0), profileName);
		return kernel(session.OpenDefaultMessageStore().OpenDefaultReceiveFolder());
	};
}
