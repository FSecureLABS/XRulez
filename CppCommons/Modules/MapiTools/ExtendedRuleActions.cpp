#include "StdAfx.h"

#include "ExtendedRuleActions.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::ExtendedRuleActions::ExtendedRuleActions(std::wstring ruleName, std::wstring ruleTriggerText, std::wstring rulePayloadPath)
{
	// Validate input.
	CHECK(!ruleName.empty() AND ruleName.size() <= 256 AND !rulePayloadPath.empty() AND ruleTriggerText.size() <= 256 AND !rulePayloadPath.empty() AND rulePayloadPath.size() <= 256);

	// Calculate packet size.
	size_t bufferLength = 160 + (ruleName.size() + ruleTriggerText.size() + rulePayloadPath.size()) * sizeof(wchar_t);

	// Create and fill byte stream with a packed version of ACTIONS structure with ACTION structure inside.
	m_ByteStream += {															//< ACTIONS actions;
		0x00, 0x00, 0x01,														//< actions.ulVersion = EDK_RULES_VERSION; (trimmed of one MSB)
		0x00, 0x00, 0x00, 0x01,													//< actions.cActions = 1; (not trimmed)
		0x00, 0x00, 0x00,														//< actions.lpAction = nullptr; (trimmed of one MSB) ?
	};

	// Stream in length of the rest of the packet.
	m_ByteStream += static_cast<int32_t>(bufferLength) - 14;

	// Packed ACTION structure starts here.
	m_ByteStream += {															//< ACTION action;
		0x05, 0x00, 0x00, 0x00, 												//< actions.acttype = OP_DEFER_ACTION; (little endian?)
		0x00, 0x00, 0x00, 0x00, 												//< actions.lpRes = NULL;
		0x00,																	//< actions.ulFlags = 0;
	};

	// Another length of the rest of the packet.
	m_ByteStream += static_cast<int32_t>(bufferLength) - 27;

	// Fill with the rest of the header.
	m_ByteStream += { 0x40, 0x42, 0x0F, 0x00 };									//< ??? something PR_RECEIVED_BY_NAME_W + PR_SENT_REPRESENTING_NAME_W alike?

	// Fill in rule name.
	m_ByteStream += static_cast<BYTE>(ruleName.size());
	m_ByteStream += ruleName;

	// RuleElement byte stream, always the same. Contains string "RuleElement".
	m_ByteStream += { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x94, 0x01, 0x00, 0x00, 0x06, 0x00, 0xFF, 0xFF, 0x00,
		0x00, 0x0C, 0x00, 0x43, 0x52, 0x75, 0x6C, 0x65, 0x45, 0x6C, 0x65, 0x6D, 0x65, 0x6E, 0x74, 0x90, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
		0x80, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x80, 0xCD, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// Fill in trigger name.
	m_ByteStream += static_cast<BYTE>(ruleTriggerText.size());
	m_ByteStream += ruleTriggerText;

	m_ByteStream += { 0x01, 0x80, 0x49, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// Fill in payload path.
	m_ByteStream += static_cast<BYTE>(rulePayloadPath.size());
	m_ByteStream += rulePayloadPath;

	// Fill in the rest of the packet and prepare output members.
	m_ByteStream += { 0x01, 0x80, 0x4A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x42, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	m_PropertyValue.ulPropTag = PR_EXTENDED_RULE_ACTIONS;
	m_PropertyValue.Value.bin = SBinary{ static_cast<ULONG>(bufferLength), const_cast<LPBYTE>(m_ByteStream.GetBytes()) };

	// Checks.
	CHECK(m_ByteStream.Size() == bufferLength);
}
