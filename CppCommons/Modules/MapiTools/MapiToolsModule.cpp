#include "StdAfx.h"

#include "MapiToolsModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// We need to define Exchange SDK GUIDs in one and only one compilation unit. Let's define them here.
#include "InitGuid.h"
#include "../../3rdParty/Exchange/EdkGuid.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> MapiTools::InitializeMapi(bool runInMultithreadedApp, bool runAsWindowsService)
{
	// MAPIInitialize flags are provided via MAPIINIT_0 structure, create one.
	MAPIINIT_0 initStructure{ MAPI_INIT_VERSION, (runInMultithreadedApp ? MAPI_MULTITHREAD_NOTIFICATIONS : 0) | (runAsWindowsService ? MAPI_NT_SERVICE : 0) };

	// Pass it to MAPIInitialize() and forward its error code.
	return CppTools::XErrorCreator(MAPIInitialize(&initStructure));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MapiTools::InitializeMapiThrows(bool runInMultithreadedApp, bool runAsWindowsService)
{
	// MAPIInitialize flags are provided via MAPIINIT_0 structure - create one and pass to a MAPIInitialize call.
	MAPIINIT_0 initStructure{ MAPI_INIT_VERSION, (runInMultithreadedApp ? MAPI_MULTITHREAD_NOTIFICATIONS : 0) | (runAsWindowsService ? MAPI_NT_SERVICE : 0) };
	CallWinApiHr(MAPIInitialize(&initStructure));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MapiTools::UninitializeMapi()
{
	// Self explanatory.
	return MAPIUninitialize();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XErrorEnumWithHresult<MapiTools::EnumerateProfilesMapiRet> MapiTools::EnumerateProfilesMapi(EnumerateProfilesCallback_t callback, void* userData)
{
	// Helper variable to hold API calls return code.
	HRESULT hr;

	// Get pointer to admin profiles.
	LPPROFADMIN adminProfiles = NULL;
	if (hr = MAPIAdminProfiles(0, &adminProfiles), FAILED(hr))
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProfilesMapiRet::AdminProfiles, hr);

	SCOPE_GUARD{ if (adminProfiles) adminProfiles->Release(); };

	// Get profile table pointer.
	LPMAPITABLE profileTable = NULL;
	if (hr = adminProfiles->GetProfileTable(0, &profileTable), FAILED(hr))
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProfilesMapiRet::GetProfileTable, hr);

	SCOPE_GUARD{ if (profileTable) profileTable->Release(); };

	// Get Rows from the profile table.
	unsigned long rowCount;
	if (hr = profileTable->GetRowCount(0, &rowCount), FAILED(hr))
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProfilesMapiRet::GetRowCount, hr);

	LPSRowSet rows = NULL;
	if (hr = profileTable->QueryRows(rowCount, 0, &rows), FAILED(hr))
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProfilesMapiRet::QueryRows, hr);

	SCOPE_GUARD{ if (rows) FreeProws(rows); };

	// Loop through the list of profiles, fill profileName and isDefaultProfile and call the callback function for each pair.
	char profileName[MAX_PATH];
	bool isDefaultProfile = false;
	for (unsigned long rowNumber = 0; rowNumber < rowCount; ++rowNumber)
	{
		// First retrieve data from rows.
		auto rowObject = &rows->aRow[rowNumber];
		for (unsigned propertyNumber = 0; propertyNumber < rowObject->cValues; ++propertyNumber)
			switch (rowObject->lpProps[propertyNumber].ulPropTag)
			{
			case PR_DISPLAY_NAME_A:
				strcpy_s(profileName, rowObject->lpProps[propertyNumber].Value.lpszA);
				break;

			case PR_DEFAULT_PROFILE:
				isDefaultProfile = rowObject->lpProps[propertyNumber].Value.b ? true : false;
			}

		// Now call the callback function with retrieved properties. If it returns false then stop the enumeration.
		if (!callback(userData, profileName, isDefaultProfile))
			return CppTools::XErrorEnumWithHresultCreator(EnumerateProfilesMapiRet::Success, S_OK);

		// Just in case, reset retrieved properties for next iteration.
		*profileName = '\0';
		isDefaultProfile = false;
	}

	// Return success.
	return CppTools::XErrorEnumWithHresultCreator(EnumerateProfilesMapiRet::Success, S_OK);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XErrorEnumWithHresult<MapiTools::EnumerateProfileFilesRet> MapiTools::EnumerateProfileFiles(WinTools::EnumerateFilesCallback_t callback, void* userData)
{
	// Get %APPDATA% folder.
	wchar_t outlookFilesPathAndMask[MAX_PATH];
	auto xeGetAppDataFolder = WinTools::GetAppDataFolder(outlookFilesPathAndMask);
	if (xeGetAppDataFolder.IsFailure())
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProfileFilesRet::GetAppDataFolder, xeGetAppDataFolder);

	// Append it with Outlook profiles path.
	auto pathAppended = WinTools::PathAppend(outlookFilesPathAndMask, L"\\Microsoft\\Outlook\\*.xml");
	if (pathAppended.size() > MAX_PATH)
		return CppTools::XErrorEnumWithHresultCreator(EnumerateProfileFilesRet::PathAppend, 0x80010135);
	wcscpy_s(outlookFilesPathAndMask, MAX_PATH, pathAppended.c_str());

	// Define a proxy callback to filter sub folders out...
	auto filterOutDirectoriesProxyCallback = [&](void*, const WIN32_FIND_DATAW& fileData) -> bool
		{ return fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : callback(userData, fileData); };

	// ...and pass it to WinTools::EnumerateFiles.
	auto xeEnumerateFiles = WinTools::EnumerateFiles(outlookFilesPathAndMask, filterOutDirectoriesProxyCallback, userData);

	// Return suitable return code.
	return CppTools::XErrorEnumWithHresultCreator(xeEnumerateFiles->GetApiCall() == WinTools::EnumerateFilesRet::FindFirstFileW ? EnumerateProfileFilesRet::EnumerateFiles_FindFirstFileW :
		EnumerateProfileFilesRet::EnumerateFiles_FindNextFileW, xeEnumerateFiles->GetHresult());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XErrorEnumWithHresult<MapiTools::IsOutlookRunningRet> MapiTools::IsOutlookRunning()
{
	// Retrieve Outlooks CLSID.
	CLSID clsid;
	auto hr = CLSIDFromProgID(L"Outlook.Application", &clsid);
	if (FAILED(hr))
		return CppTools::XErrorEnumWithHresultCreator(IsOutlookRunningRet::CLSIDFromProgID, hr);

	// Try to get active Outlook object.
	IUnknown *pUnknown = NULL;
	return  CppTools::XErrorEnumWithHresultCreator(IsOutlookRunningRet::GetActiveObject, GetActiveObject(clsid, 0, &pUnknown));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::IsOutlookProcessRunningRet MapiTools::IsOutlookProcessRunning()
{
	// Define variable used for return value and initialize it with { CppTools::EYesNoMaybe::No, <whatever> }.
	IsOutlookProcessRunningRet returnValue = { CppTools::EYesNoMaybe::No, CppTools::XErrorEnumWithHresultCreator(WinTools::EnumerateProcessesRet::CreateToolhelp32Snapshot, S_OK) };

	// Define a callback for WinTools::EnumerateProcessesBasic.
	auto enumerateProcessesBasicCallback = [](void* reValue, const PROCESSENTRY32& processData) -> bool
	{
		// Transform process name to lower case.
		std::tstring processName = processData.szExeFile;
		std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);

		// Check if process name contains "outlook" phrase.
		if (processName.find(TEXT("outlook")) != std::string::npos)
		{
			// It does. Set retValue to CppTools::EYesNoMaybe::Yes and abandon further process enumeration.
			static_cast<IsOutlookProcessRunningRet*>(reValue)->SetResult(CppTools::EYesNoMaybe::Yes);
			return false;
		}

		// Otherwise keep searching.
		return true;
	};

	// ...and pass it to WinTools::EnumerateProcessesBasic. If it returns error then return CppTools::EYesNoMaybe::Maybe.
	auto xe = WinTools::EnumerateProcesses(enumerateProcessesBasicCallback, &returnValue);

	// Return CppTools::EYesNoMaybe::Maybe if something went wrong in WinTools::EnumerateProcesses().
	return xe.IsFailure() ? IsOutlookProcessRunningRet { CppTools::EYesNoMaybe::Maybe, xe } : returnValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ULONG MapiTools::CloseFolder(LPMAPIFOLDER inboxFolder)
{
	// Self explanatory.
	return inboxFolder ? inboxFolder->Release() : 0UL;
}
