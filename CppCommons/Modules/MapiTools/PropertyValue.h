// Implementation of PropertyValue* structure family - wrappers around SPropValue.
#pragma once

namespace MapiTools
{
	/// Base, abstract structure for all PropertyValue* incarnations.
	struct PropertyValue
	{
		/// Conversion operator.
		operator SPropValue() { return m_PropertyValue; }

	protected:
		/// Protected ctor.
		/// @param propertyTag property tag value.
		PropertyValue(ULONG propertyTag) : m_PropertyValue({ propertyTag }) { }

		/// Members.
		SPropValue m_PropertyValue;												///< Underlying SPropValue object.
	};

	/// PropertyValue specialized for data of type long.
	struct PropertyValueLong : public PropertyValue
	{
		/// Public ctor.
		/// @param propertyTag property tag value.
		/// @param value property value.
		PropertyValueLong(ULONG propertyTag, LONG value) : PropertyValue(propertyTag) { m_PropertyValue.Value.l = value; }
	};

	/// PropertyValue specialized for data of type LARGE_INTEGER.
	struct PropertyValueLargeInteger : public PropertyValue
	{
		/// Public ctor.
		/// @param propertyTag property tag value.
		/// @param value property value.
		PropertyValueLargeInteger(ULONG propertyTag, LARGE_INTEGER value) : PropertyValue(propertyTag) { m_PropertyValue.Value.li = value; }
	};

	/// PropertyValue specialized for bool type.
	struct PropertyValueBool : public PropertyValue
	{
		/// Public ctor.
		/// @param propertyTag property tag value.
		/// @param flag property value.
		PropertyValueBool(ULONG propertyTag, bool flag) : PropertyValue(propertyTag) { m_PropertyValue.Value.b = flag ? TRUE : FALSE; }
	};

	/// PropertyValue specialized for data of type wchar_t*.
	struct PropertyValueWString : public PropertyValue
	{
		/// Public ctor.
		/// @param propertyTag property tag value.
		/// @param string property value.
		PropertyValueWString(ULONG propertyTag, LPWSTR string) : PropertyValue(propertyTag) { m_PropertyValue.Value.lpszW = string; }
	};

	/// PropertyValue specialized for data of type char*.
	struct PropertyValueAString : public PropertyValue
	{
		/// Public ctor.
		/// @param propertyTag property tag value.
		/// @param string property value.
		PropertyValueAString(ULONG propertyTag, LPSTR string) : PropertyValue(propertyTag) { m_PropertyValue.Value.lpszA = string; }
	};

	/// PropertyValue specialized for data of type TCHAR*.
	struct PropertyValueTString : public PropertyValue
	{
		/// Public ctor.
		/// @param propertyTag property tag value.
		/// @param string property value.
		PropertyValueTString(ULONG propertyTag, LPTSTR string) : PropertyValue(propertyTag) {
#ifdef _UNICODE
			m_PropertyValue.Value.lpszW = string;
#else
			m_PropertyValue.Value.lpszA = string;
#endif
		}
	};

	/// PropertyValue specialized for binary type.
	struct PropertyValueSBinary : public PropertyValue
	{
		/// Public ctor for SBinary.
		/// @param propertyTag property tag value.
		/// @param binary property binary value.
		PropertyValueSBinary(ULONG propertyTag, const SBinary& binary) : PropertyValue(propertyTag) { m_PropertyValue.Value.bin = binary; }

		/// Public ctor for byte stream as a size and a pointer.
		/// @param propertyTag property tag value.
		/// @param size length of byte stream.
		/// @param data byte stream.
		PropertyValueSBinary(ULONG propertyTag, ULONG size, LPBYTE data) : PropertyValueSBinary(propertyTag, SBinary{ size, data }) { }

		/// Public ctor for byte array. No need for providing array size explicitly. Just use it like that: BYTE a[3]; auto x = PropertyValueSBinary(tag, a);
		/// @param propertyTag property tag value.
		/// @param data array of bytes.
		template<size_t size> PropertyValueSBinary(ULONG propertyTag, BYTE(&data)[size]) : PropertyValueSBinary(propertyTag, SBinary{ size, data }) { }

		/// Public ctor for initializer list of bytes.
		/// @param propertyTag property tag value.
		/// @param data initializer list of bytes.
		PropertyValueSBinary(ULONG propertyTag, std::initializer_list<BYTE> data) : PropertyValueSBinary(propertyTag, SBinary{ static_cast<ULONG>(data.size()), const_cast<BYTE*>(data.begin()) }) { }
	};

	/// PropertyValue specialized for date/time type.
	struct PropertyValueDateTime : public PropertyValue
	{
		/// Public ctor for FILETIME type.
		/// @param propertyTag property tag value.
		/// @param fileTime property value of type FILETIME.
		PropertyValueDateTime(ULONG propertyTag, const FILETIME& fileTime) : PropertyValue(propertyTag) { m_PropertyValue.Value.ft = fileTime; }

		/// Public ctor for date/time as two DWORD values (high and low).
		/// @param propertyTag property tag value.
		/// @param highDateTime high value of date/time.
		/// @param lowDateTime low value of date/time.
		PropertyValueDateTime(ULONG propertyTag, DWORD highDateTime, DWORD lowDateTime) : PropertyValueDateTime(propertyTag, FILETIME{ highDateTime, lowDateTime }) { }
	};
}
