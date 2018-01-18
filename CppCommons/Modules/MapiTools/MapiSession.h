#pragma once

namespace MapiTools
{
	/// Useful wrapper for IMAPISession structure.
	struct MapiSession : public CppTools::PointerObject<MapiSession, IMAPISession>
	{
		/// Public ctor. Logs in to MAPI session. MAPI needs to be initialized before calling this function. @see MapiTools::InitializeMapi.
		/// @param ulFlags logon flags, @see MSDN documentation of MAPILogonEx function.
		/// @param profileName profile name. This string has to be shorter than 66 characters. If this parameter is empty then default profile is used.
		MapiSession(ULONG ulFlags, std::tstring profileName);

		/// Destructor.
		~MapiSession() { Logoff(); }

		/// Logs off from opened MAPI session.
		void Logoff();

		/// Opens the default message store.
		/// @return MessageStore object.
		MapiTools::MessageStore OpenDefaultMessageStore();
	};
}
