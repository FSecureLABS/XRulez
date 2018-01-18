#pragma once

namespace MapiTools
{
	/// Useful wrapper for IMAPITable structure.
	struct MapiTable : public CppTools::PointerObject<MapiTable, IMAPITable>
	{
		/// Default move constructor.
		MapiTable(MapiTable&&) = default;

		/// Destructor.
		~MapiTable() { if (!m_Pointer) return; m_Pointer->Release(); m_Pointer = nullptr; }

		/// Queries Exchange server for rows in the table. @see IMAPITable::QueryRows documentation on MSDN.
		/// @param rowCount number of rows to retrieve.
		/// @param flags bitmask of flags that control how rows are returned.
		/// @return RowSet object.
		RowSet QueryRows(LONG rowCount = 0, ULONG flags = 0);

		/// Queries Exchange server for all available rows for the table. @see HrQueryAllRows documentation on MSDN.
		/// @param propertyTagArray pointer to an SPropTagArray structure that contains an array of property tags indicating table columns.
		/// @param restriction pointer to an SRestriction structure that contains retrieval restrictions.
		/// @param sortOrder pointer to an SSortOrderSet structure identifying the sort order of the columns to be retrieved.
		/// @return RowSet object.
		RowSet QueryAllRows(LPSPropTagArray propertyTagArray = nullptr, LPSRestriction restriction = nullptr, LPSSortOrderSet sortOrder = nullptr);

	protected:
		/// Protected ctor.
		/// @param table API pointer to table.
		MapiTable(LPMAPITABLE table) : PointerObject(table) { ValidateParam(table); }

		/// Friendships.
		friend struct MapiSession;
		friend struct ExchangeModifyTable;
	};
}
