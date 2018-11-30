#pragma once

/// Microsoft Windows registry function wrapper namespace.
namespace WinTools::Registry
{
	/// Enumeration type to have all predefined root keys in one place.
	enum class HKey : ULONG_PTR
	{
		ClassesRoot = reinterpret_cast<ULONG_PTR>(HKEY_CLASSES_ROOT),
		CurrentUser = reinterpret_cast<ULONG_PTR>(HKEY_CURRENT_USER),
		LocalMachine = reinterpret_cast<ULONG_PTR>(HKEY_LOCAL_MACHINE),
		CurrentConfig = reinterpret_cast<ULONG_PTR>(HKEY_CURRENT_CONFIG),
		Users = reinterpret_cast<ULONG_PTR>(HKEY_USERS),
		PerformanceData = reinterpret_cast<ULONG_PTR>(HKEY_PERFORMANCE_DATA),	///< Windows NT/2000.
		DynData = reinterpret_cast<ULONG_PTR>(HKEY_DYN_DATA),					///< Windows 95/98.
	};

	/// Converts hive name to hive key.
	/// @param hiveName string containing hive name.
	/// @param hive output param that will be filled with appropriate registry hive key if the conversion was possible.
	/// @return true if conversion  was possible, and false if hiveName contains unknown hive name.
	CppTools::XError<bool> HiveFromName(std::string const& hiveName, OUT WinTools::Registry::HKey &hive);

	/// Enumerates sub-keys of specified registry node.
	/// @return ERROR_SUCCESS on full success, ERROR_MORE_DATA if was able to open specified registry key, but couldn't retrieve information about at least one of its sub-keys. Otherwise returns error
	///  code that occurred while trying to open specified key.
	CppTools::XError<HRESULT> EnumSubKeys(HKey rootKey, const std::wstring& subKey, OUT std::vector<std::wstring>& subKeysNames);

	/// Queries registry for a value of specified type in selected key/subKey path.
	/// @param rootKey root key (e.g. CurrentUser for HKEY_CURRENT_USER).
	/// @param subKey rest of the path to value.
	/// @param valueName name of value to query.
	/// @param type expected type of queried value.
	/// @return If there's no value with this name at key/subKey location then WinTools::SystemErrorToHresult(ERROR_FILE_NOT_FOUND) will be returned. If a value with specified name exists, but has
	///  other type than provided in type param then WinTools::SystemErrorToHresult(ERROR_UNSUPPORTED_TYPE) will be returned. If value exists and has the same type as provided then ERROR_SUCCESS will
	///  be returned. In all other cases you'll get a HRESULT error.
	CppTools::XError<HRESULT> QueryValueExistance(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, DWORD type = RRF_RT_REG_SZ);
	
	/// Prawdopodobnie te same remarksy co wy¿ej.
	CppTools::XError<HRESULT> GetValue(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, OUT DWORD& value);

	/// Retrieves a string value.
	CppTools::XError<HRESULT> GetValue(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, OUT std::wstring& value);
	
	/// Sets DWORD value in specified registry key. If the value doesn't exist, creates it.
	CppTools::XError<HRESULT> SetValue(HKey rootKey, const std::wstring& subKey, const std::wstring& valueName, DWORD const& value);

	/// Sets string value (REG_SZ) in specified registry key. If the value doesn't exist, creates it.
	/// @param fullKey opened registry key (e.g. by previous call to CreateKey) in which value of provided name should be set.
	/// @param valueName name of value to set.
	/// @param value string to set specified value to.
	/// @return HRESULT value redirected from a call to RegSetValueEx.
	CppTools::XError<HRESULT> SetStringValue(const HKEY &fullKey, const std::wstring& valueName, const std::wstring& value);

	/// Creates a registry key. If the key has already existed, sets output flag. Creates all intermediate keys (e.g. if requested to create "test1\test2\" and "test1" doesn't exist, it creates "test1", then "test2" inside).
	/// @param rootKey root key (e.g. CurrentUser for HKEY_CURRENT_USER)
	/// @param subKey rest of the path to key.
	/// @param key output key object.
	/// @param keyAlreadyExisted if non-nullptr then will be set to true if specified key already existed or false if it haven't. Can be nullptr.
	/// @return S_OK if succeed to create (or open - if key already existed) specified key. Otherwise HRESULT error code.
	CppTools::XError<HRESULT> CreateKey(HKey rootKey, const std::wstring& subKey, OUT HKEY& key, OUT bool* keyAlreadyExisted);

	/// Deletes a registry value.
	/// @param rootKey root key (e.g. CurrentUser for HKEY_CURRENT_USER).
	/// @param subKey rest of the path to value.
	/// @param valueName name of value to query.
	/// @return HRESULT value redirected from a call to RegDeleteKeyValue.
	CppTools::XError<HRESULT> DeleteValue(HKey rootKey, std::wstring const& subKey, std::wstring const& valueName);

	/// Closes the registry key opened before with e.g. call to CreateKey function.
	/// @param key registry key to close.
	/// @return S_OK if succeed to close the key, otherwise HRESULT error code.
	CppTools::XError<HRESULT> CloseKey(HKEY& key);
}
