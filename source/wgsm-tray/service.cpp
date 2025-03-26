#include "framework.h"
#include "service.h"

#include <service_c.c>

void trans_func(unsigned int u, EXCEPTION_POINTERS*)
{
	throw HRESULT_FROM_WIN32(u);
}

#define __CHECK_RPC_STATUS(lName) \
{ \
		_set_se_translator(trans_func); \
		HRESULT e = lName; \
		if(RPC_S_OK != e) \
		{ \
			throw HRESULT_FROM_WIN32(e); \
		} \
}

void* __RPC_USER MIDL_user_allocate(size_t s)
{
	BYTE* p = new BYTE[s];
	return (void*)p;
}

void __RPC_USER MIDL_user_free(void* p)
{
	if (p)
	{
		delete[] p;
	}
}

CService::CService()
	: m_strBinding(nullptr),
	m_hBinding(nullptr)
{
}

CService::~CService()
{
	CloseInterface();
}

HRESULT CService::OpenInterface()
{
	if (!m_hBinding)
	{
		try
		{
			__CHECK_RPC_STATUS(RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", (RPC_WSTR)L"", 0, 0, (RPC_WSTR*)&m_strBinding));

			__CHECK_RPC_STATUS(RpcBindingFromStringBinding((RPC_WSTR)m_strBinding, &m_hBinding));

			__CHECK_RPC_STATUS(RpcEpResolveBinding(m_hBinding, IWGServiceManager_v1_0_c_ifspec));
		}
		catch (const HRESULT& hrErr)
		{
			CloseInterface();

			return hrErr;
		}
		catch (...)
		{
			CloseInterface();

			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CService::GetState(enState& enWGState)
{
	OpenInterface();

	try
	{
		__CHECK_RPC_STATUS(IWGSM_WGGetState(m_hBinding, &enWGState));
	}
	catch (const HRESULT& hrErr)
	{
		return hrErr;
	}
	catch (...)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CService::Connect()
{
	OpenInterface();

	try
	{
		__CHECK_RPC_STATUS(IWGSM_WGConnect(m_hBinding));
	}
	catch (const HRESULT& hrErr)
	{
		return hrErr;
	}
	catch (...)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CService::Disconnect()
{
	OpenInterface();

	try
	{
		__CHECK_RPC_STATUS(IWGSM_WGDisconnect(m_hBinding));
	}
	catch (const HRESULT& hrErr)
	{
		return hrErr;
	}
	catch (...)
	{
		return E_FAIL;
	}

	return S_OK;
}

void CService::CloseInterface()
{
	try
	{
		if (m_strBinding)
		{
			RpcStringFree((RPC_WSTR*)&m_strBinding);
		}
	}
	catch (...) // ignore all errors
	{
	}

	try
	{
		if (m_hBinding)
		{
			RpcBindingFree(&m_hBinding);
		}
	}
	catch (...) // ignore all errors
	{
	}

	m_strBinding = nullptr;
	m_hBinding = nullptr;
}
