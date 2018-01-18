#pragma once

namespace CppTools
{
	/// ByteStream structure. Similar to standard streams (such as std::stringstream), but operates on raw bytes - ignoring special text characters.
	/// @note ByteStream uses operator += instead of << because operator << cannot be used with initializer_list (no joke).
	struct ByteStream
	{
		/// Concatenation operator. Adds operands to the back of the byte stream.
		/// @param byteStream - stream to add.
		/// @return Reference to the stream.
		ByteStream& operator += (std::initializer_list<BYTE> byteStream) { m_ByteStream.insert(std::end(m_ByteStream), byteStream); return *this; }

		/// Concatenation operator. Adds operand to the back of the byte stream.
		/// @param byte - byte to add.
		/// @return Reference to the stream.
		ByteStream& operator += (BYTE byte) { m_ByteStream.push_back(byte); return *this; }

		/// Concatenation operator. Adds operand to the back of the byte stream.
		/// @param byte - byte to add.
		/// @return Reference to the stream.
		ByteStream& operator += (char byte) { m_ByteStream.push_back(byte); return *this; }

		/// Concatenation operator. Adds operand to the back of the byte stream.
		/// @param int32 - integer to add.
		/// @return Reference to the stream.
		ByteStream& operator += (int32_t int32) { m_ByteStream.insert(std::end(m_ByteStream), reinterpret_cast<const BYTE*>(&int32), reinterpret_cast<const BYTE*>(&int32) + sizeof(int32)); return *this; }

		/// Concatenation operator. Adds operand to the back of the byte stream.
		/// @param string - wide string to add.
		/// @return Reference to the stream.
		ByteStream& operator += (const std::wstring& string)
			{ m_ByteStream.insert(std::end(m_ByteStream), reinterpret_cast<const BYTE*>(string.c_str()), reinterpret_cast<const BYTE*>(string.c_str()) + string.size() * sizeof(wchar_t)); return *this; }

		/// Concatenation operator. Adds operand to the back of the byte stream.
		/// @param byteStream - stream to add.
		/// @return Reference to the stream.
		template <size_t size> ByteStream& operator += (BYTE(&byteStream)[size]) { m_ByteStream.insert(std::end(m_ByteStream), byteStream, byteStream + size); return *this; }

		/// Reserves internal buffer of specified size, similarly to STL structures like std::vector.
		/// @param newSize new capacity of internal buffer.
		void Reserve(size_t newSize) { m_ByteStream.reserve(newSize); }

		/// Retrieves actual stream length.
		/// @return length of internal buffer.
		size_t Size() const { return m_ByteStream.size(); }

		/// Checks if stream is empty.
		/// @return true if stream is blank.
		bool Empty() const { return m_ByteStream.empty(); }

		/// Gives address to internal buffer.
		/// @return address to the first element of internal buffer.
		const BYTE* GetBytes() const { return &m_ByteStream[0]; }

	protected:
		/// Members.
		std::vector<BYTE> m_ByteStream;												///< Underlying stream object.
	};
}
