#include "incs.h"

CService* CService::m_pInst = nullptr;

CService::CService() :
	m_hSCManager(nullptr),
	m_hService(nullptr),
	m_hNotify(nullptr),
	m_bManualActivated(FALSE)
{
}

VOID NETIOAPI_API_ IpInterfaceChangeCallback(_In_ PVOID CallerContext, _In_ PMIB_IPINTERFACE_ROW Row OPTIONAL, _In_ MIB_NOTIFICATION_TYPE NotificationType)
{
	// Manual mode activated
	if (CService::Inst()->m_bManualActivated)
	{
		return;
	}

	// Ignore service messages
	if (MibAddInstance != NotificationType && MibDeleteInstance != NotificationType)
	{
		return;
	}

	std::unique_lock<std::mutex> gl(CService::Inst()->m_mWGServiceLock);

	std::set<std::wstring> vDNSZones;
	RequestAdapterDNSZone(vDNSZones);

	BOOL bIsAladdinZone = (vDNSZones.end() == vDNSZones.find(CService::Inst()->m_strDNSZone)) ? FALSE : TRUE;

	while (true)
	{
		SERVICE_STATUS serviceStatus = { 0 };
		if (!QueryServiceStatus(CService::Inst()->m_hService, &serviceStatus))
		{
			CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_QUERYSERVICESTATUS_PROBLEM, GetLastError());
			return;
		}

		if (bIsAladdinZone)
		{
			switch (serviceStatus.dwCurrentState)
			{
			case SERVICE_STOPPED:
				if (0 == StartService(CService::Inst()->m_hService, 0, nullptr))
				{
					CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_STARTSERVICE_PROBLEM, GetLastError());
					return;
				}
				break;
			case SERVICE_START_PENDING:
				return;
				break;
			case SERVICE_STOP_PENDING:
				break;
			case SERVICE_RUNNING:
				return;
				break;
			case SERVICE_CONTINUE_PENDING:
				return;
				break;
			case SERVICE_PAUSE_PENDING:
				break;
			case SERVICE_PAUSED:
				if (0 == StartService(CService::Inst()->m_hService, 0, nullptr))
				{
					CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_STARTSERVICE_PROBLEM, GetLastError());
					return;
				}
				break;
			default:
				return;
				break;
			}
		}
		else
		{
			switch (serviceStatus.dwCurrentState)
			{
			case SERVICE_STOPPED:
				return;
				break;
			case SERVICE_START_PENDING:
				break;
			case SERVICE_STOP_PENDING:
				return;
				break;
			case SERVICE_RUNNING:
				if (0 == ControlService(CService::Inst()->m_hService, SERVICE_CONTROL_STOP, &serviceStatus))
				{
					CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_STOPSERVICE_PROBLEM, GetLastError());
					return;
				}
				break;
			case SERVICE_CONTINUE_PENDING:
				return;
				break;
			case SERVICE_PAUSE_PENDING:
				break;
			case SERVICE_PAUSED:
				if (0 == ControlService(CService::Inst()->m_hService, SERVICE_CONTROL_STOP, &serviceStatus))
				{
					CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_STOPSERVICE_PROBLEM, GetLastError());
					return;
				}
				break;
			default:
				return;
				break;
			}
		}
	}
}

BOOL CService::Run()
{
	// Do not start service with no logging
	if (!CEventLog::getInstance().Registry())
	{
		return FALSE;
	}

	try
	{
		// Log service start
		CEventLog::getInstance().Report(EVENTLOG_SUCCESS, IDS_SERVICE_STARTED, S_OK);

		// Read parameters from resigtry
		{
			CRegKeyEx cKey;
			m_strServiceName = cKey.QueryString(WGSM_WIRE_GUARD_SERVICE_NAME);
			m_strDNSZone = cKey.QueryString(WGSM_AUTO_CONNECT_DNS_ZONE);

			if (!m_strServiceName.size())
			{
				CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_WG_SERVICE_NAME_NOT_SET, E_FAIL);
				return FALSE;
			}
		}

		// Open services manager
		m_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
		if (nullptr == m_hSCManager)
		{
			CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_OPENSCMANAGER_PROBLEM, GetLastError());
			return FALSE;
		}

		// Open Wire Guard service
		m_hService = OpenService(m_hSCManager, m_strServiceName.c_str(), SERVICE_ALL_ACCESS);
		if (nullptr == m_hService)
		{
			CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_OPENSERVICE_PROBLEM, GetLastError());
			return FALSE;
		}

		// Register for Ethernate update
		if (m_strDNSZone.size())
		{
			auto dwResult = NotifyIpInterfaceChange(AF_INET, IpInterfaceChangeCallback, NULL, TRUE, &m_hNotify);
			if (NO_ERROR != dwResult)
			{
				CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_NOTIFYIPINTERFACECHANGE, dwResult);
				return FALSE;
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CService::Stop()
{
	if (m_hNotify)
	{
		CancelMibChangeNotify2(m_hNotify);
	}

	if (m_hService)
	{
		CloseServiceHandle(m_hService);
	}

	if (m_hSCManager)
	{
		CloseServiceHandle(m_hSCManager);
	}

	CEventLog::getInstance().Report(EVENTLOG_SUCCESS, IDS_SERVICE_STOPPED, S_OK);

	return TRUE;
}

CService::~CService()
{
}

BOOL CService::Initialization()
{
	if (!m_pInst)
	{
		m_pInst = new CService();
		if (!m_pInst->Run())
		{
			delete m_pInst;
			m_pInst = nullptr;
			return FALSE;
		}
	}

	return TRUE;
}

CService* CService::Inst()
{
	return m_pInst;
}

void CService::Finalization()
{
	if (m_pInst)
	{
		// ignore error
		m_pInst->Stop();

		delete m_pInst;
		m_pInst = nullptr;
	}
}

HRESULT CService::GetState(enState* enWGState)
{
	std::unique_lock<std::mutex> gl(m_mWGServiceLock);

	SERVICE_STATUS serviceStatus = { 0 };
	if (!QueryServiceStatus(CService::Inst()->m_hService, &serviceStatus))
	{
		auto dwLastError = GetLastError();
		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_QUERYSERVICESTATUS_PROBLEM, dwLastError);

		return HRESULT_FROM_WIN32(dwLastError);
	}

	switch (serviceStatus.dwCurrentState)
	{
	case SERVICE_STOPPED:
		*enWGState = enState::enDisconnect;
		break;
	case SERVICE_RUNNING:
		*enWGState = enState::enConnect;
		break;
	default:
		*enWGState = enState::enPending;
		break;
	}

	return S_OK;
}

HRESULT CService::Connect()
{
	m_bManualActivated = TRUE;

	std::unique_lock<std::mutex> gl(m_mWGServiceLock);

	if (0 == StartService(CService::Inst()->m_hService, 0, nullptr))
	{
		auto dwLastError = GetLastError();
		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_STARTSERVICE_PROBLEM, dwLastError);

		return HRESULT_FROM_WIN32(dwLastError);
	}

	return S_OK;
}

HRESULT CService::Disconnect()
{
	m_bManualActivated = TRUE;

	std::unique_lock<std::mutex> gl(m_mWGServiceLock);

	SERVICE_STATUS serviceStatus = { 0 };
	if (0 == ControlService(CService::Inst()->m_hService, SERVICE_CONTROL_STOP, &serviceStatus))
	{
		DWORD dwLastError = GetLastError();
		CEventLog::getInstance().Report(EVENTLOG_ERROR_TYPE, IDS_STOPSERVICE_PROBLEM, dwLastError);

		return HRESULT_FROM_WIN32(dwLastError);
	}

	return S_OK;
}
