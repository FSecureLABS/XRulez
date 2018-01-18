#pragma once

namespace MapiTools
{
	/// Useful wrapper for SRowSet structure.
	struct RowSet : public CppTools::PointerObject<RowSet, SRowSet>
	{
		/// Default move constructor.
		RowSet(RowSet&&) = default;

		/// Destructor.
		~RowSet() { if (!m_Pointer) return; FreeProws(m_Pointer); m_Pointer = nullptr; }

	protected:
		/// Protected ctor.
		/// @param rows API pointer to rows.
		RowSet(LPSRowSet rows) : PointerObject(rows) { ValidateParam(rows); }

		/// Friendships.
		friend struct MapiTable;
	};
}
