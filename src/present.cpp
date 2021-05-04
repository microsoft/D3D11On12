// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"
#include "SwapChainHelper.hpp"

namespace D3D11On12
{
    HRESULT APIENTRY Device::Present1(DXGI1_6_1_DDI_ARG_PRESENT* pArgs)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(pArgs->hDevice);

        assert(pArgs->pDirtyRects == nullptr && pArgs->DirtyRects == 0);
        PresentExtensionData ExtensionData{ };
        if (pArgs->SurfacesToPresent)
        {
            ExtensionData.pSrc = Resource::CastFrom(pArgs->phSurfacesToPresent[0].hSurface);
            ExtensionData.pDXGIContext = pArgs->pDXGIContext;
        }
        pDevice->GetBatchedContext().BatchExtension(&pDevice->m_PresentExt, ExtensionData);
        pDevice->GetBatchedContext().SubmitBatch();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void Device::PresentExtension::Dispatch(D3D12TranslationLayer::ImmediateContext&, const void* pData, size_t)
    {
        m_Device->m_pPresentArgs = reinterpret_cast<PresentExtensionData const*>(pData);
        DXGIDDICB_PRESENT CBArgs = {};
        CBArgs.pDXGIContext = m_Device->m_pPresentArgs->pDXGIContext;
        ThrowFailure(m_Device->m_pDXGICallbacks->pfnPresentCb(m_Device->m_hRTDevice.handle, &CBArgs));
    }

    HRESULT APIENTRY Device::Blt(DXGI_DDI_ARG_BLT* pArgs)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT& SrcSubresourceDesc =
            Resource::CastFromAndGetImmediateResource(pArgs->hSrcResource)->GetSubresourcePlacement(pArgs->SrcSubresource);

        DXGI_DDI_ARG_BLT1 Args1 =
        {
            pArgs->hDevice,
            pArgs->hDstResource,
            pArgs->DstSubresource,
            pArgs->DstLeft,
            pArgs->DstTop,
            pArgs->DstRight,
            pArgs->DstBottom,
            pArgs->hSrcResource,
            pArgs->SrcSubresource,
            0, //SrcRect.left,
            0, //SrcRect.top,
            SrcSubresourceDesc.Footprint.Width, //SrcRect.right,
            SrcSubresourceDesc.Footprint.Height, //SrcRect.bottom,
            pArgs->Flags,
            pArgs->Rotate,
        };

        HRESULT hr = BltImpl(&Args1);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY Device::Blt1(DXGI_DDI_ARG_BLT1* pArgs)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        HRESULT hr = BltImpl(pArgs);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY Device::RotateResourceIdentities(DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* pArgs)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(pArgs->hDevice);

        // TODO: Consider making RRI an extension so we can emplace in a batch directly from the DDI args
        // without having to allocate an intermediate array.
        std::vector<D3D12TranslationLayer::Resource*> pUnderlying(pArgs->Resources);

        for (size_t i = 0; i < pArgs->Resources; i++)
        {
            pUnderlying[i] = Resource::CastFromAndGetImmediateResource(pArgs->pResources[i]);
        }

        pDevice->GetBatchedContext().RotateResourceIdentities(pUnderlying.data(), pArgs->Resources);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT APIENTRY Device::ResolveSharedResource(DXGI_DDI_ARG_RESOLVESHAREDRESOURCE* pArgs)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(pArgs->hDevice);

        HRESULT hr = pDevice->FlushBatchAndGetImmediateContext().ResolveSharedResource(Resource::CastFromAndGetImmediateResource(pArgs->hResource));
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY Device::TrimResidencySet(DXGI_DDI_ARG_TRIMRESIDENCYSET* /*pArgs*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT APIENTRY Device::SetResourcePriority(DXGI_DDI_ARG_SETRESOURCEPRIORITY* /*pArgs*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT APIENTRY Device::CheckMultiplaneOverlayColorSpaceSupport(DXGI_DDI_ARG_CHECKMULTIPLANEOVERLAYCOLORSPACESUPPORT* pArgs)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        pArgs->Supported = FALSE;

        D3D12TranslationLayer::ImmediateContext& Context = Device::CastFrom(pArgs->hDevice)->GetImmediateContextNoFlush();

        if (Context.IsXbox())
        {
            switch (pArgs->Format)
            {
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8X8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UNORM:
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                switch (pArgs->ColorSpace)
                {
                case D3DDDI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
                case D3DDDI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709:
                case D3DDDI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020:
                case D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709:
                case D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020:
                case D3DDDI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
                case D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020:
                case D3DDDI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020:
                case D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020:
                case D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020:
                case D3DDDI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020:
                    pArgs->Supported = TRUE;
                    break;
                }
                break;
            }
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT APIENTRY Device::PresentMultiplaneOverlay1(DXGI1_6_1_DDI_ARG_PRESENTMULTIPLANEOVERLAY* /*pArgs*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    HRESULT Device::BltImpl(DXGI_DDI_ARG_BLT1* pArgs)
    {
        D3D12TranslationLayer::ImmediateContext& Context = Device::CastFrom(pArgs->hDevice)->FlushBatchAndGetImmediateContext();

        // TODO: Rotation?
        assert(pArgs->Rotate == DXGI_DDI_MODE_ROTATION_IDENTITY);
        RECT DstRect = { (LONG)pArgs->DstLeft, (LONG)pArgs->DstTop, (LONG)pArgs->DstRight, (LONG)pArgs->DstBottom };
        RECT SrcRect = { (LONG)pArgs->SrcLeft, (LONG)pArgs->SrcTop, (LONG)pArgs->SrcRight, (LONG)pArgs->SrcBottom };
        Context.m_BlitHelper.Blit(
            Resource::CastFromAndGetImmediateResource(pArgs->hSrcResource),
            pArgs->SrcSubresource,
            SrcRect,
            Resource::CastFromAndGetImmediateResource(pArgs->hDstResource),
            pArgs->DstSubresource,
            DstRect,
            true,
            false);

        return S_OK;
    }

    STDMETHODIMP Device::Present(D3DKMT_PRESENT* pKMTPresent) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pArgs = m_pPresentArgs;
        m_pPresentArgs = nullptr;
        
        std::lock_guard lock(m_SwapChainManagerMutex);
        if (!m_SwapChainManager)
        {
            m_SwapChainManager = std::make_shared<D3D12TranslationLayer::SwapChainManager>(GetImmediateContextNoFlush());
        }

        auto pSwapChain = m_SwapChainManager->GetSwapChainForWindow(pKMTPresent->hWindow, *pArgs->pSrc->ImmediateResource());
        auto swapChainHelper = D3D12TranslationLayer::SwapChainHelper( pSwapChain );
        m_SwapChainManager->WaitForMaximumFrameLatency();

        HRESULT hr = swapChainHelper.StandardPresent( GetImmediateContextNoFlush(), pKMTPresent, *pArgs->pSrc->ImmediateResource() );
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    STDMETHODIMP_(void) Device::SetMaximumFrameLatency(UINT MaxFrameLatency) noexcept
    {
        UNREFERENCED_PARAMETER(MaxFrameLatency);
        D3D11on12_DDI_ENTRYPOINT_START();
        std::lock_guard lock(m_SwapChainManagerMutex);
        if (!m_SwapChainManager)
        {
            m_SwapChainManager = std::make_shared<D3D12TranslationLayer::SwapChainManager>(GetImmediateContextNoFlush());
        }
        m_SwapChainManager->SetMaximumFrameLatency(MaxFrameLatency);
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
    }

    STDMETHODIMP_(bool) Device::IsMaximumFrameLatencyReached() noexcept
    {
        bool ret = false;
        D3D11on12_DDI_ENTRYPOINT_START();
        std::lock_guard lock(m_SwapChainManagerMutex);
        if (!m_SwapChainManager)
        {
            m_SwapChainManager = std::make_shared<D3D12TranslationLayer::SwapChainManager>(GetImmediateContextNoFlush());
        }
        ret = m_SwapChainManager->IsMaximumFrameLatencyReached();
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
        return ret;
    }

    void Device::AcquireResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE, HANDLE hSyncToken) noexcept
    {
        auto pThis = CastFrom(hDevice);
        pThis->GetBatchedContext().BatchExtension(&pThis->m_AcquireResourceExt, hSyncToken);
    }

    void Device::ReleaseResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE, HANDLE hSyncToken) noexcept
    {
        auto pThis = CastFrom(hDevice);
        pThis->GetBatchedContext().BatchExtension(&pThis->m_ReleaseResourceExt, hSyncToken);
        pThis->GetBatchedContext().SubmitBatch();
    }

    void Device::QueryScanoutCaps(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3DDDI_VIDEO_PRESENT_SOURCE_ID, UINT, D3DWDDM2_6DDI_SCANOUT_FLAGS* pFlags) noexcept
    {
        // Don't do front buffer rendering
        *pFlags = D3DWDDM2_6DDI_SCANOUT_FLAGS(D3DWDDM2_6DDI_SCANOUT_FLAG_TRANSFORMATION_REQUIRED | D3DWDDM2_6DDI_SCANOUT_FLAG_UNPREDICTABLE_TIMING);
    }

    void Device::PrepareScanoutTransformation(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3DDDI_VIDEO_PRESENT_SOURCE_ID, UINT, RECT*) noexcept
    {
        // Do nothing
    }

    STDMETHODIMP_(void) Device::SharingContractPresent(_In_ ID3D11On12DDIResource* pResource) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto& Context = FlushBatchAndGetImmediateContext();
        Context.SharingContractPresent(static_cast<Resource*>(pResource)->ImmediateResource());
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
    }
}
