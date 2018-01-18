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
