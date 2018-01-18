#pragma once

namespace MapiTools
{
	/// Useful wrapper for IMsgStore structure.
	struct MessageStore : public CppTools::PointerObject<MessageStore, IMsgStore>
	{
		/// Default move constructor.
		MessageStore(MessageStore&&) = default;

		/// Destructor.
		~MessageStore() { Close(); }

		/// Closes the message store.
		void Close() { if (!m_Pointer) return; m_Pointer->Release(); m_Pointer = nullptr; }

		/// Opens the default receive folder (which is usually "inbox").
		MapiTools::MapiFolder OpenDefaultReceiveFolder();

	protected:
		/// Protected ctor.
		/// @param messageStore API pointer to message store.
		MessageStore(LPMDB messageStore) : PointerObject(messageStore) { ValidateParam(messageStore); }

		/// Friendships.
		friend struct MapiSession;
	};
}
