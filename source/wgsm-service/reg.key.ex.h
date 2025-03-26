#pragma once

#define WGSM_ROOT_REG_DIR_KEY L"SOFTWARE\\WireGuard Service Manager"

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
    DWORD QueryDword(LPCWSTR strValueName);

private:
    HKEY m_hKey;
};
