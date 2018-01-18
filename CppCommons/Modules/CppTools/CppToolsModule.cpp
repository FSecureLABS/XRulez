#include "StdAfx.h"

#include "CppToolsModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Definition of static member converter.
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> CppTools::StringConversions::m_Converter;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<bool> CppTools::GetCurrentTimeAndDate(OUT std::string& retVal, const std::string &format)
{
	// A buffer of 128 bytes should be enough for most conversions.
	char buffer[128];
	tm timeinfo;

	// Get local time and convert it to string.
	auto currentTime = std::time(nullptr);
	if (currentTime == -1 || localtime_s(&timeinfo, &currentTime) || !strftime(buffer, sizeof(buffer), format.c_str(), &timeinfo))
		return false;

	// Copy result to output buffer.
	retVal = buffer;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<BYTE> CppTools::ReadBinaryFile(std::filesystem::path const& fileLocation)
{
	// Open the file.
	std::vector<BYTE> retVector;
	std::ifstream file(fileLocation, std::ios_base::binary);
	if (!file)
		throw XException("Error opening file.", XEXCEPTION_PARAMS);

	// Copy contents to a buffer.
	std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(retVector));

	// And return it.
	return retVector;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CppTools::WriteBinaryFile(std::filesystem::path const& fileLocation, std::vector<BYTE> const& data)
{
	// Stream contents from buffer to the file.
	std::ofstream file(fileLocation, std::ios_base::binary);
	std::copy(data.begin(), data.end(), std::ostreambuf_iterator<char>(file));
}
