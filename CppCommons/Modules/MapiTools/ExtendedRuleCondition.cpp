#include "StdAfx.h"

#include "ExtendedRuleCondition.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::ExtendedRuleCondition::ExtendedRuleCondition(const std::wstring& conditionString)
{
	// Validate input.
	CHECK(!conditionString.empty() AND conditionString.size() < 256);

	// Create and fill byte stream with a packed version of SRestriction with SPropValue inside, where instead of pointers all data is in-place.
	m_ByteStream += {															//< SRestriction sres;
		0x00, 0x00, 0x03,														//< sres.rt = RES_CONTENT; (trimmed of a one MSB)
		0x01, 0x00, 0x01,														//< sres.res.resContent.ulFuzzyLevel = Trimmed FL_IGNORECASE | FL_SUBSTRING; (trimmed of a one MSB)
		0x00, 0x1F, 0x00, 0x37,													//< sres.res.resContent.ulPropTag = PR_SUBJECT_W; (not trimmed)
		// Packed SPropValue structure starts here:								    SPropValue SvcProps;
		0x00, 0x1F, 0x00, 0x37,													//< SvcProps.ulPropTag = PR_SUBJECT_W; (not trimmed)
		0x00,																	//< SvcProps.dwAlignPad = 0; (trimmed of 3 MSB)
	};

	// Copy the condition string and append it with a wide null string termination character.
	((m_ByteStream += conditionString) += '\0') += '\0';

	// Fill m_PropertyValue.
	m_PropertyValue.ulPropTag = PR_EXTENDED_RULE_CONDITION;
	m_PropertyValue.Value.bin = SBinary{ static_cast<ULONG>(m_ByteStream.Size()), const_cast<LPBYTE>(m_ByteStream.GetBytes()) };

	// Checks.
	CHECK(m_ByteStream.Size() == 15 + (conditionString.size() + 1) * sizeof(wchar_t));
}
