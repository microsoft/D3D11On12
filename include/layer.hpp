// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    static void PrintDebugMessage(std::string message)
    {
        OutputDebugStringA(message.c_str());
        OutputDebugStringA("\n");
    }


#define OPEN_TRYCATCH \
    HRESULT EntryPointHr = S_OK; \
    try \
    {

#define CLOSE_TRYCATCH_AND_STORE_HRESULT(hr) \
        EntryPointHr = hr; \
    } \
    catch (_com_error& hrEx) \
    { \
        EntryPointHr = hrEx.Error(); \
    } \
    catch (std::bad_alloc&) \
    { \
        EntryPointHr = E_OUTOFMEMORY; \
    } \

#if DBG
#define D3D11on12_DDI_ENTRYPOINT_START() \
    if (false)\
    {\
        PrintDebugMessage("D3D11on12 DDI Entrypoint: " + std::string(__FUNCTION__)); \
    }\
    OPEN_TRYCATCH
#else
#define D3D11on12_DDI_ENTRYPOINT_START() \
    OPEN_TRYCATCH
#endif

#define D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr) \
    CLOSE_TRYCATCH_AND_STORE_HRESULT(hr) \
    return EntryPointHr; \

#define D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr) \
    CLOSE_TRYCATCH_AND_STORE_HRESULT(hr) \
    if(FAILED(EntryPointHr)) \
    { \
        DeviceBase *pDevice = DeviceBase::CastFrom(hDevice); \
        pDevice->ReportError(EntryPointHr); \
    }

#define D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, hr, value) \
    if(SUCCEEDED(hr)) return value; /* If there have been no exceptions and hr is good, early out */ \
    D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr) \
    return 0; \

#define D3D11on12_DDI_ENTRYPOINT_END() \


#if 0
    void APIENTRY RelocateDeviceFuncs(D3D10DDI_HDEVICE hDevice, __in struct D3DWDDM2_0DDI_DEVICEFUNCS*)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        D3D12TranslationLayer::ImmediateContext& context = pDevice->GetImmediateContext();
    }

#endif
}