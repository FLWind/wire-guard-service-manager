#include "incs.h"

// Include RPC server stub
#include <service_s.c>

long IWGSM_WGGetState(/*[in]*/ handle_t IDL_handle, /*[out]*/enState* enWGState)
{
	return CService::Inst()->GetState(enWGState);
}

long IWGSM_WGConnect(/*[in]*/ handle_t IDL_handle)
{
	return CService::Inst()->Connect();
}

long IWGSM_WGDisconnect(/*[in]*/ handle_t IDL_handle)
{
	return CService::Inst()->Disconnect();
}


//
//  FUNCTIONS: MIDL_user_allocate and MIDL_user_free
//
//  PURPOSE: Used by stubs to allocate and free memory
//           in standard RPC calls. Not used when
//           [enable_allocate] is specified in the .acf.
//
//
//  PARAMETERS:
//    See documentations.
//
//  RETURN VALUE:
//    Exceptions on error.  This is not required,
//    you can use -error allocation on the midl.exe
//    command line instead.
//
//
void* __RPC_USER MIDL_user_allocate(size_t size)
{
	return(HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size));
}

void __RPC_USER MIDL_user_free(void* pointer)
{
	HeapFree(GetProcessHeap(), 0, pointer);
}