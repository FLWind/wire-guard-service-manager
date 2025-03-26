#include "incs.h"

CEventLog& CEventLog::getInstance()
{
	static CEventLog instance;
	return instance;
}

CEventLog::CEventLog()
	: m_bEnable(CRegKeyEx().GetRegistryFlag(WGSM_ENABLE_LOG)),
	m_hEventLog(NULL),
	m_hModule(GetModuleHandle(NULL))
{

}

CEventLog::~CEventLog()
{
	if (m_hEventLog)
	{
		DeregisterEventSource(m_hEventLog);
	}
}

BOOL CEventLog::Registry()
{
	if (nullptr == m_hModule)
	{
		return FALSE;
	}

	m_hEventLog = RegisterEventSource(NULL, LoadModuleString(IDS_CUSTOM_LOG_NAME).c_str());

	return m_hEventLog ? TRUE : FALSE;
}

void CEventLog::Report(WORD wType, UINT uID, DWORD dwErrorCode)
{
	Report(wType, LoadModuleString(uID).c_str(), dwErrorCode);
}

void CEventLog::Report(WORD wType, LPCWSTR strMessage, DWORD dwErrorCode)
{
	if (m_bEnable)
	{
		std::unique_lock<std::mutex> gl(m_mEventLock);

		ReportEvent(m_hEventLog, wType, 0, dwErrorCode, NULL, 1, 0, &strMessage, NULL);
	}
}

std::wstring CEventLog::LoadModuleString(UINT uID)
{
	static const DWORD dwModuleStringLen = 4096;
	WCHAR strModuleString[dwModuleStringLen];
	if (0 == LoadString(m_hModule, uID, strModuleString, dwModuleStringLen))
	{
		return L"";
	}

	return std::wstring(strModuleString);
}
