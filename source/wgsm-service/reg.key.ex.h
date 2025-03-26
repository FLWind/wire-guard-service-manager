#pragma once

#define WGSM_ROOT_REG_DIR_KEY L"SOFTWARE\\WGServiceManager"

// Application properties
#define WGSM_ENABLE_LOG L"EnableLog"
#define WGSM_WIRE_GUARD_SERVICE_NAME L"WireGuardServiceName"
#define WGSM_AUTO_CONNECT_DNS_ZONE L"AutoConnectDNSZone"

class CRegKeyEx
{
public:
    CRegKeyEx(REGSAM samDesired = KEY_READ);
    ~CRegKeyEx();

    BOOL GetRegistryFlag(LPCWSTR strValueName);

    BOOL IsExistsValue(LPCWSTR strValueName);

    std::wstring QueryString( LPCWSTR strValueName );
    BOOL SetString(LPCWSTR strValueName, std::wstring strValue);
   
    DWORD QueryDword(LPCWSTR strValueName);
    BOOL SetDword(LPCWSTR strValueName, DWORD dwValue);

    std::vector<BYTE> QueryVector(LPCWSTR strValueName);
    BOOL SetVector(LPCWSTR strValueName, const std::vector<BYTE>& vValue);

private:
    HKEY m_hKey;
};
