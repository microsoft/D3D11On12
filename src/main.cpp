// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

#define INITGUID
#include <coguid.h>

#define D3D11ON12_PROVIDER_GUID (0xa0ab5aac, 0xe0a4, 0x4f10, 0x83, 0xc6, 0x31, 0x93, 0x9c, 0x60, 0x4f, 0xd9)
#define D3D11ON12_PROVIDER_DESC "Microsoft.Windows.Graphics.D3D11On12"

TRACELOGGING_DEFINE_PROVIDER(
    g_hD3D11On12TraceLoggingProvider,
    D3D11ON12_PROVIDER_DESC,
    D3D11ON12_PROVIDER_GUID,
    TraceLoggingOptionMicrosoftTelemetry()
);


typedef BOOL ( WINAPI * PCOMPATVALUE ) ( 
    __in __nullterminated const char * pValueName, 
    __out_opt UINT64 * pValue );

bool GetCompatValue(const char* str, UINT64* pValue)
{
    static PCOMPATVALUE pfnCompatValue = nullptr;
    if (!pfnCompatValue)
    {
        pfnCompatValue = reinterpret_cast<PCOMPATVALUE>(GetProcAddress(GetModuleHandleW(L"dxgi.dll"), "CompatValue"));
    }
    if (pfnCompatValue)
    {
        return pfnCompatValue(str, pValue);
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------
extern "C" extern BOOL WINAPI DllMain(
    HINSTANCE hInstance,
    UINT dwReason,
    LPVOID ) noexcept
{
    // Warning, do not call outside of this module, except for functions located in kernel32.dll. BUT, do not call LoadLibrary
    // nor FreeLibrary, either. Nor, call malloc nor new; use HeapAlloc directly.

    switch (dwReason)
    {
    case (DLL_PROCESS_ATTACH):
        DisableThreadLibraryCalls( hInstance );
        TraceLoggingRegister(g_hD3D11On12TraceLoggingProvider);
        D3D12TranslationLayer::SetTraceloggingProvider(g_hD3D11On12TraceLoggingProvider);
        break;

    case (DLL_PROCESS_DETACH): // DLL_PROCESS_DETACH will be sent even if DLL_PROCESS_ATTACH fails
        TraceLoggingUnregister(g_hD3D11On12TraceLoggingProvider);
        break;
    }

    return TRUE;
}