// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    //==================================================================================================================================
    // Adapter
    // Represents a UMD adapter, responsible for CreateDevice and caps management
    //==================================================================================================================================
    class Adapter
    {
    public:
        Adapter(D3D10DDIARG_OPENADAPTER *pOpenArgs, SOpenAdapterArgs& Args) noexcept(false);

        D3D10DDI_HADAPTER GetHandle() noexcept { return MAKE_D3D10DDI_HADAPTER(this); }
        static inline Adapter* CastFrom(D3D10DDI_HADAPTER hAdapter) noexcept
        {
            return reinterpret_cast<Adapter*>(hAdapter.pDrvPrivate);
        }

    public: // Adapter DDI funcs
        static SIZE_T APIENTRY CalcPrivateDeviceSize(_In_ D3D10DDI_HADAPTER hAdapter, _In_ const D3D10DDIARG_CALCPRIVATEDEVICESIZE*);
        static HRESULT APIENTRY CreateDevice(_In_ D3D10DDI_HADAPTER hAdapter, _Inout_ D3D10DDIARG_CREATEDEVICE* pArgs);
        static HRESULT APIENTRY CloseAdapter(_In_ D3D10DDI_HADAPTER hAdapter);
        static HRESULT APIENTRY GetSupportedVersions(_In_ D3D10DDI_HADAPTER hAdapter, _Inout_ UINT32 *pNumVersions, _Out_opt_ UINT64 *pSupportedVersions);
        static HRESULT APIENTRY GetCaps(_In_ D3D10DDI_HADAPTER hAdapter, _In_ const D3D10_2DDIARG_GETCAPS *pData);

    public:
        CComPtr<ID3D12Device1> m_pUnderlyingDevice;
        ID3D12CommandQueue* m_p3DCommandQueue;
        CComPtr<ID3D12CompatibilityDevice> m_pCompatDevice;

        UINT m_NodeIndex;
        D3D12_FEATURE_DATA_ARCHITECTURE1 m_Architecture;
        D3D12_FEATURE_DATA_D3D12_OPTIONS m_Caps;
        D3D12_FEATURE_DATA_SHADER_MODEL m_ShaderModelCaps;
        bool m_bComputeOnly;

        PrivateCallbacks m_Callbacks;
        PrivateCallbacks2 m_Callbacks2;
        const bool m_bAPIDisablesGPUTimeout;
        const bool m_bSupportDisplayableTextures;
        const bool m_bSupportDeferredContexts;
        const bool m_bSupportsNewPresentPath;
    };

};