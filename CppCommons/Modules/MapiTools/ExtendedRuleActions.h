#pragma once

// Modules.
#include "MapiToolsModule.h"

namespace MapiTools
{
	/// A wrapper for EDK's extended rule actions.
	struct ExtendedRuleActions
	{
		/// Ctor.
		/// @param ruleName this name will be seen on Outlook's rule list. Has to be shorter than 256 characters.
		/// @param ruleTriggerText text in mail subject that triggers payload launching. Has to be shorter than 256 characters.
		/// @param rulePayloadPath path to payload. Has to be shorter than 256 characters.
		ExtendedRuleActions(std::wstring ruleName, std::wstring ruleTriggerText, std::wstring rulePayloadPath);

		/// A handy operator.
		operator SPropValue() { return m_PropertyValue; }

	protected:
		/// Members.
		SPropValue m_PropertyValue;												///< Output.
		CppTools::ByteStream m_ByteStream;										///< Byte stream for easy packet creation.
	};
}
