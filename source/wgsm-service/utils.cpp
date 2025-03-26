#include "incs.h"

void RequestAdapterDNSZone(std::set<std::wstring>& vDNSZones)
{
	// Find all other DNS zones
	std::vector<BYTE> vData;
	vData.resize(1000000); // 1MB buffer for network data

	PIP_ADAPTER_ADDRESSES pCurrAddresses = (PIP_ADAPTER_ADDRESSES)vData.data();
	ULONG ulSize = static_cast<ULONG>(vData.size());

	if (ERROR_SUCCESS == GetAdaptersAddresses(AF_INET, 0, nullptr, pCurrAddresses, &ulSize))
	{
		while (pCurrAddresses)
		{
			if (pCurrAddresses->DnsSuffix && wcslen(pCurrAddresses->DnsSuffix))
			{
				vDNSZones.insert(pCurrAddresses->DnsSuffix);
			}

			pCurrAddresses = pCurrAddresses->Next;
		}
	}
}
