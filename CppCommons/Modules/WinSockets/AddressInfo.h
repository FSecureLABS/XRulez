#pragma once

/// AddressInfo, a wrapper for ADDRINFOW structure.
struct AddressInfo
{
	/// Public ctor.
	/// @param address ADDRINFO object.
	AddressInfo(const ADDRINFOW& address) : m_AddressInfo(address) { }

	/// Creates a socket and connects it.
	/// @param socketObject output socket.
	/// @return XWsaError.
	CppTools::XError<WinSockets::WsaError> Connect(OUT WinSockets::Socket& socketObject);

	/// Translates internal ADDRINFO object into socket type string.
	/// @return Socket type string.
	std::wstring GetSocketTypeString() const;

	/// Translates internal ADDRINFO object into protocol type string.
	/// @return Protocol type string.
	std::wstring GetProtocolString() const;

	/// Translates internal ADDRINFO object into string containing flags in hex.
	/// @return String containing flags in hex.
	std::wstring GetFlagsString() const;

	/// Translates internal ADDRINFO object into address family string.
	/// @return Address family string.
	std::wstring GetFamilyString() const;

	/// Retrieves canonical name from internal ADDRINFO object.
	/// @return Canonical name (or L"(null)" if void).
	std::wstring GetCanonicalNameString() const { return m_AddressInfo.ai_canonname ? m_AddressInfo.ai_canonname : L"(null)"; }

private:
	ADDRINFOW m_AddressInfo;													///< Underlying ADDRINFOW object.
};
