#include "incs.h"

CRegKeyEx::CRegKeyEx(REGSAM samDesired)
	: m_hKey(nullptr)
{
	RegOpenKeyExW(HKEY_LOCAL_MACHINE, WGSM_ROOT_REG_DIR_KEY, 0, samDesired, &m_hKey); // ignore errors
}

CRegKeyEx::~CRegKeyEx()
{
	if (m_hKey)
	{
		RegCloseKey(m_hKey);
	}	
}

BOOL CRegKeyEx::GetRegistryFlag(LPCWSTR strValueName)
{
	if (IsExistsValue(strValueName))
	{
		return (0 == QueryDword(strValueName)) ? FALSE : TRUE;
	}

	return FALSE;
}

BOOL CRegKeyEx::IsExistsValue(LPCWSTR strValueName)
{
	if (!m_hKey)
	{
		return FALSE;
	}

	DWORD dwValueSize = 0;
	auto lRes = RegQueryValueEx(m_hKey, strValueName, NULL, NULL, NULL, &dwValueSize);

	return (ERROR_SUCCESS == lRes) ? TRUE : FALSE;
}


std::wstring CRegKeyEx::QueryString(LPCWSTR strValueName)
{
	if (!m_hKey)
	{
		return L"";
	}

	DWORD dwStrPathLen = 4096 + sizeof(WCHAR);
	WCHAR strPath[4096];

	auto lRes = RegGetValue(m_hKey, NULL, strValueName, RRF_RT_REG_SZ, NULL, strPath, &dwStrPathLen);
	if (ERROR_SUCCESS != lRes)
	{
		return L"";
	}

	return strPath;
}

DWORD CRegKeyEx::QueryDword(LPCWSTR strValueName)
{
	if (!m_hKey)
	{
		return 0;
	}

	DWORD dwValue = 0;
	DWORD dwLen = sizeof(DWORD);

	auto lRes = RegGetValue(m_hKey, NULL, strValueName, RRF_RT_REG_DWORD, NULL, &dwValue, &dwLen);
	if (ERROR_SUCCESS != lRes)
	{
		return 0;
	}

	return dwValue;
}
