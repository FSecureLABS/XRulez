#include "StdAfx.h"

#include "MapiTable.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::RowSet MapiTools::MapiTable::QueryRows(LONG rowCount, ULONG flags)
{
	// Sanity check.
	ValidateParam(m_Pointer);

	// If rowCount is not set, get number of available rows.
	if (!rowCount)
		CallMapi(m_Pointer->GetRowCount(flags, reinterpret_cast<ULONG*>(&rowCount)));				//< Get number of available rows.
	if (!rowCount)																//< List might be empty, but we are not allowed to query for zero rows.
		rowCount = 1;

	// Retrieve rows and return the RowSet object.
	LPSRowSet rows;
	CallMapi(m_Pointer->QueryRows(rowCount, flags, &rows));
	return rows;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MapiTools::RowSet MapiTools::MapiTable::QueryAllRows(LPSPropTagArray propertyTagArray, LPSRestriction restriction, LPSSortOrderSet sortOrder)
{
	// Sanity check.
	ValidateParam(m_Pointer);

	// Retrieve rows and return the RowSet object.
	LPSRowSet rows;
	CallWinApiHr(HrQueryAllRows(m_Pointer, propertyTagArray, restriction, sortOrder, 0, &rows));
	return rows;
}
