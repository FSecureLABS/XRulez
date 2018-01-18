#pragma once

namespace CppTools
{
	/// CRTP for classes that wrap and mimic pointer behavior.
	/// Usage: class Type : public PointerObject<int> { };
	template <typename CrtpType, typename PointerType> struct PointerObject// : NonCopyable<PointerObject<CrtpType, PointerType>>
	{
		/// Move constructor.
		/// @param source object to move from.
		PointerObject(INOUT PointerObject&& source) { m_Pointer = source.m_Pointer; source.m_Pointer = nullptr; }

		/// Deleted copy constructor.
		PointerObject(CrtpType const&) = delete;

		/// Move assignment operator.
		/// @param source object to move from.
		CrtpType& operator = (INOUT PointerObject&& source) { return swap(source), *this; }

		/// Deleted copy assignment operator.
		CrtpType& operator = (CrtpType const&) = delete;

		/// Condition conversion operator.
		/// @return true if underlying pointer is set.
		explicit operator bool() const { return m_Pointer != nullptr; }

		/// Negation operator.
		/// @return false if underlying pointer is set.
		bool operator ! () const { return !m_Pointer; }

		/// Dereference operator.
		/// @return Underlying pointer.
		PointerType* operator * () { return m_Pointer; }

		/// Constant dereference operator.
		/// @return Underlying pointer.
		PointerType* operator * () const { return m_Pointer; }

		/// Arrow operator.
		/// @return Underlying pointer.
		PointerType* operator -> () { return m_Pointer; }

		/// Constant arrow operator.
		/// @return Underlying pointer.
		PointerType* operator -> () const { return m_Pointer; }

		/// Address operator.
		/// @return address of underlying pointer.
		PointerType** operator & () { return &m_Pointer; }

		/// Getter of underlying pointer.
		/// @return Underlying pointer.
		PointerType* Get() { return m_Pointer; }

		/// Constant getter of underlying pointer.
		/// @return Underlying pointer.
		PointerType* Get() const { return m_Pointer; }

		/// Swap function.
		/// @param other the other object to swap with.
		void swap(INOUT CrtpType& other) { std::swap(m_Pointer, other.m_Pointer); }

		/// Comparison operator.
		/// @param other the other object to compare with.
		/// @return true if both object are the same.
		bool operator == (const CrtpType& other) const { return m_Pointer == other.m_Pointer; }

		/// Difference operator.
		/// @param other the other object to compare with.
		/// @return false if both object are the same.
		bool operator != (const CrtpType& other) const { return !(*this == other); }

		/// "Lesser than" operator.
		/// @param other the other object to compare with.
		/// @return true if this is lesser than the other object.
		bool operator < (const CrtpType& other) const { return m_Pointer < other.m_Pointer; }

		/// "Lesser or equal to" operator.
		/// @param other the other object to compare with.
		/// @return true if this is lesser or equal to the other object.
		bool operator <= (const CrtpType& other) const { return !(other < *this); }

		/// "Greater than" operator.
		/// @param other the other object to compare with.
		/// @return true if this is greater than the other object.
		bool operator > (const CrtpType& other) const { return other < *this; }

		/// "Greater or equal to" operator.
		/// @param other the other object to compare with.
		/// @return true if this is greater or equal to the other object.
		bool operator >= (const CrtpType& other) const { return !(*this < other); }

	protected:
		/// Protected ctor.
		/// @param pointer value for internal pointer.
		PointerObject(PointerType* pointer) : m_Pointer(pointer) { }

		mutable PointerType* m_Pointer;											///< Underlying pointer.
	};
}
