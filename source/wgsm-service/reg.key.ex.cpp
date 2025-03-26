#include "incs.h"

CRegKeyEx::CRegKeyEx(REGSAM samDesired)
	: m_hKey(nullptr)
{
	auto lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, WGSM_ROOT_REG_DIR_KEY, 0, samDesired, &m_hKey);
	if (ERROR_SUCCESS != lRes)
	{
		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_COULD_NOT_OPEN_ROOT_REG_KEY, lRes);
		throw lRes;
	}
}

CRegKeyEx::~CRegKeyEx()
{
	RegCloseKey(m_hKey);
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
	DWORD dwValueSize = 0;
	auto lRes = RegQueryValueEx(m_hKey, strValueName, NULL, NULL, NULL, &dwValueSize);

	return (ERROR_SUCCESS == lRes) ? TRUE : FALSE;
}


std::wstring CRegKeyEx::QueryString(LPCWSTR strValueName)
{
	DWORD dwStrPathLen = 4096 + sizeof(WCHAR);
	WCHAR strPath[4096];

	auto lRes = RegGetValue(m_hKey, NULL, strValueName, RRF_RT_REG_SZ, NULL, strPath, &dwStrPathLen);
	if (ERROR_SUCCESS != lRes)
	{
		auto strError = CEventLog::getInstance().LoadModuleString(IDS_COULD_NOT_READ_REG_VALUE);
		strError += strValueName;

		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, strError.c_str(), lRes);
		return L"";
	}

	return strPath;
}

BOOL CRegKeyEx::SetString(LPCWSTR strValueName, std::wstring strValue)
{
	auto lRes = RegSetValueEx(m_hKey, strValueName, 0, REG_EXPAND_SZ, reinterpret_cast<const BYTE*>(strValue.c_str()), static_cast<DWORD>(strValue.size() * sizeof(WCHAR)));
	if (ERROR_SUCCESS != lRes)
	{
		auto strError = CEventLog::getInstance().LoadModuleString(IDS_COULD_NOT_SET_REG_VALUE);
		strError += strValueName;

		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, strError.c_str(), lRes);
		return FALSE;
	}

	return TRUE;
}

DWORD CRegKeyEx::QueryDword(LPCWSTR strValueName)
{
	DWORD dwValue = 0;
	DWORD dwLen = sizeof(DWORD);

	auto lRes = RegGetValue(m_hKey, NULL, strValueName, RRF_RT_REG_DWORD, NULL, &dwValue, &dwLen);
	if (ERROR_SUCCESS != lRes)
	{
		auto strError = CEventLog::getInstance().LoadModuleString(IDS_COULD_NOT_READ_REG_VALUE);
		strError += strValueName;

		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, strError.c_str(), lRes);
		return 0;
	}

	return dwValue;
}

BOOL CRegKeyEx::SetDword(LPCWSTR strValueName, DWORD dwValue)
{
	auto lRes = RegSetValueEx(m_hKey, strValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwValue), sizeof(DWORD));
	if (ERROR_SUCCESS != lRes)
	{
		auto strError = CEventLog::getInstance().LoadModuleString(IDS_COULD_NOT_SET_REG_VALUE);
		strError += strValueName;

		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, strError.c_str(), lRes);
		return FALSE;
	}

	return TRUE;
}

std::vector<BYTE> CRegKeyEx::QueryVector(LPCWSTR strValueName)
{
	std::vector<BYTE> vData;
	DWORD dwDataLen = 4096;
	vData.resize(dwDataLen);

	auto lRes = RegGetValue(m_hKey, NULL, strValueName, RRF_RT_REG_BINARY, NULL, vData.data(), &dwDataLen);
	if (ERROR_SUCCESS != lRes)
	{
		auto strError = CEventLog::getInstance().LoadModuleString(IDS_COULD_NOT_READ_REG_VALUE);
		strError += strValueName;

		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, strError.c_str(), lRes);
		return std::vector<BYTE>();
	}

	vData.resize(dwDataLen);

	return vData;
}

BOOL CRegKeyEx::SetVector(LPCWSTR strValueName, const std::vector<BYTE>& vValue)
{
	auto lRes = RegSetValueEx(m_hKey, strValueName, 0, REG_BINARY, vValue.data(), static_cast<DWORD>(vValue.size()));
	if (ERROR_SUCCESS != lRes)
	{
		auto strError = CEventLog::getInstance().LoadModuleString(IDS_COULD_NOT_SET_REG_VALUE);
		strError += strValueName;

		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, strError.c_str(), lRes);
		return FALSE;
	}

	return TRUE;
}
