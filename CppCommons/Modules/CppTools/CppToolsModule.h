#pragma once

// Module's precompiled header pack inclusion.
#if !defined(CPPTOOLSMODULE_HH_INCLUDED)
#	include "CppToolsModule.hh"
#endif

// Non-copyable mixin/idiom.
#include "NonCopyable.h"

// ScopeGuard class and macros.
#include "ScopeGuard.h"

// XError class and macros.
#include "XError.h"

// ByteStream class.
#include "ByteStream.h"

// PointerObject template.
#include "PointerObject.h"

// UniqueReleasePtr structure.
#include "UniqueReleasePtr.h"

// Base64 cipher.
#include "Base64.h"

/// @brief Useful tools that enhance C++ language and simplify its usage.
namespace CppTools
{
	/// Tristate element of: yes, no, maybe. Can be used when there is need to return flag, with a possibility to come across an error or undetermined result.
	enum class EYesNoMaybe { Yes, No, Maybe };

	/// Reads file to a BYTE vector.
	/// @param fileLocation path to input file.
	/// @return Vector containing file BYTEs.
	std::vector<BYTE> ReadBinaryFile(std::filesystem::path const& fileLocation);

	/// Writes BYTE vector to a file.
	/// @param fileLocation path to output file.
	/// @param data BYTE vector to put in to the file.
	void WriteBinaryFile(std::filesystem::path const& fileLocation, std::vector<BYTE> const& data);

	/// Gets current date and time and formats it with specified schema to string.
	/// @param retVal string containing formated current date and time.
	/// @param format schema of convertion.
	/// @return true if succeeded.
	/// @see http://www.cplusplus.com/reference/ctime/strftime/
	CppTools::XError<bool> GetCurrentTimeAndDate(OUT std::string& retVal, const std::string &format = "%Y-%m-%d %H:%M:%S");

	/// String conversions.
	struct StringConversions
	{
		/// String conversion template. Usage: auto mbcs = CppTools::StringConversions::Convert<std::string>(std::to_wstring(42));
		template<typename DestinationType, typename ImplicitSourceType> static DestinationType Convert(ImplicitSourceType)
		{
			static_assert(false, "Converter works on string types only (string, wstring, char*, wchar_t*)."); return nullptr;
		}

		// Specializations for standard strings.
		template<> static std::string Convert(std::string string) { return string; }
		template<> static std::wstring Convert(std::wstring string) { return string; }
		template<> static std::string Convert(std::wstring string) { return Unicode2Mbcs(string); }
		template<> static std::wstring Convert(std::string string) { return Mbcs2Unicode(string); }

		// Specializations for raw string pointers.
		template<> static std::string Convert(char* string) { return string; }
		template<> static std::wstring Convert(wchar_t* string) { return string; }
		template<> static std::string Convert(wchar_t* string) { return Unicode2Mbcs(string); }
		template<> static std::wstring Convert(char* string) { return Mbcs2Unicode(string); }

		template<> static std::string Convert(char const* string) { return string; }
		template<> static std::wstring Convert(wchar_t const* string) { return string; }
		template<> static std::string Convert(wchar_t const* string) { return Unicode2Mbcs(string); }
		template<> static std::wstring Convert(char const* string) { return Mbcs2Unicode(string); }

		/// Converts UNICODE string to multi-byte character string.
		/// @param unicodeString string to convert.
		/// @return converted string.
		/// @see Mbcs2Unicode.
		static std::string Unicode2Mbcs(const std::wstring& unicodeString) { return m_Converter.to_bytes(unicodeString); }

		/// Converts multi-byte character string to UNICODE string.
		/// @param mbcsString string to convert.
		/// @return converted string.
		/// @see Unicode2Mbcs.
		static std::wstring Mbcs2Unicode(const std::string& mbcsString) { return m_Converter.from_bytes(mbcsString); }

		/// Converts UNICODE string to TCHAR string.
		/// @param unicodeString string to convert. 
		/// @return converted string.
		/// @see Tstring2Unicode.
		static std::tstring Unicode2Tstring(const std::wstring& unicodeString)
		{
#			ifdef _UNICODE
				return unicodeString;
#			else
				return Unicode2Mbcs(unicodeString);
#			endif
		}

		/// Converts TCHAR string to UNICODE string.
		/// @param tstring string to convert.
		/// @return converted string.
		/// @see Unicode2Tstring.
		static std::wstring Tstring2Unicode(const std::tstring& tstring)
		{
#			ifdef _UNICODE
				return tstring;
#			else
				return Mbcs2Unicode(tstring);
#			endif
		}

		/// Converts multi-byte character string to TCHAR string.
		/// @param mbcsString string to convert.
		/// @return converted string.
		/// @see Tstring2Mbcs.
		static std::tstring Mbcs2Tstring(const std::string& mbcsString)
		{
#			ifdef _UNICODE
				return Mbcs2Unicode(mbcsString);
#			else
				return mbcsString;
#			endif
		}

		/// Converts TCHAR string to multi-byte character string.
		/// @param tstring string to convert.
		/// @return converted string.
		/// @see Mbcs2Tstring.
		static std::string Tstring2Mbcs(const std::tstring& tstring)
		{
#			ifdef _UNICODE
				return Unicode2Mbcs(tstring);
#			else
				return tstring;
#			endif
		}

		/// Creates a copy of a multi-byte character string that contains only lower case characters.
		/// @param string string to convert.
		/// @return Converted string.
		/// @see ToLowerInplace.
		static std::string ToLower(std::string const& string) { std::string retVal = string; ToLowerInplace(retVal); return retVal; }

		/// Converts all characters in a multi-byte character string to lower case.
		/// @param string string to convert.
		/// @return Converted string.
		/// @see ToLower.
		static void ToLowerInplace(INOUT std::string& string) { for (auto& c : string) c = static_cast<char>(tolower(c)); }

		/// Creates a copy of an UNICODE string that contains only lower case characters.
		/// @param string string to convert.
		/// @return Converted string.
		/// @see ToLowerInplace.
		static std::wstring ToLower(std::wstring const& string) { std::wstring retVal = string; ToLowerInplace(retVal); return retVal; }

		/// Converts all characters in a UNICODE string to lower case.
		/// @param string string to convert.
		/// @return Converted string.
		/// @see ToLower.
		static void ToLowerInplace(INOUT std::wstring& string) { for (auto& c : string) c = static_cast<wchar_t>(tolower(c)); }


		/// Converts any integer number to hexadecimal multi-byte character string.
		/// @param integer number to convert.
		/// @return string containing hexadecimal number.
		/// @see NumberToHexString, NumberToHexTString.
		template <typename T> static std::wstring NumberToHexWString(T integer)
			{ std::wstringstream stream; stream << L"0x" << std::setfill(L'0') << std::setw(sizeof(T) * 2) << std::hex << integer; return stream.str(); }

		/// Converts any integer number to hexadecimal UNICODE string.
		/// @param integer number to convert.
		/// @return string containing hexadecimal number.
		/// @see NumberToHexWString, NumberToHexTString.
		template <typename T> static std::string NumberToHexString(T integer)
			{ std::stringstream stream; stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << integer; return stream.str(); }

		/// Converts any integer number to hexadecimal t-string.
		/// @param integer number to convert.
		/// @return string containing hexadecimal number.
		/// @see NumberToHexString, NumberToHexWString.
		template <typename T> static std::tstring NumberToHexTString(T integer)
			{ std::tstringstream stream; stream << TEXT("0x") << std::setfill(TEXT('0')) << std::setw(sizeof(T) * 2) << std::hex << integer; return stream.str(); }

	private:
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_Converter;					///< Converter object.
	};
}

// XException and InvalidParam structs and macros.
#include "XException.h"
