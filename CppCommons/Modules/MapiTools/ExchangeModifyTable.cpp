#include "StdAfx.h"

// Modules.
#include "MapiToolsModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::MapiTable MapiTools::ExchangeModifyTable::GetMapiTable()
{
	// Sanity check.
	ValidateParam(m_Pointer);

	// Retrieve table and return the table object.
	LPMAPITABLE table;
	CallMapi(m_Pointer->GetTable(0, &table));

	return table;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<bool> MapiTools::ExchangeModifyTable::DeleteRule(LARGE_INTEGER const& ruleId)
{
	// Allocate a row list.
	LPROWLIST list;
	CallMapi(MAPIAllocateBuffer(offsetof(ROWLIST, aEntries) + 1 * sizeof(ROWENTRY), reinterpret_cast<LPVOID*>(&list)));
	SCOPE_GUARD{ MAPIFreeBuffer(list); };

	// Fill the list.
	SPropValue propertyValue;
	propertyValue.ulPropTag = PR_RULE_ID;
	propertyValue.Value.li = ruleId;
	list->cEntries = 1;
	list->aEntries[0].cValues = 1;
	list->aEntries[0].ulRowFlags = ROW_REMOVE;
	list->aEntries[0].rgPropVals = &propertyValue;

	// Modify the table.
	HRESULT hr = m_Pointer->ModifyTable(0, list);
	if (hr == 0x8004010F)														//< Undocumented magic value - rule of provided ID doesn't exist.
		return false;
	if (FAILED(hr))
		throw MapiTools::MapiException(hr, XEXCEPTION_PARAMS, "m_Pointer->ModifyTable(0, list)");

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<MapiTools::ExchangeModifyTable::RuleInfo> MapiTools::ExchangeModifyTable::EnumerateRules()
{
	// Prepare for enumeration.
	auto table = GetMapiTable();												//< Get the table.
	CallMapi(table->Restrict(nullptr, TBL_ASYNC));								//< Reset restrictions.
	CallMapi(table->SeekRow(BOOKMARK_BEGINNING, 0, NULL));						//< Reset cursor.

	// Query table for rows.
	LPSRowSet rows;
	CallMapi(table->QueryRows(1000, 0, &rows));									//< Maximum 1000 rows.
	
	// Parse returned data.
	std::vector<RuleInfo> retVector;
	for (unsigned i = 0; i < rows->cRows; ++i)
	{
		// Pre-fill output entry with invalid/blank data.
		std::wstring ruleName = L"[Unknown]", ruleProvider = L"[Unknown]";
		LARGE_INTEGER ruleId{ 0xFFffFFff, (LONG)-1 };
		ULONG ruleSequenceNo = (ULONG )-1;
		
		// Search through columns, find those we are here for and parse their values.
		for (ULONG j = 0; j < rows->aRow[i].cValues; j++)
			switch (rows->aRow[i].lpProps[j].ulPropTag)
			{
			case (PR_RULE_NAME & 0xFFff0000) | PT_STRING8:
				ruleName = CppTools::StringConversions::Convert<std::wstring>(rows->aRow[i].lpProps[j].Value.lpszA); break;

			case (PR_RULE_NAME & 0xFFff0000) | PT_UNICODE:
				ruleName = rows->aRow[i].lpProps[j].Value.lpszW; break;

			case (PR_RULE_PROVIDER & 0xFFff0000) | PT_STRING8:
				ruleProvider = CppTools::StringConversions::Convert<std::wstring>(rows->aRow[i].lpProps[j].Value.lpszA); break;

			case (PR_RULE_PROVIDER & 0xFFff0000) | PT_UNICODE:
				ruleProvider = rows->aRow[i].lpProps[j].Value.lpszW; break;

			case PR_RULE_ID:
				ruleId = rows->aRow[i].lpProps[j].Value.li; break;

			case PR_RULE_SEQUENCE:
				ruleSequenceNo = rows->aRow[i].lpProps[j].Value.ul; break;
			}

		// Store parsed rule entry.
		retVector.push_back({ ruleName, ruleProvider, ruleId, ruleSequenceNo });
	}
	
	// Clean-up and return the data.
	if (rows)
		FreeProws(rows);

	return retVector;
}
