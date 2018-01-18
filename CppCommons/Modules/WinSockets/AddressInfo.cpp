#include "StdAfx.h"
#include "WinSocketsModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<WinSockets::WsaError> WinSockets::AddressInfo::Connect(OUT WinSockets::Socket& socketObject)
{
	// Create a socket.
	auto rawSocket = socket(m_AddressInfo.ai_family, m_AddressInfo.ai_socktype, m_AddressInfo.ai_protocol);
	if (rawSocket == INVALID_SOCKET)
		return WinSockets::WsaError();

	// Connect to server.
	if (connect(rawSocket, m_AddressInfo.ai_addr, static_cast<int>(m_AddressInfo.ai_addrlen)) == SOCKET_ERROR)
	{
		auto retValue = WinSockets::WsaError();
		closesocket(rawSocket);
		return retValue;
	}

	// If succeed copy socket to internal pointer and return 0.
	socketObject.m_Socket = rawSocket;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring WinSockets::AddressInfo::GetSocketTypeString() const
{
	switch (m_AddressInfo.ai_socktype)
	{
	case 0: return L"unspecified";
	case SOCK_STREAM: return L"stream";
	case SOCK_DGRAM: return L"datagram";
	case SOCK_RAW: return L"raw";
	case SOCK_RDM: return L"reliable message datagram";
	case SOCK_SEQPACKET: return L"pseudo-stream packet";
	default: return L"other (" + std::to_wstring(m_AddressInfo.ai_socktype) + L")";
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring WinSockets::AddressInfo::GetProtocolString() const
{
	switch (m_AddressInfo.ai_protocol)
	{
	case 0: return L"unspecified";
	case IPPROTO_TCP: return L"TCP";
	case IPPROTO_UDP: return L"UDP";
	default: return L"other (" + std::to_wstring(m_AddressInfo.ai_protocol) + L")";
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring WinSockets::AddressInfo::GetFlagsString() const
{
	std::wostringstream oss;
	oss << std::hex << m_AddressInfo.ai_flags;
	return oss.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring WinSockets::AddressInfo::GetFamilyString() const
{
	// Helper lambda to translate IP address (with port number) to text.
	auto TranslateInternetAddressToString = [](const ADDRINFOW& addressInfo) -> std::wstring
	{
		LPSOCKADDR ip = reinterpret_cast<LPSOCKADDR>(addressInfo.ai_addr);
		WCHAR stringBuffer[46];
		DWORD bufferLength = static_cast<DWORD>(std::size(stringBuffer));
		return WSAAddressToStringW(ip, static_cast<DWORD>(addressInfo.ai_addrlen), nullptr, stringBuffer, &bufferLength) ? L"ERROR : " + std::to_wstring(WSAGetLastError()) : stringBuffer;
	};

	switch (m_AddressInfo.ai_family)
	{
	case AF_UNSPEC: return L"unspecified";
	case AF_NETBIOS: return L"NetBIOS";
	case AF_INET: return L"IPv4 address (" + TranslateInternetAddressToString(m_AddressInfo) + L")";
	case AF_INET6: return L"IPv6 address (" + TranslateInternetAddressToString(m_AddressInfo) + L")";
	default: return L"other (" + std::to_wstring(m_AddressInfo.ai_family) + L")";
	}
}
