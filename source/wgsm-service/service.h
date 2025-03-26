#pragma once

class CService
{
	static CService* m_pInst;

	CService();

	BOOL Run();
	BOOL Stop();

public:
	friend VOID NETIOAPI_API_ IpInterfaceChangeCallback(_In_ PVOID CallerContext, _In_ PMIB_IPINTERFACE_ROW Row OPTIONAL, _In_ MIB_NOTIFICATION_TYPE NotificationType);

	~CService();

	static BOOL Initialization();
	static CService* Inst();
	static void Finalization();

	HRESULT GetState(enState* enWGState);
	HRESULT Connect();
	HRESULT Disconnect();

private:
	// Lock
	std::mutex m_mWGServiceLock;

	// Parameters
	std::wstring m_strServiceName;
	std::wstring m_strDNSZone;

	// Serice manager
	SC_HANDLE m_hSCManager;
	SC_HANDLE m_hService;

	// Ethernet adapters update
	HANDLE m_hNotify;
};