#pragma once

class CService
{
public:
	CService();

	~CService();

	HRESULT OpenInterface();

	HRESULT GetState(enState& enWGState);

	HRESULT Connect();

	HRESULT Disconnect();

private:
	void CloseInterface();

private:
	LPTSTR m_strBinding;
	RPC_BINDING_HANDLE m_hBinding;
};