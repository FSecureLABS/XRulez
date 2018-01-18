#pragma once

/// @brief XRMod-specific code name space.
namespace XRMod
{
	/// Adsa Application structure.
	struct Application
	{
		/// This class can be instantiated only in the entry point of the application.
		friend int ::Main();

		/// Public destructor.
		~Application() { }

	protected:
		/// Protected ctor.
		Application() { }

		/// Main function of the application.
		/// @return true if succeed.
		CppTools::XError<bool> Process();

	protected:
		/// Another executable command line processing helper function. Handles one single command-line command. Should not be called from DLL builds.
		/// @param pathToImage path to XRulez binary to patch.
		/// @param commandKey name-part the a command to process.
		/// @param commandValue value-part the a command to process.
		/// @return false if parameters are not valid and program should exit.
		CppTools::XError<bool> HandleSingleCommand(const std::wstring& pathToImage, const std::wstring& commandKey, const std::wstring& commandValue);


		/// Performs parameter patching in a XRulez binary.
		/// @param pathToImage path to XRulez binary to patch.
		/// @param stringId string key.
		/// @param stringName string value.
		/// @return true if succeeded.
		CppTools::XError<bool> XRMod::Application::PatchString(const std::wstring& pathToImage, unsigned stringId, const std::wstring& stringName);

		/// Shows Exe usage to the user.
		void ShowUsage();
	};
}
