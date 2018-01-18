#include "StdAfx.h"

// Modules.
#include "MapiToolsModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::MapiSession::MapiSession(ULONG ulFlags, std::tstring profileName)
	: PointerObject(nullptr)
{
	// Input validation.
	ValidateParam(profileName.size() <= 65);

	// Call MAPILogonEx.
	CallWinApiHr(MAPILogonEx(NULL, const_cast<LPTSTR>(profileName.c_str()), nullptr, ulFlags, &m_Pointer));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MapiTools::MapiSession::Logoff()
{
	if (!m_Pointer)
		return;

	// Log off.
	m_Pointer->Logoff(0, 0, 0);
	m_Pointer->Release();
	m_Pointer = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::MessageStore MapiTools::MapiSession::OpenDefaultMessageStore()
{
	// Retrieve the table of all message stores.
	LPMAPITABLE rawTable;
	CallWinApiHr(m_Pointer->GetMsgStoresTable(0, &rawTable));

	// Now we need to get the default message store out of the contentsTable list. In order to do that, first we need to set up restriction for looking up the store.
	SRestriction restriction{ RES_PROPERTY };									//< Restriction type: comparing a property.
	restriction.res.resProperty.relop = RELOP_EQ;								//< Testing equality.
	restriction.res.resProperty.ulPropTag = PR_DEFAULT_STORE;					//< Tag to compare.

	// We also need a restriction property for looking up the default store.
	SPropValue spv{ PR_DEFAULT_STORE };											//< Tag type.
	spv.Value.b = TRUE;															//< Tag value.
	restriction.res.resProperty.lpProp = &spv;									//< Property tag and value to compare against.

	// Create an array of single property tag. This is effectively a description of a column we want to get.
	static SPropTagArray propertyTagArray{ 1, PR_ENTRYID };

	// Convert the table to an array which can be stepped through. Only one message store should have PR_DEFAULT_STORE set to true, so only one row will be returned.
	MapiTools::MapiTable contentsTable(rawTable);
	RowSet rowSet = contentsTable.QueryAllRows(&propertyTagArray, &restriction);

	// We got the default receiving folder. Lastly, we're going to open the associated message table.
	LPMDB messageStore;
	CallWinApiHr(m_Pointer->OpenMsgStore(NULL, rowSet->aRow[0].lpProps[0].Value.bin.cb, (LPENTRYID)rowSet->aRow[0].lpProps[0].Value.bin.lpb, nullptr,
		MDB_NO_DIALOG | MAPI_BEST_ACCESS | MDB_ONLINE, &messageStore));
	return messageStore;
}
