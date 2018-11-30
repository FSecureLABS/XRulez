#include "StdAfx.h"
#include "Registry.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<bool> WinTools::Registry::HiveFromName(std::string const& hiveName, OUT Registry::HKey &hive)
{
	// Use simple macro to DRY.
#	define TRY_PARSE_HIVE(x) if (!hiveName.compare(#x)) hive = (WinTools::Registry::HKey)(LONGLONG)(x);
	TRY_PARSE_HIVE(HKEY_CLASSES_ROOT)
	else TRY_PARSE_HIVE(HKEY_CURRENT_USER)
	else TRY_PARSE_HIVE(HKEY_LOCAL_MACHINE)
	else TRY_PARSE_HIVE(HKEY_CURRENT_CONFIG)
	else TRY_PARSE_HIVE(HKEY_USERS)
	else TRY_PARSE_HIVE(HKEY_PERFORMANCE_DATA)
	else TRY_PARSE_HIVE(HKEY_DYN_DATA)
	else return false;
#	undef TRY_PARSE_HIVE

	// If we're here then hive contains properly parsed registry hive key and we may return success.
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::QueryValueExistance(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, DWORD type)
{
	return WinTools::SystemErrorToHresult(RegGetValueW(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), valueName.c_str(), type, nullptr, nullptr, nullptr));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::GetValue(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, OUT DWORD& value)
{
	DWORD dummy = sizeof(DWORD);
	return WinTools::SystemErrorToHresult(RegGetValueW(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), valueName.c_str(), RRF_RT_REG_DWORD, nullptr, &value, &dummy));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::GetValue(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, OUT std::wstring& value)
{
	// Read size.
	DWORD size;
	if (HRESULT result = WinTools::SystemErrorToHresult(RegGetValueW(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), valueName.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &size)))
		return WinTools::SystemErrorToHresult(result);

	// Read and return the value.
	value.resize(size);
	return WinTools::SystemErrorToHresult(RegGetValueW(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), valueName.c_str(), RRF_RT_REG_SZ, nullptr, &value[0], &size));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::CreateKey(HKey rootKey, const std::wstring& subKey, OUT HKEY& key, OUT bool* keyAlreadyExisted)
{
	DWORD dwDisposition;
	if (HRESULT retVal = RegCreateKeyExW(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, &dwDisposition))
		return WinTools::SystemErrorToHresult(retVal);

	if (keyAlreadyExisted)
		*keyAlreadyExisted = dwDisposition == REG_OPENED_EXISTING_KEY;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::SetValue(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, DWORD const& value)
{
	return WinTools::SystemErrorToHresult(RegSetKeyValue(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), valueName.c_str(), REG_DWORD, &value, sizeof(DWORD)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::SetStringValue(const HKEY &fullKey, const std::wstring& valueName, const std::wstring& value)
{
	return WinTools::SystemErrorToHresult(RegSetValueExW(fullKey, valueName.c_str(), 0, RRF_RT_REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), static_cast<DWORD>(value.size() * sizeof(wchar_t))));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::EnumSubKeys(HKey rootKey, const std::wstring& subKey, OUT std::vector<std::wstring>& subKeysNames)
{
	// First, open specified key.
	HKEY hKey;
	if (auto status = RegOpenKeyEx(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), 0, KEY_READ, &hKey))
		return WinTools::SystemErrorToHresult(status);
	SCOPE_GUARD{ RegCloseKey(hKey); };

	// Get the class name and the value count. 
	DWORD numberOfSubKeys = 0;
	if (auto retCode = RegQueryInfoKey(hKey, nullptr, nullptr, nullptr, &numberOfSubKeys, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
		return WinTools::SystemErrorToHresult(retCode);

	// Enumerate the sub-keys.
	DWORD returnValue = ERROR_SUCCESS;
	for (decltype(numberOfSubKeys) i = 0; i < numberOfSubKeys; ++i)
	{
		// Retrieve a single key.
		TCHAR subKeyName[MAX_PATH];
		DWORD subKeyNameSize = MAX_PATH;
		if (auto retCode = RegEnumKeyEx(hKey, i, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr))
			returnValue = ERROR_MORE_DATA;
		else
			subKeysNames.push_back(subKeyName); 
	}

	// At this point subKeysNames vector contains all accessible sub key names.
	return returnValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::DeleteValue(HKey rootKey, std::wstring const& subKey, std::wstring const& valueName)
{
	return WinTools::SystemErrorToHresult(RegDeleteKeyValueW(reinterpret_cast<HKEY>(rootKey), subKey.c_str(), valueName.c_str()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CppTools::XError<HRESULT> WinTools::Registry::CloseKey(HKEY& key)
{
	return WinTools::SystemErrorToHresult(RegCloseKey(key));
}
