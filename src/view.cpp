// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{

    SIZE_T APIENTRY Device::CalcPrivateDepthStencilViewSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11DDIARG_CREATEDEPTHSTENCILVIEW* /*pCreateDSV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        const SIZE_T depthStencilViewSize = sizeof(DepthStencilView);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, depthStencilViewSize);
    }

    VOID APIENTRY Device::CreateDepthStencilView(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11DDIARG_CREATEDEPTHSTENCILVIEW* pCreateDSV, D3D10DDI_HDEPTHSTENCILVIEW hDSV, D3D10DDI_HRTDEPTHSTENCILVIEW /*hRTDSV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device &device = *Device::CastFrom(hDevice);
        
        new (hDSV.pDrvPrivate) DepthStencilView(device, *Resource::CastFromAndGetImmediateResource(pCreateDSV->hDrvResource), pCreateDSV);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyDepthStencilView(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILVIEW hDSV)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        DepthStencilView::CastFrom(hDSV)->~View();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    SIZE_T APIENTRY Device::CalcPrivateRenderTargetViewSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATERENDERTARGETVIEW* /*pCreateRTV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T renderTargetViewSize = sizeof(RenderTargetView);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, renderTargetViewSize);
    }

    VOID APIENTRY Device::CreateRenderTargetView(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATERENDERTARGETVIEW* pCreateRTV, D3D10DDI_HRENDERTARGETVIEW hRTV, D3D10DDI_HRTRENDERTARGETVIEW /*hRTRTV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device &device = *Device::CastFrom(hDevice);

        new (hRTV.pDrvPrivate) RenderTargetView(device, *Resource::CastFromAndGetImmediateResource(pCreateRTV->hDrvResource), pCreateRTV);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyRenderTargetView(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRENDERTARGETVIEW hRTV)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        RenderTargetView::CastFrom(hRTV)->~View();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    SIZE_T APIENTRY Device::CalcPrivateShaderResourceViewSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATESHADERRESOURCEVIEW* /*pCreateSRV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T shaderResourceViewSize = sizeof(ShaderResourceView);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, shaderResourceViewSize);
    }

    VOID APIENTRY Device::CreateShaderResourceView(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATESHADERRESOURCEVIEW* pCreateSRV, D3D10DDI_HSHADERRESOURCEVIEW hSRV, D3D10DDI_HRTSHADERRESOURCEVIEW /*hRTSRV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device &device = *Device::CastFrom(hDevice);

        new (hSRV.pDrvPrivate) ShaderResourceView(device, *Resource::CastFromAndGetImmediateResource(pCreateSRV->hDrvResource), pCreateSRV);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyShaderResourceView(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADERRESOURCEVIEW hSRV)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        ShaderResourceView::CastFrom(hSRV)->~View();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    SIZE_T APIENTRY Device::CalcPrivateUnorderedAccessViewSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATEUNORDEREDACCESSVIEW* /*pCreateUAV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        const SIZE_T unorderedAccessViewSize = sizeof(UnorderedAccessView);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, unorderedAccessViewSize);
    }

    VOID APIENTRY Device::CreateUnorderedAccessView(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATEUNORDEREDACCESSVIEW* pCreateUAV, D3D11DDI_HUNORDEREDACCESSVIEW hUAV, D3D11DDI_HRTUNORDEREDACCESSVIEW /*hRTUAV*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device &device = *Device::CastFrom(hDevice);

        new (hUAV.pDrvPrivate) UnorderedAccessView(device, *Resource::CastFromAndGetImmediateResource(pCreateUAV->hDrvResource), pCreateUAV);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyUnorderedAccessView(D3D10DDI_HDEVICE hDevice, D3D11DDI_HUNORDEREDACCESSVIEW hUAV)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        UnorderedAccessView::CastFrom(hUAV)->~View();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::OMSetRenderTargets(D3D10DDI_HDEVICE hDevice, CONST __in_ecount(NumRTVs) D3D10DDI_HRENDERTARGETVIEW* pRTVs, __in_range(0, 8) UINT NumRTVs, UINT RTVsToUnbind, __in_opt D3D10DDI_HDEPTHSTENCILVIEW hDSV, CONST __in_ecount(NumUavs) D3D11DDI_HUNORDEREDACCESSVIEW* pUavs, CONST UINT* pInitialCounts, UINT UAVStartSlot, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumUavs, UINT UAVFirsttoSet, UINT UAVNumberUpdated)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        DXGI_SAMPLE_DESC LocalSampleDesc = { 1, 0 };
        bool bFoundSampleDesc = false;
        D3D12TranslationLayer::RTV *ppRTVs[D3D10_DDI_SIMULTANEOUS_RENDER_TARGET_COUNT];
        RenderTargetView::GatherViewsFromHandles(pRTVs, ppRTVs, NumRTVs);

        UINT NumValidRTVs= 0;
        for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            auto pRTV = (i < NumRTVs) ? ppRTVs[i] : nullptr;
            // RTV formats/samples are baked into the PSO
            DXGI_FORMAT &CurrentFormat = pDevice->m_PSODesc.RTVFormats[i];
            DXGI_FORMAT NewFormat = pRTV ? pRTV->GetDesc12().Format : DXGI_FORMAT_UNKNOWN;
            if (CurrentFormat != NewFormat)
            {
                CurrentFormat = NewFormat;
                pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
            }

            if (pRTV)
            {
                NumValidRTVs = i + 1;
                if(!bFoundSampleDesc)
                {
                    bFoundSampleDesc = true;
                    auto pResource = pRTV->m_pResource;
                    LocalSampleDesc.Count = pResource->AppDesc()->Samples();
                    LocalSampleDesc.Quality = pResource->AppDesc()->Quality();
                }
            }
        }

        if (pDevice->m_PSODesc.NumRenderTargets != NumValidRTVs)
        {
            pDevice->m_PSODesc.NumRenderTargets = NumValidRTVs;
            pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
        }

        D3D12TranslationLayer::DSV *pDSV = DepthStencilView::CastToTranslationView(hDSV);
        // DSV format/samples are baked into the PSO
        DXGI_FORMAT &CurrentFormat = pDevice->m_PSODesc.DSVFormat;
        DXGI_FORMAT NewFormat = pDSV ? pDSV->GetDesc12().Format : DXGI_FORMAT_UNKNOWN;
        if (CurrentFormat != NewFormat)
        {
            CurrentFormat = NewFormat;
            pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
        }
        if (pDSV && !bFoundSampleDesc)
        {
            bFoundSampleDesc = true;
            auto pResource = pDSV->m_pResource;
            LocalSampleDesc.Count = pResource->AppDesc()->Samples();
            LocalSampleDesc.Quality = pResource->AppDesc()->Quality();
        }

        if (!bFoundSampleDesc)
        {
            LocalSampleDesc.Count = 1;
            LocalSampleDesc.Quality = 0;
        }
        if (LocalSampleDesc.Count != pDevice->m_PSODesc.SampleDesc.Count ||
            LocalSampleDesc.Quality != pDevice->m_PSODesc.SampleDesc.Quality)
        {
            pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
            pDevice->m_PSODesc.SampleDesc = LocalSampleDesc;
        }

        std::vector<D3D12TranslationLayer::UAV *> ppUAV(NumUavs);
        for (UINT i = 0; i < NumUavs; i++)
        {
            ppUAV[i] = UnorderedAccessView::CastToTranslationView(pUavs[i]);
        }

        UNREFERENCED_PARAMETER(RTVsToUnbind);
        UNREFERENCED_PARAMETER(UAVFirsttoSet);
        UNREFERENCED_PARAMETER(UAVNumberUpdated);
        pDevice->GetBatchedContext().OMSetRenderTargets(ppRTVs, NumRTVs, pDSV, NumUavs ? ppUAV.data() : nullptr, pInitialCounts, UAVStartSlot, NumUavs);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::OMSetRenderTargets(D3D10DDI_HDEVICE hDevice, CONST __in_ecount(NumRTVs) D3D10DDI_HRENDERTARGETVIEW* pRTVs, __in_range(0, 8) UINT NumRTVs, UINT RTVsToUnbind, __in_opt D3D10DDI_HDEPTHSTENCILVIEW hDSV, CONST __in_ecount(NumUavs) D3D11DDI_HUNORDEREDACCESSVIEW* pUavs, CONST UINT* pInitialCounts, UINT UAVStartSlot, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumUavs, UINT UAVFirsttoSet, UINT UAVNumberUpdated)
    {
        D3D10DDI_HRENDERTARGETVIEW RTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
        D3D11DDI_HUNORDEREDACCESSVIEW UAVs[D3D11_1_UAV_SLOT_COUNT];
        std::transform(pRTVs, pRTVs + NumRTVs, RTVs, [](D3D10DDI_HRENDERTARGETVIEW h) { return GetImmediate(h); });
        std::transform(pUavs, pUavs + NumUavs, UAVs, [](D3D11DDI_HUNORDEREDACCESSVIEW h) { return GetImmediate(h); });
        DeviceBase::OMSetRenderTargets(hDevice, RTVs, NumRTVs, RTVsToUnbind, GetImmediate(hDSV), UAVs, pInitialCounts, UAVStartSlot, NumUavs, UAVFirsttoSet, UAVNumberUpdated);
    }
    
    void APIENTRY DeviceBase::CsSetUnorderedAccessViews(D3D10DDI_HDEVICE hDevice, UINT Start, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumViews, __in_ecount(NumViews) CONST D3D11DDI_HUNORDEREDACCESSVIEW* pUAVs, __in_ecount(NumViews) CONST UINT* pInitialCounts)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        std::vector<D3D12TranslationLayer::UAV *> ppUAV(NumViews);
        for (UINT i = 0; i < NumViews; i++)
        {
            ppUAV[i] = UnorderedAccessView::CastToTranslationView(pUAVs[i]);
        }

        pDevice->GetBatchedContext().CsSetUnorderedAccessViews(Start, NumViews, NumViews ? ppUAV.data() : nullptr , pInitialCounts);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::CsSetUnorderedAccessViews(D3D10DDI_HDEVICE hDevice, UINT Start, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumViews, __in_ecount(NumViews) CONST D3D11DDI_HUNORDEREDACCESSVIEW* pUAVs, __in_ecount(NumViews) CONST UINT* pInitialCounts)
    {
        D3D11DDI_HUNORDEREDACCESSVIEW UAVs[D3D11_1_UAV_SLOT_COUNT];
        std::transform(pUAVs, pUAVs + NumViews, UAVs, [](D3D11DDI_HUNORDEREDACCESSVIEW h) { return GetImmediate(h); });
        DeviceBase::CsSetUnorderedAccessViews(hDevice, Start, NumViews, UAVs, pInitialCounts);
    }

    template <typename TDest, typename TSource>
    void CopyViewDimensions(TDest& Dest, TSource const& Source)
    {
#define COPY_FIELD(Field) __if_exists(TDest::Field) { Dest.Field = Source.Field; }
        COPY_FIELD(FirstElement);
        COPY_FIELD(NumElements);
        COPY_FIELD(ArraySize);
        COPY_FIELD(FirstArraySlice);
        COPY_FIELD(MipLevels);
        COPY_FIELD(MostDetailedMip);
        COPY_FIELD(PlaneSlice);
        COPY_FIELD(First2DArrayFace);
        COPY_FIELD(NumCubes);
        COPY_FIELD(WSize);
        COPY_FIELD(MipSlice);
        __if_exists(TDest::FirstWSlice)
        {
            Dest.FirstWSlice = Source.FirstW;
        }
        __if_exists(TDest::ResourceMinLODClamp)
        {
            Dest.ResourceMinLODClamp = 0.0f;
        }
#undef COPY_FIELD
    }

    template <typename TDest>
    void AddDepthStencilPlaneIndex(TDest& Dest, DXGI_FORMAT viewFormat)
    {
        switch (viewFormat)
        {
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            Dest.PlaneSlice = 0;
            break;
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            Dest.PlaneSlice = 1;
            break;
        }
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC GetTranslationDesc(D3D12TranslationLayer::Resource& resource, D3DWDDM2_0DDIARG_CREATESHADERRESOURCEVIEW const* pDDIDesc)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC Desc12 = {};
        Desc12.Format = pDDIDesc->Format;
        Desc12.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        switch (pDDIDesc->ResourceDimension)
        {
        case D3D10DDIRESOURCE_BUFFER:
        case D3D11DDIRESOURCE_BUFFEREX:
            Desc12.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            CopyViewDimensions(Desc12.Buffer, pDDIDesc->Buffer);
            if (pDDIDesc->ResourceDimension == D3D11DDIRESOURCE_BUFFEREX)
            {
                Desc12.Buffer.Flags = (pDDIDesc->BufferEx.Flags & D3D11_DDI_BUFFEREX_SRV_FLAG_RAW) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
            }
            else
            {
                Desc12.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            }
            if (pDDIDesc->Format == DXGI_FORMAT_UNKNOWN)
            {
                Desc12.Buffer.StructureByteStride = resource.Parent()->m_flags11.StructureByteStride;
            }
            else
            {
                Desc12.Buffer.StructureByteStride = 0;
            }
            break;
        case D3D10DDIRESOURCE_TEXTURE1D:
            if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                CopyViewDimensions(Desc12.Texture1DArray, pDDIDesc->Tex1D);
            }
            else
            {
                Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                CopyViewDimensions(Desc12.Texture1D, pDDIDesc->Tex1D);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURE2D:
            if (resource.AppDesc()->Samples() > 1)
            {
                Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                if (resource.AppDesc()->ArraySize() > 1)
                {
                    Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    CopyViewDimensions(Desc12.Texture2DMSArray, pDDIDesc->Tex2D);
                }
            }
            else if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                CopyViewDimensions(Desc12.Texture2DArray, pDDIDesc->Tex2D);
                AddDepthStencilPlaneIndex(Desc12.Texture2DArray, Desc12.Format);
            }
            else
            {
                Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                CopyViewDimensions(Desc12.Texture2D, pDDIDesc->Tex2D);
                AddDepthStencilPlaneIndex(Desc12.Texture2D, Desc12.Format);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURECUBE:
            if (resource.AppDesc()->ArraySize() > 6)
            {
                Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                CopyViewDimensions(Desc12.TextureCubeArray, pDDIDesc->TexCube);
            }
            else
            {
                Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                CopyViewDimensions(Desc12.TextureCube, pDDIDesc->TexCube);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURE3D:
            Desc12.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            CopyViewDimensions(Desc12.Texture3D, pDDIDesc->Tex3D);
            break;
        }
        return Desc12;
    }

    D3D12_RENDER_TARGET_VIEW_DESC GetTranslationDesc(D3D12TranslationLayer::Resource& resource, D3DWDDM2_0DDIARG_CREATERENDERTARGETVIEW const* pDDIDesc)
    {
        D3D12_RENDER_TARGET_VIEW_DESC Desc12 = {};
        Desc12.Format = pDDIDesc->Format;
        switch (pDDIDesc->ResourceDimension)
        {
        case D3D10DDIRESOURCE_BUFFER:
        case D3D11DDIRESOURCE_BUFFEREX:
            Desc12.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
            CopyViewDimensions(Desc12.Buffer, pDDIDesc->Buffer);
            break;
        case D3D10DDIRESOURCE_TEXTURE1D:
            if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                CopyViewDimensions(Desc12.Texture1DArray, pDDIDesc->Tex1D);
            }
            else
            {
                Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                CopyViewDimensions(Desc12.Texture1D, pDDIDesc->Tex1D);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURE2D:
            if (resource.AppDesc()->Samples() > 1)
            {
                Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                if (resource.AppDesc()->ArraySize() > 1)
                {
                    Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    CopyViewDimensions(Desc12.Texture2DMSArray, pDDIDesc->Tex2D);
                }
            }
            else if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                CopyViewDimensions(Desc12.Texture2DArray, pDDIDesc->Tex2D);
                AddDepthStencilPlaneIndex(Desc12.Texture2DArray, Desc12.Format);
            }
            else
            {
                Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                CopyViewDimensions(Desc12.Texture2D, pDDIDesc->Tex2D);
                AddDepthStencilPlaneIndex(Desc12.Texture2D, Desc12.Format);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURECUBE:
            Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            Desc12.Texture2DArray.ArraySize = pDDIDesc->TexCube.ArraySize;
            Desc12.Texture2DArray.FirstArraySlice = pDDIDesc->TexCube.FirstArraySlice;
            Desc12.Texture2DArray.MipSlice = pDDIDesc->TexCube.MipSlice;
            Desc12.Texture2DArray.PlaneSlice = 0;
            AddDepthStencilPlaneIndex(Desc12.Texture2DArray, Desc12.Format);
            break;
        case D3D10DDIRESOURCE_TEXTURE3D:
            Desc12.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            CopyViewDimensions(Desc12.Texture3D, pDDIDesc->Tex3D);
            break;
        }
        return Desc12;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC GetTranslationDesc(D3D12TranslationLayer::Resource& resource, D3D11DDIARG_CREATEDEPTHSTENCILVIEW const* pDDIDesc)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC Desc12 = {};
        Desc12.Format = pDDIDesc->Format;
        Desc12.Flags = 
            ((pDDIDesc->Flags & D3D11_DDI_CREATE_DSV_READ_ONLY_DEPTH) ? D3D12_DSV_FLAG_READ_ONLY_DEPTH : D3D12_DSV_FLAG_NONE) |
            ((pDDIDesc->Flags & D3D11_DDI_CREATE_DSV_READ_ONLY_STENCIL) ? D3D12_DSV_FLAG_READ_ONLY_STENCIL : D3D12_DSV_FLAG_NONE);
        switch (pDDIDesc->ResourceDimension)
        {
        case D3D10DDIRESOURCE_TEXTURE1D:
            if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                CopyViewDimensions(Desc12.Texture1DArray, pDDIDesc->Tex1D);
            }
            else
            {
                Desc12.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                CopyViewDimensions(Desc12.Texture1D, pDDIDesc->Tex1D);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURE2D:
            if (resource.AppDesc()->Samples() > 1)
            {
                Desc12.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                if (resource.AppDesc()->ArraySize() > 1)
                {
                    Desc12.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    CopyViewDimensions(Desc12.Texture2DMSArray, pDDIDesc->Tex2D);
                }
            }
            else if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                CopyViewDimensions(Desc12.Texture2DArray, pDDIDesc->Tex2D);
            }
            else
            {
                Desc12.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                CopyViewDimensions(Desc12.Texture2D, pDDIDesc->Tex2D);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURECUBE:
            Desc12.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            CopyViewDimensions(Desc12.Texture2DArray, pDDIDesc->TexCube);
            break;
        case D3D10DDIRESOURCE_BUFFER:
        case D3D11DDIRESOURCE_BUFFEREX:
        case D3D10DDIRESOURCE_TEXTURE3D:
            ASSUME(false);
            break;
        }
        return Desc12;
    }

    D3D12TranslationLayer::D3D12_UNORDERED_ACCESS_VIEW_DESC_WRAPPER GetTranslationDesc(D3D12TranslationLayer::Resource& resource, D3DWDDM2_0DDIARG_CREATEUNORDEREDACCESSVIEW const* pDDIDesc)
    {
        D3D12TranslationLayer::D3D12_UNORDERED_ACCESS_VIEW_DESC_WRAPPER DescTranslation = {};
        auto& Desc12 = DescTranslation.m_Desc12;
        Desc12.Format = pDDIDesc->Format;
        switch (pDDIDesc->ResourceDimension)
        {
        case D3D10DDIRESOURCE_BUFFER:
        case D3D11DDIRESOURCE_BUFFEREX:
            Desc12.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            CopyViewDimensions(Desc12.Buffer, pDDIDesc->Buffer);
            DescTranslation.m_D3D11UAVFlags =
                ((pDDIDesc->Buffer.Flags & D3D11_DDI_BUFFER_UAV_FLAG_APPEND) ? D3D11_BUFFER_UAV_FLAG_APPEND : 0) |
                ((pDDIDesc->Buffer.Flags & D3D11_DDI_BUFFER_UAV_FLAG_COUNTER) ? D3D11_BUFFER_UAV_FLAG_COUNTER : 0) |
                ((pDDIDesc->Buffer.Flags & D3D11_DDI_BUFFER_UAV_FLAG_RAW) ? D3D11_BUFFER_UAV_FLAG_RAW : 0);
            Desc12.Buffer.Flags = ((pDDIDesc->Buffer.Flags & D3D11_DDI_BUFFER_UAV_FLAG_RAW) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE);
            Desc12.Buffer.StructureByteStride = resource.Parent()->m_flags11.StructureByteStride;
            break;
        case D3D10DDIRESOURCE_TEXTURE1D:
            if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                CopyViewDimensions(Desc12.Texture1DArray, pDDIDesc->Tex1D);
            }
            else
            {
                Desc12.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                CopyViewDimensions(Desc12.Texture1D, pDDIDesc->Tex1D);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURE2D:
            if (resource.AppDesc()->ArraySize() > 1)
            {
                Desc12.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                CopyViewDimensions(Desc12.Texture2DArray, pDDIDesc->Tex2D);
                AddDepthStencilPlaneIndex(Desc12.Texture2DArray, Desc12.Format);
            }
            else
            {
                Desc12.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                CopyViewDimensions(Desc12.Texture2D, pDDIDesc->Tex2D);
                AddDepthStencilPlaneIndex(Desc12.Texture2D, Desc12.Format);
            }
            break;
        case D3D10DDIRESOURCE_TEXTURE3D:
            Desc12.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            CopyViewDimensions(Desc12.Texture3D, pDDIDesc->Tex3D);
            break;
        case D3D10DDIRESOURCE_TEXTURECUBE:
            ASSUME(false);
            break;
        }
        return DescTranslation;
    }

    D3D12TranslationLayer::VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL GetTranslationDesc(D3D12TranslationLayer::Resource &resource, D3D11_1DDIARG_CREATEVIDEODECODEROUTPUTVIEW const* pDDIDesc)
    {
        D3D12TranslationLayer::VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL DescTranslation = {};
        DescTranslation.Format = resource.AppDesc()->Format();
        DescTranslation.ArraySlice = pDDIDesc->FirstArraySlice;
        return DescTranslation;
    }

    D3D12TranslationLayer::VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL GetTranslationDesc(D3D12TranslationLayer::Resource &resource, D3D11_1DDIARG_CREATEVIDEOPROCESSORINPUTVIEW const* pDDIDesc)
    {
        D3D12TranslationLayer::VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL DescTranslation = {};
        DescTranslation.Format = resource.AppDesc()->Format();
        DescTranslation.ArraySlice = pDDIDesc->FirstArraySlice;
        DescTranslation.MipSlice = pDDIDesc->MipSlice;
        return DescTranslation;
    }

    D3D12TranslationLayer::VIDEO_PROCESSOR_OUTPUT_VIEW_DESC_INTERNAL GetTranslationDesc(D3D12TranslationLayer::Resource &resource, D3D11_1DDIARG_CREATEVIDEOPROCESSOROUTPUTVIEW const* pDDIDesc)
    {
        D3D12TranslationLayer::VIDEO_PROCESSOR_OUTPUT_VIEW_DESC_INTERNAL DescTranslation = {};
        DescTranslation.Format = resource.AppDesc()->Format();
        CopyViewDimensions(DescTranslation, *pDDIDesc);
        return DescTranslation;
    }
}