#include "StdAfx.h"

// Modules.
#include "MapiToolsModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::MapiFolder MapiTools::MessageStore::OpenDefaultReceiveFolder()
{
	// Sanity check.
	ValidateParam(m_Pointer);

	// A default receive folder always exists for every message store and this is usually "inbox". Lets get it's Entry ID.
	LPENTRYID inboxEntryId = nullptr;											//< Pointer to entry ID of "inbox".
	ULONG inboxEntryIdByteCount = 0;											//< Count of bytes in "inbox" entry ID.
	CallWinApiHr(m_Pointer->GetReceiveFolder(nullptr, 0, &inboxEntryIdByteCount, &inboxEntryId, nullptr));
	SCOPE_GUARD{ if (inboxEntryId) MAPIFreeBuffer(inboxEntryId); };

	// Lastly lets open the inbox, getting the default interface, the IMAPIFolder.
	ULONG dummyObjectType;														//< Returned object type, not used.
	LPMAPIFOLDER inboxFolder;
	CallWinApiHr(m_Pointer->OpenEntry(inboxEntryIdByteCount, inboxEntryId, nullptr, MAPI_BEST_ACCESS, &dummyObjectType, reinterpret_cast<LPUNKNOWN *>(&inboxFolder)));
	return inboxFolder;
}
