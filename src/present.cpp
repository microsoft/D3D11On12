// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    template <typename T, size_t Size> struct CircularArray
    {
        T m_Array[Size];
        struct iterator
        {
            using difference_type = ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::random_access_iterator_tag;
            T* m_Begin;
            T* m_Current;
            iterator(T* Begin, T* Current) : m_Begin(Begin), m_Current(Current) { }
            iterator increment(ptrdiff_t distance) const
            {
                ptrdiff_t totalDistance = (distance + std::distance(m_Begin, m_Current)) % Size;
                totalDistance = totalDistance >= 0 ? totalDistance : totalDistance + Size;
                return iterator(m_Begin, m_Begin + totalDistance);
            }
            iterator& operator++() { *this = increment(1); return *this; }
            iterator operator++(int) { iterator ret = *this; *this = increment(1); return ret; }
            iterator& operator--() { *this = increment(-1); return *this; }
            iterator operator--(int) { iterator ret = *this; *this = increment(-1); return ret; }
            iterator operator+(ptrdiff_t v) { return increment(v); }
            iterator& operator+=(ptrdiff_t v) { *this = increment(v); return *this; }
            iterator operator-(ptrdiff_t v) { return increment(-v); }
            iterator& operator-=(ptrdiff_t v) { *this = increment(-v); return *this; }
            bool operator==(iterator const& o) const { return o.m_Begin == m_Begin && o.m_Current == m_Current; }
            bool operator!=(iterator const& o) const { return !(o == *this); }
            reference operator*() { return *m_Current; }
            pointer operator->() { return m_Current; }
            ptrdiff_t operator-(iterator const& o) const
            {
                assert(o.m_Begin == m_Begin);
                ptrdiff_t rawDistance = std::distance(o.m_Current, m_Current);
                return rawDistance >= 0 ? rawDistance : rawDistance + Size;
            }
        };

        iterator begin() { return iterator(m_Array, m_Array); }
        T& operator[](size_t index) { return *(begin() + index); }
    };

    class SwapChainManager
    {
    public:
        SwapChainManager(D3D12TranslationLayer::ImmediateContext& ImmCtx)
            : m_ImmCtx(ImmCtx)
        {
        }
        ~SwapChainManager()
        {
            m_ImmCtx.WaitForCompletion(D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK);
        }
        IDXGISwapChain3* GetSwapChainForWindow(HWND hwnd, Resource& presentingResource)
        {
            auto& spSwapChain = m_SwapChains[hwnd];
            auto pResourceDesc = presentingResource.ImmediateResource()->AppDesc();
            DXGI_SWAP_CHAIN_DESC Desc = {};
            if (spSwapChain)
            {
                spSwapChain->GetDesc(&Desc);
                if (Desc.BufferDesc.Format != pResourceDesc->Format() ||
                    Desc.BufferDesc.Width != pResourceDesc->Width() ||
                    Desc.BufferDesc.Height != pResourceDesc->Height())
                {
                    m_ImmCtx.WaitForCompletion(D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK);
                    ThrowFailure(spSwapChain->ResizeBuffers(BufferCount,
                        pResourceDesc->Width(),
                        pResourceDesc->Height(),
                        pResourceDesc->Format(),
                        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));
                }
            }
            else
            {
                Desc.BufferCount = BufferCount;
                Desc.BufferDesc.Format = pResourceDesc->Format();
                Desc.BufferDesc.Width = pResourceDesc->Width();
                Desc.BufferDesc.Height = pResourceDesc->Height();
                Desc.OutputWindow = hwnd;
                Desc.Windowed = 1;
                Desc.SampleDesc.Count = 1;
                Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
                unique_comptr<IDXGIFactory> spFactory;
                ThrowFailure(CreateDXGIFactory2(0, IID_PPV_ARGS(&spFactory)));
                unique_comptr<IDXGISwapChain> spBaseSwapChain;
                ThrowFailure(spFactory->CreateSwapChain(
                    m_ImmCtx.GetCommandQueue(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS),
                    &Desc, &spBaseSwapChain));
                ThrowFailure(spBaseSwapChain->QueryInterface(&spSwapChain));
            }
            return spSwapChain.get();
        }
        void SetMaximumFrameLatency(UINT MaxFrameLatency)
        {
            m_MaximumFrameLatency = MaxFrameLatency;
        }
        bool IsMaximumFrameLatencyReached()
        {
            UINT64 CompletedFenceValue = m_ImmCtx.GetCompletedFenceValue(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS);
            while (m_PresentFenceValuesBegin != m_PresentFenceValuesEnd &&
                *m_PresentFenceValuesBegin <= CompletedFenceValue)
            {
                ++m_PresentFenceValuesBegin;
            }
            return std::distance(m_PresentFenceValuesBegin, m_PresentFenceValuesEnd) >= (ptrdiff_t)m_MaximumFrameLatency;
        }
        void WaitForMaximumFrameLatency()
        {
            // Looping, because max frame latency can be dropped, and we may
            // need to wait for multiple presents to complete here.
            while (IsMaximumFrameLatencyReached())
            {
                m_ImmCtx.WaitForFenceValue(
                    D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS,
                    *m_PresentFenceValuesBegin);
            }
        }

    private:
        static constexpr UINT BufferCount = 2;
        D3D12TranslationLayer::ImmediateContext& m_ImmCtx;
        std::map<HWND, unique_comptr<IDXGISwapChain3>> m_SwapChains;
        UINT m_MaximumFrameLatency = 3;
        CircularArray<UINT64, 17> m_PresentFenceValues = {};
        // The fence value to wait on
        decltype(m_PresentFenceValues)::iterator m_PresentFenceValuesBegin = m_PresentFenceValues.begin();
        // The fence value to write to
        decltype(m_PresentFenceValues)::iterator m_PresentFenceValuesEnd = m_PresentFenceValuesBegin;
    };

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
            m_SwapChainManager = std::make_shared<SwapChainManager>(GetImmediateContextNoFlush());
        }

        auto pSwapChain = m_SwapChainManager->GetSwapChainForWindow(pKMTPresent->hWindow, *pArgs->pSrc);
        m_SwapChainManager->WaitForMaximumFrameLatency();

        unique_comptr<ID3D12Resource> backBuffer;
        pSwapChain->GetBuffer(pSwapChain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&backBuffer));

        D3D12TranslationLayer::ResourceCreationArgs destArgs = *pArgs->pSrc->ImmediateResource()->Parent();
        destArgs.m_appDesc.m_Samples = 1;
        destArgs.m_appDesc.m_bindFlags = D3D12TranslationLayer::RESOURCE_BIND_RENDER_TARGET;
        destArgs.m_desc12.SampleDesc.Count = 1;
        destArgs.m_desc12.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        auto destResource = D3D12TranslationLayer::Resource::OpenResource(
            &GetImmediateContextNoFlush(),
            destArgs,
            backBuffer.get(),
            D3D12TranslationLayer::DeferredDestructionType::Submission,
            D3D12_RESOURCE_STATE_COMMON);
        D3D12_RESOURCE_STATES OperationState;
        if (pArgs->pSrc->ImmediateResource()->AppDesc()->Samples() > 1)
        {
            GetImmediateContextNoFlush().ResourceResolveSubresource(destResource.get(), 0, pArgs->pSrc->ImmediateResource(), 0, destArgs.m_appDesc.Format());
            OperationState = D3D12_RESOURCE_STATE_RESOLVE_DEST;
        }
        else
        {
            GetImmediateContextNoFlush().ResourceCopy(destResource.get(), pArgs->pSrc->ImmediateResource());
            OperationState = D3D12_RESOURCE_STATE_COPY_DEST;
        }
        D3D12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.get(), OperationState, D3D12_RESOURCE_STATE_COMMON);
        GetImmediateContextNoFlush().GetGraphicsCommandList()->ResourceBarrier(1, &Barrier);
        GetImmediateContextNoFlush().Flush(D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK);

        HRESULT hr = pSwapChain->Present(pKMTPresent->FlipInterval, pKMTPresent->FlipInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0);
        GetImmediateContextNoFlush().GetCommandListManager(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS)->SetNeedSubmitFence();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    STDMETHODIMP_(void) Device::SetMaximumFrameLatency(UINT MaxFrameLatency) noexcept
    {
        UNREFERENCED_PARAMETER(MaxFrameLatency);
        D3D11on12_DDI_ENTRYPOINT_START();
        std::lock_guard lock(m_SwapChainManagerMutex);
        if (!m_SwapChainManager)
        {
            m_SwapChainManager = std::make_shared<SwapChainManager>(GetImmediateContextNoFlush());
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
            m_SwapChainManager = std::make_shared<SwapChainManager>(GetImmediateContextNoFlush());
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
