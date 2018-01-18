#pragma once

// Modules.
#include "MapiToolsModule.h"

namespace MapiTools
{
	/// A wrapper for EDK's extended rule condition.
	struct ExtendedRuleCondition
	{
		/// Ctor.
		/// @param conditionString condition string. Has to be shorter than 256 characters.
		ExtendedRuleCondition(const std::wstring& conditionString);

		/// A handy operator.
		operator SPropValue() { return m_PropertyValue; }

	protected:
		/// Members.
		SPropValue m_PropertyValue;												///< Output.
		CppTools::ByteStream m_ByteStream;										///< Byte stream for easy packet creation.
	};
}
