#include "incs.h"

#define SERVICE_NAME			L"WGSM"

#define __CHECK_RPC_STATUS(lName) \
		if(RPC_S_OK != lName) \
		{ \
			return; \
		}

static HANDLE					g_StopSignal;
static SERVICE_STATUS_HANDLE	g_HandleStatus;
static SERVICE_STATUS			g_ServiceStatus;

/****************************************************************************************
 *  Functions
 */

void WINAPI ControlHandler(DWORD request) {

    if (SERVICE_CONTROL_STOP == request || SERVICE_CONTROL_SHUTDOWN == request)
    {
        CService::Finalization();

        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_HandleStatus, &g_ServiceStatus);

        SetEvent(g_StopSignal);
    }
    else
    {
        SetServiceStatus(g_HandleStatus, &g_ServiceStatus);
    }
}

void StartListen()
{
    // Removes an interface from the RPC run-time library registry IWGServiceManager
    RpcServerUnregisterIf(IWGServiceManager_v1_0_s_ifspec, nullptr, 0);

    // Listen only local RPC
    __CHECK_RPC_STATUS(RpcServerUseProtseq(reinterpret_cast<RPC_WSTR>(const_cast<LPTSTR>(L"ncalrpc")), 1, nullptr));

    // Registers an interface with the RPC run-time library
    __CHECK_RPC_STATUS(RpcServerRegisterIfEx(IWGServiceManager_v1_0_s_ifspec, nullptr, nullptr, RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, RPC_C_LISTEN_MAX_CALLS_DEFAULT, nullptr));

    // Get binding handles over which remote procedure calls can be received
    RPC_BINDING_VECTOR* pbindingVector = nullptr;
    __CHECK_RPC_STATUS(RpcServerInqBindings(&pbindingVector));

    // Add server address information in the local endpoint-map database
    __CHECK_RPC_STATUS(RpcEpRegister(IWGServiceManager_v1_0_s_ifspec, pbindingVector, nullptr, nullptr));

    // To makes available of using session context concurrency from client's multiple threads
    RpcSsDontSerializeContext();

    // Start listen remote procedure calls
    __CHECK_RPC_STATUS(RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE));

    WaitForSingleObject(g_StopSignal, INFINITE);

    // Removes server-address information from the local endpoint-map database
    RpcEpUnregister(IWGServiceManager_v1_0_s_ifspec, pbindingVector, 0);

    // Delete the binding vector
    RpcBindingVectorFree(&pbindingVector);

    // Removes an interface from the RPC run-time library registry
    RpcServerUnregisterIf(IWGServiceManager_v1_0_s_ifspec, nullptr, 0);
}

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;

    g_HandleStatus = RegisterServiceCtrlHandler(SERVICE_NAME, ControlHandler);

    if (!g_HandleStatus)
    {
        return;
    }

    g_StopSignal = CreateEvent(NULL, true, false, NULL);

    if (!g_StopSignal || !CService::Initialization())
    {
        if (g_StopSignal) {
            CloseHandle(g_StopSignal);
        }
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = -1;
        SetServiceStatus(g_HandleStatus, &g_ServiceStatus);
        return;
    }

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_HandleStatus, &g_ServiceStatus);

    StartListen();

    CloseHandle(g_StopSignal);
}

int wmain(int argc, wchar_t* argv[])
{
    SERVICE_TABLE_ENTRY ServiceTable[1];

    ServiceTable[0].lpServiceName = const_cast<LPWSTR>(SERVICE_NAME);
    ServiceTable[0].lpServiceProc = ServiceMain;

    StartServiceCtrlDispatcher(ServiceTable);

    return 0;
}