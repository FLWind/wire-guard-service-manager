#pragma once

class CEventLog
{
public:

    static CEventLog& getInstance();

private:
    CEventLog();

public:
    ~CEventLog();
    CEventLog(CEventLog const&) = delete;
    void operator=(CEventLog const&) = delete;

    BOOL Registry();

    void Report(WORD wType, UINT uID, DWORD dwErrorCode);
    void Report(WORD wType, LPCWSTR strMessage, DWORD dwErrorCode);

    std::wstring LoadModuleString(UINT uID);

private:
    BOOL m_bEnable;
    std::mutex m_mEventLock;
    HANDLE m_hEventLog;
    HMODULE m_hModule;
};