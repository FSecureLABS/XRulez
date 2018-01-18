#pragma once

namespace CppTools
{
	/// Static structure representing Base64 encryption/decryption.
	struct Base64
	{
		/// Encodes byte stream to base64.
		/// @param bytesToEncode byte stream to encode.
		/// @return std::string containing encoded stream.
		static std::string Encode(std::vector<BYTE> const& bytesToEncode);

		/// Decodes byte stream from base64.
		/// @param bytesToDecode byte stream to decode.
		/// @return std::vector containing decoded stream.
		static std::vector<BYTE> Decode(std::string const& bytesToDecode);

	private:
		static const std::string s_Base64Characters;							///< All base64 characters in one place.
	};
}
