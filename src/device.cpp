// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void APIENTRY Device::CheckFormatSupport(D3D10DDI_HDEVICE hDevice, DXGI_FORMAT format, _Out_ UINT* pData)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        D3D12TranslationLayer::ImmediateContext& context = pDevice->GetImmediateContextNoFlush();

        D3D12_FEATURE_DATA_FORMAT_SUPPORT SupportStruct = {};
        SupportStruct.Format = format;
        HRESULT hr = context.CheckFormatSupport(SupportStruct);
        if (FAILED(hr))
        {
            assert(hr == E_FAIL);
            *pData = D3D10_DDI_FORMAT_SUPPORT_NOT_SUPPORTED;
            return;
        }

        auto APISupport1 = SupportStruct.Support1;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) *pData |= D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) *pData |= D3D10_DDI_FORMAT_SUPPORT_RENDERTARGET;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_BLENDABLE) *pData |= D3D10_DDI_FORMAT_SUPPORT_BLENDABLE;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET) *pData |= D3D10_DDI_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD) *pData |= D3D10_DDI_FORMAT_SUPPORT_MULTISAMPLE_LOAD;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_DECODER_OUTPUT) *pData |= D3D11_1DDI_FORMAT_SUPPORT_DECODER_OUTPUT;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_VIDEO_PROCESSOR_OUTPUT) *pData |= D3D11_1DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_VIDEO_PROCESSOR_INPUT) *pData |= D3D11_1DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER) *pData |= D3D11_1DDI_FORMAT_SUPPORT_VERTEX_BUFFER;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_BUFFER) *pData |= D3D11_1DDI_FORMAT_SUPPORT_BUFFER;
        constexpr D3D12_FORMAT_SUPPORT1 D3D12_FORMAT_SUPPORT1_CAPTURE = (D3D12_FORMAT_SUPPORT1)0x80000000;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_CAPTURE) *pData |= D3D11_1DDI_FORMAT_SUPPORT_CAPTURE;
        if (APISupport1 & D3D12_FORMAT_SUPPORT1_SHADER_GATHER) *pData |= D3D11_1DDI_FORMAT_SUPPORT_SHADER_GATHER;

        auto APISupport2 = SupportStruct.Support2;
        if (APISupport2 & D3D12_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP) *pData |= D3D11_1DDI_FORMAT_SUPPORT_OUTPUT_MERGER_LOGIC_OP;
        if (APISupport2 & D3D12_FORMAT_SUPPORT2_TILED) *pData |= D3DWDDM1_3DDI_FORMAT_SUPPORT_TILED;
        if (APISupport2 & D3D12_FORMAT_SUPPORT2_MULTIPLANE_OVERLAY) *pData |= D3D11_1DDI_FORMAT_SUPPORT_MULTIPLANE_OVERLAY;
        if (APISupport2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) *pData |= D3D11_1DDI_FORMAT_SUPPORT_UAV_WRITES;
        if (APISupport2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) *pData |= D3DWDDM2_0DDI_FORMAT_SUPPORT_UAV_READS;

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY Device::CheckMultisampleQualityLevels(D3D10DDI_HDEVICE hDevice, DXGI_FORMAT format, UINT sampleCount, UINT flags, _Out_ UINT* pData)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        D3D12TranslationLayer::ImmediateContext& context = pDevice->GetImmediateContextNoFlush();

        assert((flags & ~D3DWDDM1_3DDI_CHECK_MULTISAMPLE_QUALITY_LEVELS_TILED_RESOURCE) == 0);

        D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags12 = (flags & D3DWDDM1_3DDI_CHECK_MULTISAMPLE_QUALITY_LEVELS_TILED_RESOURCE) ?
            D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_TILED_RESOURCE : D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

        if (pDevice->GetAdapter()->m_bComputeOnly)
        {
            *pData = 0;
        }
        else
        {
            context.CheckMultisampleQualityLevels(format, sampleCount, flags12, pData);
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY Device::CheckCounterInfo(D3D10DDI_HDEVICE hDevice, _Out_ D3D10DDI_COUNTER_INFO* pCounterInfo)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        pCounterInfo->LastDeviceDependentCounter = static_cast<D3D10DDI_QUERY>(0);
        pCounterInfo->NumDetectableParallelUnits = 1;
        pCounterInfo->NumSimultaneousCounters = 0;

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY Device::CheckCounter(D3D10DDI_HDEVICE hDevice, D3D10DDI_QUERY Query, _Out_ D3D10DDI_COUNTER_TYPE * /*pCounterType*/,
        _Out_ UINT * /*pActiveCounters*/, _Out_writes_opt_(*pNameLength) LPSTR pName, _Inout_opt_  UINT *pNameLength, _Out_writes_opt_(*pUnitsLength) LPSTR pUnits,
        _Inout_opt_  UINT *pUnitsLength, _Out_writes_opt_(*pDescriptionLength) LPSTR pDescription, _Inout_opt_  UINT *pDescriptionLength)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        UNREFERENCED_PARAMETER(Query);
        assert(Query >= D3D10DDI_COUNTER_DEVICE_DEPENDENT_0);
        // Null terminate output strings for SAL
        if (pName && pNameLength && *pNameLength > 0)
        {
            pName[0] = '\0';
            *pNameLength = 1;
        }
        if (pUnits && pUnitsLength && *pUnitsLength > 0)
        {
            pUnits[0] = '\0';
            *pUnitsLength = 1;
        }
        if (pDescription && pDescriptionLength && *pDescriptionLength > 0)
        {
            pDescription[0] = '\0';
            *pDescriptionLength = 1;
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, E_NOTIMPL);
    }

    void APIENTRY Device::DestroyDevice(D3D10DDI_HDEVICE hDevice)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        pDevice->~Device();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::DestroyDevice(D3D10DDI_HDEVICE hDevice)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        DeviceDeferred *pDevice = DeviceDeferred::CastFrom(hDevice);

        pDevice->~DeviceDeferred();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::SetHardwareProtection(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, INT value)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().SetHardwareProtection(Resource::CastFromAndGetImmediateResource(hResource), value);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::SetHardwareProtectionState(D3D10DDI_HDEVICE hDevice, BOOL enabled)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().SetHardwareProtectionState(enabled);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT Device::EnsureVideoDevice()
    {
        if (!m_pVideoDevice)
        {
            m_pVideoDevice.reset(new (m_pVideoDeviceSpace)VideoDevice(*this));
        }

        return S_OK;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    VideoDevice* Device::GetVideoDevice()
    {
        return m_pVideoDevice.get();
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY Device::RetrieveSubObject(
        D3D10DDI_HDEVICE hDevice,
        UINT32 SubDeviceID,
        SIZE_T ParamSize,
        void *pParams,
        SIZE_T OutputParamSize,
        void *pOutputParamsBuffer
        )
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        HRESULT hr = S_OK;

        if (SubDeviceID != D3DWDDM2_4DDI_VIDEO_FUNCTIONS ||
            ParamSize != sizeof(D3DWDDM2_4DDI_VIDEO_INPUT) ||
            pParams == nullptr ||
            OutputParamSize != sizeof(D3DWDDM2_4DDI_VIDEO_OUTPUT) ||
            pOutputParamsBuffer == nullptr)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            D3DWDDM2_4DDI_VIDEO_INPUT *pInput = static_cast<D3DWDDM2_4DDI_VIDEO_INPUT *>(pParams);
            hr = pDevice->EnsureVideoDevice();
            if (SUCCEEDED(hr))
            {
                VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
                pVideoDevice->FillVideoDDIFunctions(pInput->pWDDM2_4VideoDeviceFuncs);
            }
        }
        
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    D3D12TranslationLayer::TranslationLayerCallbacks Device::GetTranslationLayerCallbacks()
    {
        D3D12TranslationLayer::TranslationLayerCallbacks callbacks;
        callbacks.m_pfnPostSubmit = [this]() { m_BatchedContext.PostSubmit(); };
        return callbacks;
    }

    D3D12TranslationLayer::BatchedContext::Callbacks Device::GetBatchedContextCallbacks()
    {
        D3D12TranslationLayer::BatchedContext::Callbacks callbacks;
        callbacks.PostSubmitCallback = [this]() { PostSubmit(); };
        callbacks.ThreadErrorCallback = [this](HRESULT hr) { ReportError(hr); };
        return callbacks;
    }

    D3D12TranslationLayer::BatchedContext::Callbacks DeviceDeferred::GetBatchedContextCallbacks()
    {
        D3D12TranslationLayer::BatchedContext::Callbacks callbacks;
        // Deferred contexts never submit - so intentionally not filling PostSubmitCallback
        callbacks.ThreadErrorCallback = [this](HRESULT hr) { ReportError(hr); };
        return callbacks;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    UINT64 GetDebugFlags()
    {
#if DBG
        UINT64 debugFlags = 0;
        GetCompatValue("11On12Debug", &debugFlags);
        return debugFlags;
#else
        return 0ull;
#endif
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    inline bool BatchedContextUseWorkerThread(UINT CreateDeviceFlags)
    {
        return !(CreateDeviceFlags & D3D10DDI_CREATEDEVICE_FLAG_DISABLE_EXTRA_THREAD_CREATION);
    }

    inline bool BatchedContextCreatesAndDestroysAreMultiThreaded(UINT CreateDeviceFlags)
    {
        return (CreateDeviceFlags & D3D11DDI_CREATEDEVICE_FLAG_SINGLETHREADED) == 0;
    }

    inline bool ImmediateContextCreatesAndDestroysAreMultiThreaded(UINT CreateDeviceFlags)
    {
        return BatchedContextCreatesAndDestroysAreMultiThreaded(CreateDeviceFlags) || BatchedContextUseWorkerThread(CreateDeviceFlags);
    }

    inline bool IsXboxCreateFlags(UINT CreateDeviceFlags)
    {
        return (CreateDeviceFlags & D3D11DDI_CREATEDEVICE_FLAG_IS_XBOX) == D3D11DDI_CREATEDEVICE_FLAG_IS_XBOX;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    inline D3D12TranslationLayer::ImmediateContext::CreationArgs GetImmCtxArgs(Adapter* pAdapter, UINT CreateDeviceFlags)
    {
        UINT64 roundTripPSOs = 0;

        D3D12TranslationLayer::ImmediateContext::CreationArgs args = {};
        args.RequiresBufferOutOfBoundsHandling = !pAdapter->m_bComputeOnly;
        args.CreatesAndDestroysAreMultithreaded = ImmediateContextCreatesAndDestroysAreMultiThreaded(CreateDeviceFlags);
        args.RenamingIsMultithreaded = BatchedContextUseWorkerThread(CreateDeviceFlags);
        args.UseThreadpoolForPSOCreates = true;
        args.UseRoundTripPSOs = GetCompatValue("RoundTripPSOs", &roundTripPSOs) && roundTripPSOs;
        args.UseResidencyManagement = true;
        args.DisableGPUTimeout = pAdapter->m_bAPIDisablesGPUTimeout;
        args.IsXbox = IsXboxCreateFlags(CreateDeviceFlags);
#ifdef __D3D11On12CreatorID_INTERFACE_DEFINED__
        args.CreatorID = __uuidof(D3D11On12CreatorID);
#endif
        return args;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    inline D3D12TranslationLayer::SharedResourceHelpers::CreationFlags GetSharedResourceFlags(Adapter* pAdapter)
    {
        D3D12TranslationLayer::SharedResourceHelpers::CreationFlags flags = {};
        flags.SupportDisplayableTextures = pAdapter->m_bSupportDisplayableTextures;
        return flags;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    inline D3D12TranslationLayer::BatchedContext::CreationArgs GetBatchedCtxArgs(UINT CreateDeviceFlags, D3D10DDIARG_CREATEDEVICE const&)
    {
        D3D12TranslationLayer::BatchedContext::CreationArgs args = {};
        args.CreatesAndDestroysAreMultithreaded = BatchedContextCreatesAndDestroysAreMultiThreaded(CreateDeviceFlags);
        args.SubmitBatchesToWorkerThread = BatchedContextUseWorkerThread(CreateDeviceFlags);
        return args;
    }
    inline D3D12TranslationLayer::BatchedContext::CreationArgs GetBatchedCtxArgs(UINT, DeferredCtxArgs const& Args)
    {
        D3D12TranslationLayer::BatchedContext::CreationArgs args = {};
        args.CreatesAndDestroysAreMultithreaded = false;
        args.SubmitBatchesToWorkerThread = false;
        args.pParentContext = &Args.pBaseDevice->GetBatchedContext();
        return args;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    inline D3D12TranslationLayer::ResourceAllocationContext GetResourceAllocationContextImpl(UINT CreateDeviceFlags)
    {
        if (ImmediateContextCreatesAndDestroysAreMultiThreaded(CreateDeviceFlags))
        {
            return D3D12TranslationLayer::ResourceAllocationContext::FreeThread;
        }
        return D3D12TranslationLayer::ResourceAllocationContext::ImmediateContextThreadLongLived;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    DeviceBase::DDITableLatest* GetDeviceFuncsFromCreateArgs(D3D10DDIARG_CREATEDEVICE const* pArgs)
    {
        return pArgs->pWDDM2_6DeviceFuncs;
    }
    DeviceBase::DDITableLatest* GetDeviceFuncsFromCreateArgs(D3D11DDIARG_CREATEDEFERREDCONTEXT const* pArgs)
    {
        return pArgs->pWDDM2_6ContextFuncs;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    D3D12TranslationLayer::BatchedContext& GetPrimaryBatchedContext(D3D10DDIARG_CREATEDEVICE const&, D3D12TranslationLayer::BatchedContext& BatchedCtx)
    {
        return BatchedCtx;
    }
    D3D12TranslationLayer::BatchedContext& GetPrimaryBatchedContext(DeferredCtxArgs const& Args, D3D12TranslationLayer::BatchedContext&)
    {
        return Args.pBaseDevice->GetBatchedContext();
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    template <typename TArgs>
    DeviceBase::DeviceBase(Adapter* pAdapter,
                           D3D12TranslationLayer::ImmediateContext& ImmCtx,
                           D3D12TranslationLayer::BatchedContext::Callbacks batchedCtxCallbacks,
                           D3D12TranslationLayer::COptLockedContainer<GraphicsPipelineStateCache>& GraphicsPSOCache,
                           D3D12TranslationLayer::COptLockedContainer<ComputePipelineStateCache>& ComputePSOCache,
                           TArgs const& Args)
        : m_pDDITable(GetDeviceFuncsFromCreateArgs(&Args))
        , m_hRTCoreLayer(Args.hRTCoreLayer)
        , m_pCallbacks(Args.pWDDM2_6UMCallbacks)
        , m_pAdapter(pAdapter)
        , m_GraphicsPSOCache(GraphicsPSOCache)
        , m_ComputePSOCache(ComputePSOCache)
        , m_BatchedContext(ImmCtx, GetBatchedCtxArgs(Args.Flags, Args), std::move(batchedCtxCallbacks))
        , m_PrimaryBatchedContext(GetPrimaryBatchedContext(Args, m_BatchedContext))
    {
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    template <typename TDevice>
    void DeviceBase::FillContextDDIs()
    {
        m_pDDITable->pfnVsSetShader = TDevice::SetShader<D3D12TranslationLayer::e_VS>;
        m_pDDITable->pfnPsSetShader = TDevice::SetShader<D3D12TranslationLayer::e_PS>;
        m_pDDITable->pfnGsSetShader = TDevice::SetShader<D3D12TranslationLayer::e_GS>;
        m_pDDITable->pfnHsSetShader = TDevice::SetShader<D3D12TranslationLayer::e_HS>;
        m_pDDITable->pfnDsSetShader = TDevice::SetShader<D3D12TranslationLayer::e_DS>;
        m_pDDITable->pfnCsSetShader = TDevice::SetShader<D3D12TranslationLayer::e_CS>;

        m_pDDITable->pfnVsSetShaderWithIfaces = TDevice::SetShaderWithIfaces<D3D12TranslationLayer::e_VS>;
        m_pDDITable->pfnPsSetShaderWithIfaces = TDevice::SetShaderWithIfaces<D3D12TranslationLayer::e_PS>;
        m_pDDITable->pfnGsSetShaderWithIfaces = TDevice::SetShaderWithIfaces<D3D12TranslationLayer::e_GS>;
        m_pDDITable->pfnHsSetShaderWithIfaces = TDevice::SetShaderWithIfaces<D3D12TranslationLayer::e_HS>;
        m_pDDITable->pfnDsSetShaderWithIfaces = TDevice::SetShaderWithIfaces<D3D12TranslationLayer::e_DS>;
        m_pDDITable->pfnCsSetShaderWithIfaces = TDevice::SetShaderWithIfaces<D3D12TranslationLayer::e_CS>;

        m_pDDITable->pfnVsSetShaderResources = TDevice::SetShaderResources<D3D12TranslationLayer::e_VS>;
        m_pDDITable->pfnPsSetShaderResources = TDevice::SetShaderResources<D3D12TranslationLayer::e_PS>;
        m_pDDITable->pfnGsSetShaderResources = TDevice::SetShaderResources<D3D12TranslationLayer::e_GS>;
        m_pDDITable->pfnHsSetShaderResources = TDevice::SetShaderResources<D3D12TranslationLayer::e_HS>;
        m_pDDITable->pfnDsSetShaderResources = TDevice::SetShaderResources<D3D12TranslationLayer::e_DS>;
        m_pDDITable->pfnCsSetShaderResources = TDevice::SetShaderResources<D3D12TranslationLayer::e_CS>;

        m_pDDITable->pfnVsSetConstantBuffers = TDevice::SetConstantBuffers<D3D12TranslationLayer::e_VS>;
        m_pDDITable->pfnPsSetConstantBuffers = TDevice::SetConstantBuffers<D3D12TranslationLayer::e_PS>;
        m_pDDITable->pfnGsSetConstantBuffers = TDevice::SetConstantBuffers<D3D12TranslationLayer::e_GS>;
        m_pDDITable->pfnHsSetConstantBuffers = TDevice::SetConstantBuffers<D3D12TranslationLayer::e_HS>;
        m_pDDITable->pfnDsSetConstantBuffers = TDevice::SetConstantBuffers<D3D12TranslationLayer::e_DS>;
        m_pDDITable->pfnCsSetConstantBuffers = TDevice::SetConstantBuffers<D3D12TranslationLayer::e_CS>;

        m_pDDITable->pfnVsSetSamplers = TDevice::SetSamplers<D3D12TranslationLayer::e_VS>;
        m_pDDITable->pfnPsSetSamplers = TDevice::SetSamplers<D3D12TranslationLayer::e_PS>;
        m_pDDITable->pfnGsSetSamplers = TDevice::SetSamplers<D3D12TranslationLayer::e_GS>;
        m_pDDITable->pfnHsSetSamplers = TDevice::SetSamplers<D3D12TranslationLayer::e_HS>;
        m_pDDITable->pfnDsSetSamplers = TDevice::SetSamplers<D3D12TranslationLayer::e_DS>;
        m_pDDITable->pfnCsSetSamplers = TDevice::SetSamplers<D3D12TranslationLayer::e_CS>;

        m_pDDITable->pfnDraw = TDevice::Draw;
        m_pDDITable->pfnDrawAuto = TDevice::DrawAuto;
        m_pDDITable->pfnDrawIndexed = TDevice::DrawIndexed;
        m_pDDITable->pfnDrawIndexedInstanced = TDevice::DrawIndexedInstanced;
        m_pDDITable->pfnDrawIndexedInstancedIndirect = TDevice::DrawIndexedInstancedIndirect;
        m_pDDITable->pfnDrawInstanced = TDevice::DrawInstanced;
        m_pDDITable->pfnDrawInstancedIndirect = TDevice::DrawInstancedIndirect;
        m_pDDITable->pfnDispatch = TDevice::Dispatch;
        m_pDDITable->pfnDispatchIndirect = TDevice::DispatchIndirect;

        m_pDDITable->pfnSetBlendState = TDevice::SetBlendState;
        m_pDDITable->pfnSetDepthStencilState = TDevice::SetDepthStencilState;
        m_pDDITable->pfnSetRasterizerState = TDevice::SetRasterizerState;

        m_pDDITable->pfnIaSetIndexBuffer = TDevice::IaSetIndexBuffer;
        m_pDDITable->pfnIaSetInputLayout = TDevice::IaSetInputLayout;
        m_pDDITable->pfnIaSetTopology = TDevice::IaSetTopology;
        m_pDDITable->pfnIaSetVertexBuffers = TDevice::IaSetVertexBuffers;

        m_pDDITable->pfnSoSetTargets = TDevice::SoSetTargets;
        m_pDDITable->pfnSetRenderTargets = TDevice::OMSetRenderTargets;
        m_pDDITable->pfnCsSetUnorderedAccessViews = TDevice::CsSetUnorderedAccessViews;

        m_pDDITable->pfnSetViewports = TDevice::SetViewports;
        m_pDDITable->pfnSetScissorRects = TDevice::SetScissorRects;

        m_pDDITable->pfnClearDepthStencilView = TDevice::ClearDepthStencilView;
        m_pDDITable->pfnClearRenderTargetView = TDevice::ClearRenderTargetView;
        m_pDDITable->pfnClearUnorderedAccessViewFloat = TDevice::ClearUnorderedAccessViewFloat;
        m_pDDITable->pfnClearUnorderedAccessViewUint = TDevice::ClearUnorderedAccessViewUint;
        m_pDDITable->pfnClearView = TDevice::ClearView;

        m_pDDITable->pfnDiscard = TDevice::Discard;
        m_pDDITable->pfnGenMips = TDevice::GenMips;

        m_pDDITable->pfnQueryBegin = TDevice::QueryBegin;
        m_pDDITable->pfnQueryEnd = TDevice::QueryEnd;
        m_pDDITable->pfnSetPredication = TDevice::SetPredication;

        if (m_pAdapter->m_bComputeOnly)
        {
            m_pDDITable->pfnDynamicIABufferMapNoOverwrite = TDevice::MapNoOverwriteComputeOnlyResource;
            m_pDDITable->pfnDynamicIABufferMapDiscard = TDevice::MapDiscardComputeOnlyResource;
            m_pDDITable->pfnDynamicIABufferUnmap = TDevice::UnmapComputeOnlyResource;
            m_pDDITable->pfnDynamicConstantBufferMapDiscard = TDevice::MapDiscardComputeOnlyResource;
            m_pDDITable->pfnDynamicConstantBufferUnmap = TDevice::UnmapComputeOnlyResource;
            m_pDDITable->pfnDynamicResourceMapDiscard = TDevice::MapDiscardComputeOnlyResource;
            m_pDDITable->pfnDynamicResourceUnmap = TDevice::UnmapComputeOnlyResource;
            m_pDDITable->pfnDynamicConstantBufferMapNoOverwrite = TDevice::MapNoOverwriteComputeOnlyResource;
        }
        else
        {
            m_pDDITable->pfnDynamicIABufferMapNoOverwrite = TDevice::MapRenamedBuffer;
            m_pDDITable->pfnDynamicIABufferMapDiscard = TDevice::RenameAndMapBuffer;
            m_pDDITable->pfnDynamicIABufferUnmap = TDevice::UnmapRenamedBuffer;
            m_pDDITable->pfnDynamicConstantBufferMapDiscard = TDevice::RenameAndMapBuffer;
            m_pDDITable->pfnDynamicConstantBufferUnmap = TDevice::UnmapRenamedBuffer;
            m_pDDITable->pfnDynamicResourceMapDiscard = TDevice::MapDiscardResource;
            m_pDDITable->pfnDynamicResourceUnmap = TDevice::UnmapDynamicResource;
            m_pDDITable->pfnDynamicConstantBufferMapNoOverwrite = TDevice::MapRenamedBuffer;
        }

        m_pDDITable->pfnResourceCopy = TDevice::ResourceCopy;
        m_pDDITable->pfnResourceCopyRegion = TDevice::ResourceCopyRegion;
        m_pDDITable->pfnResourceConvert = TDevice::ResourceCopy;
        m_pDDITable->pfnResourceConvertRegion = TDevice::ResourceCopyRegion;
        m_pDDITable->pfnResourceResolveSubresource = TDevice::ResourceResolveSubresource;

        m_pDDITable->pfnDefaultConstantBufferUpdateSubresourceUP = TDevice::ResourceUpdateSubresourceUP;
        m_pDDITable->pfnResourceUpdateSubresourceUP = TDevice::ResourceUpdateSubresourceUP;

        m_pDDITable->pfnSetResourceMinLOD = TDevice::SetResourceMinLOD;
        m_pDDITable->pfnCopyStructureCount = TDevice::CopyStructureCount;

        m_pDDITable->pfnRelocateDeviceFuncs = TDevice::RelocateDeviceFuncs;

        m_pDDITable->pfnUpdateTileMappings = TDevice::UpdateTileMappings;
        m_pDDITable->pfnCopyTileMappings = TDevice::CopyTileMappings;
        m_pDDITable->pfnCopyTiles = TDevice::CopyTiles;
        m_pDDITable->pfnUpdateTiles = TDevice::UpdateTiles;
        m_pDDITable->pfnTiledResourceBarrier = TDevice::TiledResourceBarrier;
        m_pDDITable->pfnResizeTilePool = TDevice::ResizeTilePool;

        m_pDDITable->pfnSetMarkerMode = nullptr;
        m_pDDITable->pfnSetMarker = nullptr;

        m_pDDITable->pfnShaderResourceViewReadAfterWriteHazard = TDevice::ShaderResourceViewReadAfterWriteHazard;
        m_pDDITable->pfnResourceReadAfterWriteHazard = TDevice::ResourceReadAfterWriteHazard;

        m_pDDITable->pfnSetHardwareProtection = TDevice::SetHardwareProtection;
        m_pDDITable->pfnSetHardwareProtectionState = TDevice::SetHardwareProtectionState;
        m_pDDITable->pfnSetShaderCacheSession = TDevice::SetShaderCache;

        m_pDDITable->pfnDestroyDevice = TDevice::DestroyDevice;
        m_pDDITable->pfnCommandListExecute = TDevice::ExecuteCommandList;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    Device::Device(Adapter* pAdapter, D3D10DDIARG_CREATEDEVICE* pArgs)
        : DeviceBase(pAdapter, m_ImmediateContext, GetBatchedContextCallbacks(), m_GraphicsPSOCache, m_ComputePSOCache, *pArgs)
        , m_pDXGITable(pArgs->DXGIBaseDDI.pDXGIDDIBaseFunctions6_1)
        , m_hRTDevice(pArgs->hRTDevice)
        , m_pKTCallbacks(pArgs->pKTCallbacks)
        , m_pDXGICallbacks(pArgs->DXGIBaseDDI.pDXGIBaseCallbacks)
        , m_translationCallbacks(GetTranslationLayerCallbacks())
        , m_ImmediateContext(pAdapter->m_NodeIndex,
                       pAdapter->m_Caps,
                       pAdapter->m_pUnderlyingDevice,
                       pAdapter->m_p3DCommandQueue,
                       m_translationCallbacks,
                       GetDebugFlags(),
                       GetImmCtxArgs(pAdapter, pArgs->Flags))
        , m_SharedResourceHelpers(m_ImmediateContext, GetSharedResourceFlags(pAdapter))
        , m_ResourceAllocationContext(GetResourceAllocationContextImpl(pArgs->Flags))
    {
        FillContextDDIs<Device>();

        m_pDDITable->pfnFlush = Flush;

        m_pDDITable->pfnGetResourceLayout = nullptr;

        m_pDDITable->pfnRetrieveShaderComment = RetrieveShaderComment;

        m_pDDITable->pfnStagingResourceMap = MapUnderlyingSynchronize;
        m_pDDITable->pfnStagingResourceUnmap = UnmapUnderlyingStaging;
        m_pDDITable->pfnResourceMap = MapDefault;
        m_pDDITable->pfnResourceUnmap = UnmapDefault;
        m_pDDITable->pfnQueryGetData = Query::QueryGetData;

        m_pDDITable->pfnGetMipPacking = GetMipPacking;
        m_pDDITable->pfnResourceIsStagingBusy = ResourceIsStagingBusy;

        m_pDDITable->pfnCheckFormatSupport = CheckFormatSupport;
        m_pDDITable->pfnCheckMultisampleQualityLevels = CheckMultisampleQualityLevels;
        m_pDDITable->pfnCheckCounterInfo = CheckCounterInfo;
        m_pDDITable->pfnCheckCounter = CheckCounter;

        m_pDDITable->pfnCalcPrivateBlendStateSize = CalcPrivateBlendStateSize;
        m_pDDITable->pfnCreateBlendState = CreateBlendState;
        m_pDDITable->pfnDestroyBlendState = DestroyBlendState;
        m_pDDITable->pfnCalcPrivateDepthStencilStateSize = CalcPrivateDepthStencilStateSize;
        m_pDDITable->pfnCreateDepthStencilState = CreateDepthStencilState;
        m_pDDITable->pfnDestroyDepthStencilState = DestroyDepthStencilState;
        m_pDDITable->pfnCalcPrivateRasterizerStateSize = CalcPrivateRasterizerStateSize;
        m_pDDITable->pfnCreateRasterizerState = CreateRasterizerState;
        m_pDDITable->pfnDestroyRasterizerState = DestroyRasterizerState;
        m_pDDITable->pfnCalcPrivateSamplerSize = Sampler::CalcPrivateSamplerSize;
        m_pDDITable->pfnCreateSampler = Sampler::CreateSampler;
        m_pDDITable->pfnDestroySampler = Sampler::DestroySampler;

        m_pDDITable->pfnCalcPrivateQuerySize = Query::CalcPrivateQuerySize;
        m_pDDITable->pfnCreateQuery = Query::CreateQuery;
        m_pDDITable->pfnDestroyQuery = Query::DestroyQuery;

        m_pDDITable->pfnCalcPrivateResourceSize = Resource::CalcPrivateResourceSize;
        m_pDDITable->pfnCreateResource = Resource::CreateResource;
        m_pDDITable->pfnCalcPrivateOpenedResourceSize = Resource::CalcPrivateOpenedResourceSize;
        m_pDDITable->pfnOpenResource = Resource::OpenResource;
        m_pDDITable->pfnDestroyResource = Resource::DestroyResource;

        m_pDDITable->pfnCalcPrivateDepthStencilViewSize = CalcPrivateDepthStencilViewSize;
        m_pDDITable->pfnCreateDepthStencilView = CreateDepthStencilView;
        m_pDDITable->pfnDestroyDepthStencilView = DestroyDepthStencilView;

        m_pDDITable->pfnCalcPrivateRenderTargetViewSize = CalcPrivateRenderTargetViewSize;
        m_pDDITable->pfnCreateRenderTargetView = CreateRenderTargetView;
        m_pDDITable->pfnDestroyRenderTargetView = DestroyRenderTargetView;

        m_pDDITable->pfnCalcPrivateShaderResourceViewSize = CalcPrivateShaderResourceViewSize;
        m_pDDITable->pfnCreateShaderResourceView = CreateShaderResourceView;
        m_pDDITable->pfnDestroyShaderResourceView = DestroyShaderResourceView;

        m_pDDITable->pfnCalcPrivateUnorderedAccessViewSize = CalcPrivateUnorderedAccessViewSize;
        m_pDDITable->pfnCreateUnorderedAccessView = CreateUnorderedAccessView;
        m_pDDITable->pfnDestroyUnorderedAccessView = DestroyUnorderedAccessView;

        m_pDDITable->pfnCalcPrivateElementLayoutSize = CalcPrivateElementLayoutSize;
        m_pDDITable->pfnCreateElementLayout = CreateElementLayout;
        m_pDDITable->pfnDestroyElementLayout = DestroyElementLayout;

        m_pDDITable->pfnCalcPrivateShaderSize = CalcPrivateShaderSize;
        m_pDDITable->pfnCalcPrivateGeometryShaderWithStreamOutput = CalcPrivateGeometryShaderWithStreamOutput;
        m_pDDITable->pfnCalcPrivateTessellationShaderSize = CalcPrivateTessellationShaderSize;
        m_pDDITable->pfnDestroyShader = DestroyShader;

        m_pDDITable->pfnAcquireResource = AcquireResource;
        m_pDDITable->pfnReleaseResource = ReleaseResource;

        m_pDDITable->pfnCalcPrivateShaderCacheSessionSize = ShaderCache::CalcPrivateSize;
        m_pDDITable->pfnCreateShaderCacheSession = ShaderCache::Create;
        m_pDDITable->pfnDestroyShaderCacheSession = ShaderCache::Destroy;

        m_pDDITable->pfnCheckDeferredContextHandleSizes = CheckDeferredContextHandleSizes;
        m_pDDITable->pfnCalcDeferredContextHandleSize = CalcDeferredContextHandleSize;
        m_pDDITable->pfnCalcPrivateDeferredContextSize = CalcPrivateDeferredContextSize;
        m_pDDITable->pfnCreateDeferredContext = CreateDeferredContext;
        m_pDDITable->pfnRecycleCreateDeferredContext = DeviceDeferred::RecycleCreateDeferredContext;

        m_pDDITable->pfnCalcPrivateCommandListSize = CommandList::CalcPrivateSize;
        m_pDDITable->pfnCreateCommandList = CommandList::Create;
        m_pDDITable->pfnDestroyCommandList = CommandList::Destroy;
        m_pDDITable->pfnRecycleDestroyCommandList = CommandList::Destroy;
        m_pDDITable->pfnRecycleCreateCommandList = CommandList::RecycleCreate;
        m_pDDITable->pfnRecycleCommandList = nullptr; // Deferred context method only

        m_pDDITable->pfnQueryScanoutCaps = QueryScanoutCaps;
        m_pDDITable->pfnPrepareScanoutTransformation = PrepareScanoutTransformation;

        assert(pArgs->Interface == D3DWDDM2_7_DDI_INTERFACE_VERSION);

        m_pDXGITable->pfnBlt = Blt;
        m_pDXGITable->pfnBlt1 = Blt1;
        m_pDXGITable->pfnPresent1 = Present1;
        m_pDXGITable->pfnRotateResourceIdentities = RotateResourceIdentities;
        m_pDXGITable->pfnResolveSharedResource = ResolveSharedResource;
        // TODO
        m_pDXGITable->pfnTrimResidencySet = TrimResidencySet;
        m_pDXGITable->pfnSetResourcePriority = SetResourcePriority;
        m_pDXGITable->pfnCheckMultiplaneOverlayColorSpaceSupport = CheckMultiplaneOverlayColorSpaceSupport;
        m_pDXGITable->pfnPresentMultiplaneOverlay1 = PresentMultiplaneOverlay1;

        // Not implemented (Handled by DXGIOn12)
        m_pDXGITable->pfnPresent = nullptr;
        m_pDXGITable->pfnCheckPresentDurationSupport = nullptr;
        m_pDXGITable->pfnGetGammaCaps = nullptr;
        m_pDXGITable->pfnGetMultiplaneOverlayCaps = nullptr;
        m_pDXGITable->pfnGetMultiplaneOverlayGroupCaps = nullptr;
        m_pDXGITable->pfnOfferResources1 = nullptr;
        m_pDXGITable->pfnReclaimResources = nullptr;
        m_pDXGITable->pfnPresentMultiplaneOverlay = nullptr;
        m_pDXGITable->pfnQueryResourceResidency = nullptr;
        m_pDXGITable->pfnSetDisplayMode = nullptr;

        *pArgs->ppfnRetrieveSubObject = RetrieveSubObject;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    Device::~Device()
    {
        for (auto& buffer : m_spInterfaceBuffers) // Before the immediate context is destroyed.
        {
            buffer.reset();
        }
        GetBatchedContext().ProcessBatch();
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    DeviceDeferred::DeviceDeferred(Device* pBaseDevice, D3D11DDIARG_CREATEDEFERREDCONTEXT const* pArgs)
        : DeviceBase(pBaseDevice->GetAdapter(),
                     pBaseDevice->GetImmediateContextNoFlush(),
                     GetBatchedContextCallbacks(),
                     pBaseDevice->m_GraphicsPSOCache,
                     pBaseDevice->m_ComputePSOCache,
                     DeferredCtxArgs{ pArgs, pBaseDevice })
        , m_pBaseDevice(pBaseDevice)
    {
        // Now that we might have multiple threads accessing these caches, add locks to them
        m_GraphicsPSOCache.InitLock();
        m_ComputePSOCache.InitLock();

        PopulateDeferredInitMethod(m_pDDITable->pfnCreateBlendState, m_pDDITable->pfnDestroyBlendState);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateDepthStencilState, m_pDDITable->pfnDestroyDepthStencilState);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateRasterizerState, m_pDDITable->pfnDestroyRasterizerState);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateSampler, m_pDDITable->pfnDestroySampler);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateRenderTargetView, m_pDDITable->pfnDestroyRenderTargetView);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateDepthStencilView, m_pDDITable->pfnDestroyDepthStencilView);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateShaderResourceView, m_pDDITable->pfnDestroyShaderResourceView);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateUnorderedAccessView, m_pDDITable->pfnDestroyUnorderedAccessView);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateElementLayout, m_pDDITable->pfnDestroyElementLayout);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateShaderCacheSession, m_pDDITable->pfnDestroyShaderCacheSession);
        PopulateDeferredInitMethod(m_pDDITable->pfnCreateCommandList, m_pDDITable->pfnDestroyCommandList);
        PopulateDeferredShaderInit(m_pDDITable->pfnCreateComputeShader);
        PopulateDeferredShaderInit(m_pDDITable->pfnCreateVertexShader);
        PopulateDeferredShaderInit(m_pDDITable->pfnCreatePixelShader);
        PopulateDeferredShaderInit(m_pDDITable->pfnCreateGeometryShader);
        PopulateDeferredShaderInit(m_pDDITable->pfnCreateDomainShader);
        PopulateDeferredShaderInit(m_pDDITable->pfnCreateHullShader);
        m_pDDITable->pfnDestroyShader = [](D3D10DDI_HDEVICE, D3D10DDI_HSHADER) {};
        m_pDDITable->pfnAbandonCommandList = [](D3D10DDI_HDEVICE) {};

        FillContextDDIs<DeviceDeferred>();

        m_pDDITable->pfnCreateResource = Resource::CreateDeferredResource;
        m_pDDITable->pfnDestroyResource = Resource::DestroyResource;
        m_pDDITable->pfnCreateQuery = Query::CreateDeferred;
        m_pDDITable->pfnDestroyQuery = Query::DestroyQuery;
        m_pDDITable->pfnRecycleCommandList = CommandList::Recycle;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void Device::ReportError(HRESULT hr) noexcept
    {
        if (FAILED(hr) && hr != DXGI_DDI_ERR_WASSTILLDRAWING)
        {
            // Force the runtime to realize device removal from "kernel" (the underlying device)
            // This prevents device removed errors from being stomped to "driver internal error"
            D3DDDICB_ESCAPE EscapeCB = {};
            D3DDDI_EXECUTIONSTATEESCAPE StateCheck = {};
            EscapeCB.hDevice = m_hRTDevice.handle;
            EscapeCB.pPrivateDriverData = &StateCheck;
            EscapeCB.PrivateDriverDataSize = sizeof(StateCheck);
            EscapeCB.Flags.DeviceStatusQuery = 1;
            (void)m_pKTCallbacks->pfnEscapeCb(nullptr, &EscapeCB);
        }
        DeviceBase::ReportError(hr);
    }
    void DeviceBase::ReportError(HRESULT hr) noexcept
    {
        m_pCallbacks->pfnSetErrorCb(m_hRTCoreLayer, hr);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void Device::PostSubmit()
    {
        // Trigger amortized operations
        m_pCallbacks->pfnPerformAmortizedProcessingCb(m_hRTCoreLayer);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    STDMETHODIMP_(void) Device::WriteToSubresource(D3D10DDI_HRESOURCE hDstResource, UINT DstSubresource, _In_opt_ const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) noexcept
    {
        FlushBatchAndGetImmediateContext().WriteToSubresource(Resource::CastFromAndGetImmediateResource(hDstResource), DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    STDMETHODIMP_(void) Device::ReadFromSubresource(void* pDstData, UINT DstRowPitch, UINT DstDepthPitch, D3D10DDI_HRESOURCE hSrcResource, UINT SrcSubresource, _In_opt_ const D3D11_BOX* pSrcBox) noexcept
    {
        FlushBatchAndGetImmediateContext().ReadFromSubresource(pDstData, DstRowPitch, DstDepthPitch, Resource::CastFromAndGetImmediateResource(hSrcResource), SrcSubresource, pSrcBox);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void DeviceBase::PreDraw()
    {
        if (m_DirtyStates & e_GraphicsPipelineStateDirty)
        {
            m_pGraphicsPSO = CreateOrRetrievePSO<D3D12TranslationLayer::e_Draw>();
            m_StatesToReassert |= e_GraphicsPipelineStateDirty;

            UploadInterfaceDataIfNecessary(D3D12TranslationLayer::e_VS);
            UploadInterfaceDataIfNecessary(D3D12TranslationLayer::e_PS);
            UploadInterfaceDataIfNecessary(D3D12TranslationLayer::e_GS);
            UploadInterfaceDataIfNecessary(D3D12TranslationLayer::e_HS);
            UploadInterfaceDataIfNecessary(D3D12TranslationLayer::e_DS);
        }

        if (m_StatesToReassert & e_GraphicsPipelineStateDirty)
        {
            GetBatchedContext().SetPipelineState(m_pGraphicsPSO);
        }

        m_DirtyStates &= ~e_GraphicsPipelineStateDirty;
        m_StatesToReassert &= ~e_GraphicsPipelineStateDirty;


        // The PSO was set to be a graphics PSO
        // The next dispatch call must set a compute PSO
        m_StatesToReassert |= e_ComputePipelineStateDirty;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void DeviceBase::PreDispatch()
    {
        // Note: Reserve operations may cause root signature to change, so PSO should be constructed afterwards
        if (m_DirtyStates & e_ComputePipelineStateDirty)
        {
            m_pComputePSO = CreateOrRetrievePSO<D3D12TranslationLayer::e_Dispatch>(); // throw ( _com_error, bad_alloc )
            m_StatesToReassert |= e_ComputePipelineStateDirty;

            UploadInterfaceDataIfNecessary(D3D12TranslationLayer::e_CS);
        }

        if (m_StatesToReassert & e_ComputePipelineStateDirty)
        {
            GetBatchedContext().SetPipelineState(m_pComputePSO);
        }

        m_StatesToReassert &= ~e_ComputePipelineStateDirty;
        m_DirtyStates &= ~e_ComputePipelineStateDirty;

        // The PSO was set to be a graphics PSO
        // The next dispatch call must set a compute PSO
        m_StatesToReassert |= e_GraphicsPipelineStateDirty;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void DeviceBase::InitInterfacesBuffer(D3D12TranslationLayer::EShaderStage eShaderType) noexcept(false)
    {
        if (m_spInterfaceBuffers[eShaderType])
        {
            return;
        }

        D3D12TranslationLayer::ResourceCreationArgs CBArgs = {};
        static constexpr UINT MaxCBSize = D3D11_SHADER_MAX_INTERFACES * 8 * sizeof(UINT);
        CBArgs.m_appDesc = D3D12TranslationLayer::AppResourceDesc(1, 1, 1, 1, 1, 1, MaxCBSize, 1, DXGI_FORMAT_UNKNOWN, 1, 0,
                                                                  D3D12TranslationLayer::RESOURCE_USAGE_DYNAMIC, D3D12TranslationLayer::RESOURCE_CPU_ACCESS_WRITE,
                                                                  D3D12TranslationLayer::RESOURCE_BIND_CONSTANT_BUFFER, D3D12_RESOURCE_DIMENSION_BUFFER);
        CBArgs.m_heapDesc = CD3DX12_HEAP_DESC(MaxCBSize, GetImmediateContextNoFlush().GetHeapProperties(D3D12_HEAP_TYPE_UPLOAD));
        CBArgs.m_desc12 = CD3DX12_RESOURCE_DESC::Buffer(MaxCBSize);
        CBArgs.m_flags11 = { D3D11_BIND_CONSTANT_BUFFER, 0, D3D11_CPU_ACCESS_WRITE, 0 };
        unique_comptr<D3D12TranslationLayer::Resource> spCB =
            D3D12TranslationLayer::Resource::CreateResource(&GetImmediateContextNoFlush(), CBArgs, D3D12TranslationLayer::ResourceAllocationContext::FreeThread);
        m_spInterfaceBuffers[eShaderType].reset(new D3D12TranslationLayer::BatchedResource(m_PrimaryBatchedContext, spCB.release(), true));
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void DeviceBase::UploadInterfaceDataIfNecessary(D3D12TranslationLayer::EShaderStage eShaderType) noexcept
    {
        if (m_bInterfaceBufferDirty[eShaderType])
        {
            D3D12TranslationLayer::MappedSubresource Data;
            GetBatchedContext().RenameAndMapBuffer(m_spInterfaceBuffers[eShaderType].get(), &Data);

            UINT* pDataUINTs = reinterpret_cast<UINT*>(Data.pData);
            for (UINT i = 0; i < m_InterfaceInstances[eShaderType].size(); ++i)
            {
                *pDataUINTs = m_InterfaceInstances[eShaderType][i];
                pDataUINTs += 4; // Legacy CB alignment issues
            }
            for (UINT i = 0; i < m_InterfacePointerData[eShaderType].size(); ++i)
            {
                pDataUINTs[0] = m_InterfacePointerData[eShaderType][i].uCBID;
                pDataUINTs[1] = m_InterfacePointerData[eShaderType][i].uCBOffset;
                pDataUINTs[2] = m_InterfacePointerData[eShaderType][i].uBaseTex;
                pDataUINTs[3] = m_InterfacePointerData[eShaderType][i].uBaseSamp;
                pDataUINTs += 4;
            }

            m_bInterfaceBufferDirty[eShaderType] = false;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    template<D3D12TranslationLayer::EPipelineType Type> struct PSOTraits;
    template<> struct PSOTraits<D3D12TranslationLayer::e_Draw>
    {
        typedef D3D12TranslationLayer::GRAPHICS_PIPELINE_STATE_DESC APIPSODesc;
        static APIPSODesc GetPSODesc(PSODesc& Desc, ComputePSOKey& /*ComputeDesc*/, UINT NodeMask)
        {
            APIPSODesc APIDesc = Desc;
            APIDesc.NodeMask = NodeMask;
            return APIDesc;
        }
    };
    template<> struct PSOTraits<D3D12TranslationLayer::e_Dispatch>
    {
        typedef D3D12TranslationLayer::COMPUTE_PIPELINE_STATE_DESC APIPSODesc;
        static APIPSODesc GetPSODesc(PSODesc& /*Desc*/, ComputePSOKey& ComputeDesc, UINT NodeMask)
        {
            APIPSODesc APIDesc = { };
            APIDesc.pCompute = ComputeDesc.pCS->Underlying();
            APIDesc.NodeMask = NodeMask;
            return APIDesc;
        }
    };

    //----------------------------------------------------------------------------------------------------------------------------------
    template<D3D12TranslationLayer::EPipelineType Type>
    D3D12TranslationLayer::PipelineState* DeviceBase::CreateOrRetrievePSO() noexcept(false)
    {
        typedef PSOTraits<Type> PSOTraits;
        typedef typename PSOCacheKey<Type>::KeyType Key;

        PipelineStateCacheLocked<Key> PSOCache = GetPSOCache<Type>();
        Key const& PSOKey = GetPSOCacheKey<Type>();

        auto iter = PSOCache->find(PSOKey);
        if (iter != PSOCache->end())
        {
            return iter->second->pState.get();
        }

        std::shared_ptr<PipelineStateCacheEntry<Key>> spCached = std::make_shared<PipelineStateCacheEntry<Key>>(m_PrimaryBatchedContext); // throw( bad_alloc );
        // No cached PSO exists, time to create one
        PipelineStateCacheEntry<Key>& CacheEntry = *spCached;

        typename PSOTraits::APIPSODesc APIDesc = PSOTraits::GetPSODesc(m_PSODesc, m_ComputePSODesc, GetImmediateContextNoFlush().GetNodeMask());
        CacheEntry.pState.reset( new D3D12TranslationLayer::PipelineState(&GetImmediateContextNoFlush(), APIDesc)); // throw( bad_alloc, _com_error )
        
        AddUses(PSOKey, spCached);
        iter = PSOCache->emplace(PSOKey, spCached).first;
        spCached->pKey = &iter->first;
        return spCached->pState.get();
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    static const D3D11DDI_HANDLESIZE HandleSizes[] =
    {
        {D3D10DDI_HT_RESOURCE,                   sizeof(Resource) },
        {D3D10DDI_HT_SHADERRESOURCEVIEW,         sizeof(DeviceChildDeferred<D3D10DDI_HSHADERRESOURCEVIEW>)},
        {D3D10DDI_HT_RENDERTARGETVIEW,           sizeof(DeviceChildDeferred<D3D10DDI_HRENDERTARGETVIEW>)},
        {D3D10DDI_HT_DEPTHSTENCILVIEW,           sizeof(DeviceChildDeferred<D3D10DDI_HDEPTHSTENCILVIEW>)},
        {D3D10DDI_HT_SHADER,                     sizeof(DeviceChildDeferred<D3D10DDI_HSHADER>)},
        {D3D10DDI_HT_ELEMENTLAYOUT,              sizeof(DeviceChildDeferred<D3D10DDI_HELEMENTLAYOUT>)},
        {D3D10DDI_HT_BLENDSTATE,                 sizeof(DeviceChildDeferred<D3D10DDI_HBLENDSTATE>)},
        {D3D10DDI_HT_DEPTHSTENCILSTATE,          sizeof(DeviceChildDeferred<D3D10DDI_HDEPTHSTENCILSTATE>)},
        {D3D10DDI_HT_RASTERIZERSTATE,            sizeof(DeviceChildDeferred<D3D10DDI_HRASTERIZERSTATE>)},
        {D3D10DDI_HT_SAMPLERSTATE,               sizeof(DeviceChildDeferred<D3D10DDI_HSAMPLER>)},
        {D3D10DDI_HT_QUERY,                      sizeof(Query)},
        {D3D11DDI_HT_COMMANDLIST,                sizeof(DeviceChildDeferred<D3D11DDI_HCOMMANDLIST>)},
        {D3D11DDI_HT_UNORDEREDACCESSVIEW,        sizeof(DeviceChildDeferred<D3D11DDI_HUNORDEREDACCESSVIEW>)},
        {D3D11_1DDI_HT_DECODE,                   sizeof(DeviceChildDeferred<D3D11_1DDI_HDECODE>)},
        {D3D11_1DDI_HT_VIDEOPROCESSORENUM,       sizeof(DeviceChildDeferred<D3D11_1DDI_HVIDEOPROCESSORENUM>)},
        {D3D11_1DDI_HT_VIDEOPROCESSOR,           sizeof(DeviceChildDeferred<D3D11_1DDI_HVIDEOPROCESSOR>)},
        {D3D11_1DDI_HT_VIDEODECODEROUTPUTVIEW,   sizeof(DeviceChildDeferred<D3D11_1DDI_HVIDEODECODEROUTPUTVIEW>)},
        {D3D11_1DDI_HT_VIDEOPROCESSORINPUTVIEW,  sizeof(DeviceChildDeferred<D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW>)},
        {D3D11_1DDI_HT_VIDEOPROCESSOROUTPUTVIEW, sizeof(DeviceChildDeferred<D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW>)},
        {D3DWDDM2_2DDI_HT_CACHESESSION,          sizeof(DeviceChildDeferred<D3DWDDM2_2DDI_HCACHESESSION>)},
    };
    void APIENTRY Device::CheckDeferredContextHandleSizes(D3D10DDI_HDEVICE, UINT* pNumSizes, D3D11DDI_HANDLESIZE* pSizes) noexcept
    {
        *pNumSizes = std::extent<decltype(HandleSizes)>::value;
        if (pSizes)
        {
            std::copy(HandleSizes, std::end(HandleSizes), pSizes);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    SIZE_T APIENTRY Device::CalcDeferredContextHandleSize(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE handleType, void*) noexcept
    {
        return std::find_if(HandleSizes, std::end(HandleSizes), [handleType](D3D11DDI_HANDLESIZE const& pair)
        {
            return pair.HandleType == handleType;
        })->DriverPrivateSize;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    SIZE_T APIENTRY Device::CalcPrivateDeferredContextSize(D3D10DDI_HDEVICE, D3D11DDIARG_CALCPRIVATEDEFERREDCONTEXTSIZE const*) noexcept
    {
        return sizeof(DeviceDeferred);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY Device::CreateDeferredContext(D3D10DDI_HDEVICE hDevice, D3D11DDIARG_CREATEDEFERREDCONTEXT const* pArgs) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        auto pDeferredDevice = new (pArgs->hDrvContext.pDrvPrivate) DeviceDeferred(pDevice, pArgs); // throw
        pDeferredDevice->GetBatchedContext().ClearState(); // Record a ClearState command into this context's batch
        pDeferredDevice->ClearTrackedState();

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY DeviceDeferred::RecycleCreateDeferredContext(D3D10DDI_HDEVICE, D3D11DDIARG_CREATEDEFERREDCONTEXT const* pArgs) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDeferredDevice = CastFrom(pArgs->hDrvContext);
        pDeferredDevice->GetBatchedContext().ClearState(); // Record a ClearState command into this context's batch
        pDeferredDevice->ClearTrackedState();
        pDeferredDevice->m_bMapDiscardCalled = false;
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY DeviceBase::ExecuteCommandList(D3D10DDI_HDEVICE hDevice, D3D11DDI_HCOMMANDLIST hCmdList) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        auto pCommandList = CommandList::CastFrom(hCmdList);

        ++pCommandList->m_ExecutionCount;
        // We don't support this yet.
        MICROSOFT_TELEMETRY_ASSERT(pCommandList->m_ExecutionCount == 0 || !pCommandList->m_bMapDiscardCalled);
        pDevice->m_bMapDiscardCalled |= pCommandList->m_bMapDiscardCalled;

        pDevice->GetBatchedContext().SubmitCommandListBatch(pCommandList->m_CommandList.get());
        // It's guaranteed that a command list both begins and ends with a ClearState command, so we'll
        // clear our own tracked state here.
        pDevice->ClearTrackedState();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    SIZE_T APIENTRY CommandList::CalcPrivateSize(D3D10DDI_HDEVICE, D3D11DDIARG_CREATECOMMANDLIST const*)
    {
        return sizeof(CommandList);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY CommandList::Create(D3D10DDI_HDEVICE hDevice, D3D11DDIARG_CREATECOMMANDLIST const* pArgs, D3D11DDI_HCOMMANDLIST handle, D3D11DDI_HRTCOMMANDLIST rthandle)
    {
        // Note: Forwarding to RecycleCreate as that one returns the HRESULT instead of reporting it.
        // If it fails, report the error here.
        HRESULT hr = CommandList::RecycleCreate(hDevice, pArgs, handle, rthandle);
        if (FAILED(hr))
        {
            DeviceBase::CastFrom(hDevice)->ReportError(hr);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY CommandList::Destroy(D3D10DDI_HDEVICE hDevice, D3D11DDI_HCOMMANDLIST h)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = Device::CastFrom(hDevice);
        auto pCommandList = CastFrom(h);

        // Defer destruction of batches referenced by command lists to ensure that they stay alive
        // even if they're referenced within other command lists.
        pDevice->GetBatchedContext().AddPostBatchFunction([spBatch = pCommandList->m_CommandList.release(), pDevice]() mutable
        {
            if (spBatch)
            {
                pDevice->GetBatchedContext().RetireBatch(
                    std::unique_ptr<D3D12TranslationLayer::BatchedContext::Batch>(spBatch));
                spBatch = nullptr;
            }
        });

        pCommandList->~CommandList();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY CommandList::RecycleCreate(D3D10DDI_HDEVICE hDevice, D3D11DDIARG_CREATECOMMANDLIST const* pArgs, D3D11DDI_HCOMMANDLIST handle, D3D11DDI_HRTCOMMANDLIST)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = Device::CastFrom(hDevice);
        auto pDeferredDevice = DeviceDeferred::CastFrom(pArgs->hDeferredContext);

        pDeferredDevice->GetBatchedContext().ClearState(); // Command list execution should always finish with a clean state
        auto pBatch = pDeferredDevice->GetBatchedContext().FinishBatch();

        auto pCommandList = new (handle.pDrvPrivate) CommandList(*pDevice);
        pCommandList->m_CommandList = std::move(pBatch);
        pCommandList->m_bMapDiscardCalled = pDeferredDevice->m_bMapDiscardCalled;

        pDeferredDevice->ClearTrackedState();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY CommandList::Recycle(D3D10DDI_HDEVICE, D3D11DDI_HCOMMANDLIST)
    {
        // Don't think anything is needed here.
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void DeviceBase::ClearTrackedState()
    {
        m_PSODesc = PSODesc();
        m_ComputePSODesc = ComputePSOKey();
        m_DirtyStates |= e_GraphicsPipelineStateDirty | e_ComputePipelineStateDirty;
        m_StatesToReassert |= e_GraphicsPipelineStateDirty | e_ComputePipelineStateDirty;
        std::fill(m_bInterfaceBufferBound, std::end(m_bInterfaceBufferBound), false);
        for (auto& vec : m_InterfaceInstances) vec.clear();
        for (auto& vec : m_InterfacePointerData) vec.clear();
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    STDMETHODIMP Device::GetD3D12Device(REFIID riid, void** ppv) noexcept
    {
        return m_pAdapter->m_pUnderlyingDevice->QueryInterface(riid, ppv);
    }
    STDMETHODIMP Device::GetGraphicsQueue(REFIID riid, void** ppv) noexcept
    {
        return GetImmediateContextNoFlush().GetCommandQueue(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS)->QueryInterface(riid, ppv);
    }
    STDMETHODIMP Device::EnqueueSetEvent(_In_ HANDLE hEvent) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        HRESULT hr = FlushBatchAndGetImmediateContext().EnqueueSetEvent(D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK, hEvent);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }
    STDMETHODIMP_(UINT) Device::GetNodeMask() noexcept
    {
        return (1 << m_pAdapter->m_NodeIndex);
    }

    // Resource wrapping helpers
    STDMETHODIMP Device::OpenSharedHandle(_In_ HANDLE hSharedHandle, _Out_writes_bytes_(PrivateDriverDataSize) void* pPrivateDriverData, UINT PrivateDriverDataSize, _Out_ D3DKMT_HANDLE* hKMTHandle) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        m_SharedResourceHelpers.InitializePrivateDriverData(D3D12TranslationLayer::DeferredDestructionType::Completion, pPrivateDriverData, PrivateDriverDataSize);
        *hKMTHandle = m_SharedResourceHelpers.CreateKMTHandle(hSharedHandle);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::CreateWrappingHandle(_In_ IUnknown* pResource, WrapReason reason, _Out_writes_bytes_(PrivateDriverDataSize) void* pPrivateDriverData, UINT PrivateDriverDataSize, _Out_ D3DKMT_HANDLE* hKMTHandle) noexcept
    {
        UNREFERENCED_PARAMETER(reason); // Currently only one reason
        D3D11on12_DDI_ENTRYPOINT_START();
        auto DeferredDestructionType = D3D12TranslationLayer::DeferredDestructionType::Submission;

        m_SharedResourceHelpers.InitializePrivateDriverData(DeferredDestructionType, pPrivateDriverData, PrivateDriverDataSize);
        *hKMTHandle = m_SharedResourceHelpers.CreateKMTHandle(pResource);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::FillResourceInfo(D3DKMT_HANDLE hKMTHandle, _In_ D3D11_RESOURCE_FLAGS const* pFlagOverrides, _Out_ D3D11On12::ResourceInfo* pResourceInfo) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D12TranslationLayer::ResourceInfo resourceInfoNextLayer;
        m_SharedResourceHelpers.QueryResourceInfoFromKMTHandle(hKMTHandle, pFlagOverrides, &resourceInfoNextLayer);
        switch (resourceInfoNextLayer.m_Type)
        {
        default: throw _com_error(E_UNEXPECTED);
        case D3D12TranslationLayer::ResourceType:
            pResourceInfo->m_Type = ResourceType;
            pResourceInfo->Resource.m_ResourceDesc = resourceInfoNextLayer.Resource.m_ResourceDesc;
            pResourceInfo->Resource.m_Flags11 = resourceInfoNextLayer.Resource.m_Flags11;
            pResourceInfo->Resource.m_HeapFlags = resourceInfoNextLayer.Resource.m_HeapFlags;
            pResourceInfo->Resource.m_HeapProps = resourceInfoNextLayer.Resource.m_HeapProps;
            break;
        case D3D12TranslationLayer::TiledPoolType:
            pResourceInfo->m_Type = TiledPoolType;
            pResourceInfo->TiledPool.m_HeapDesc = resourceInfoNextLayer.TiledPool.m_HeapDesc;
            break;
        }
        pResourceInfo->m_bShared = resourceInfoNextLayer.m_bShared;
        pResourceInfo->m_GDIHandle = resourceInfoNextLayer.m_GDIHandle;
        pResourceInfo->m_bNTHandle = resourceInfoNextLayer.m_bNTHandle;
        pResourceInfo->m_bSynchronized = resourceInfoNextLayer.m_bSynchronized;
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP_(void) Device::DestroyKMTHandle(D3DKMT_HANDLE handle) noexcept
    {
        m_SharedResourceHelpers.DestroyKMTHandle(handle);
    }


    // Interop helpers
    STDMETHODIMP_(void) Device::TransitionResourceForRelease(_In_ ID3D11On12DDIResource* pResource, D3D12_RESOURCE_STATES State) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        FlushBatchAndGetImmediateContext().GetResourceStateManager().TransitionResource(
            static_cast<Resource*>(pResource)->ImmediateResource(),
            State,
            D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS,
            D3D12TranslationLayer::SubresourceTransitionFlags::NoBindingTransitions |
            D3D12TranslationLayer::SubresourceTransitionFlags::StateMatchExact |
            D3D12TranslationLayer::SubresourceTransitionFlags::NotUsedInCommandListIfNoStateChange);
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
        assert(SUCCEEDED(EntryPointHr));
    }
    STDMETHODIMP_(void) Device::ApplyAllResourceTransitions() noexcept
    {
        FlushBatchAndGetImmediateContext().GetResourceStateManager().ApplyAllResourceTransitions();
    }
    
    STDMETHODIMP Device::AddResourceWaitsToQueue(_In_ ID3D11On12DDIResource* p11on12DDIResource, _In_ ID3D12CommandQueue* pCommmandQueue) noexcept 
    {
        auto* pResource = static_cast<Resource*>(p11on12DDIResource)->ImmediateResource();

        D3D12TranslationLayer::ImmediateContext& ImmCtx = FlushBatchAndGetImmediateContext();

        for (UINT i = 0; i < (UINT)D3D12TranslationLayer::COMMAND_LIST_TYPE::MAX_VALID; ++i)
        {
            auto* pCommandListManager = ImmCtx.GetCommandListManager(static_cast<D3D12TranslationLayer::COMMAND_LIST_TYPE>(i));
            if (pCommandListManager)
            {
                UINT64 WaitValue = pResource->m_LastUsedCommandListID[i];

                if (WaitValue > pCommandListManager->GetCompletedFenceValue())
                {
                    auto* pFence = pCommandListManager->GetFence();
                    pCommmandQueue->Wait(pFence->Get(), WaitValue);
                }
            }
        }

        return S_OK;
    }
    
    STDMETHODIMP Device::AddDeferredWaitsToResource(_In_ ID3D11On12DDIResource* p11on12DDIResource, UINT NumSync, _In_reads_(NumSync) UINT64* pSignalValues, _In_reads_(NumSync) ID3D12Fence** ppFences) noexcept 
    {
        
        try
        {
            auto* pResource = static_cast<Resource*>(p11on12DDIResource)->ImmediateResource();
            
            std::vector<D3D12TranslationLayer::DeferredWait> DeferredWaits;
            DeferredWaits.reserve(NumSync); // throw( bad_alloc )

            D3D12TranslationLayer::ImmediateContext& ImmCtx = FlushBatchAndGetImmediateContext();

            for (UINT i = 0; i < NumSync; i++)
            {
                const D3D12TranslationLayer::DeferredWait deferredWait = 
                {
                    std::make_shared<D3D12TranslationLayer::Fence>(&ImmCtx, ppFences[i]),
                    pSignalValues[i]
                };
                DeferredWaits.push_back(deferredWait);
            }

            pResource->AddDeferredWaits(DeferredWaits);
        }
        catch( std::bad_alloc& )
        {
            return E_OUTOFMEMORY;
        }
        catch( _com_error& ex )
        {
            return ex.Error();
        }

        return S_OK;
    }

    // Fence helpers
    STDMETHODIMP Device::CreateFence(UINT64 InitialValue, UINT DXGIInternalFenceFlags, _COM_Outptr_ ID3D11On12DDIFence** ppFence) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        typedef enum DXGI_INTERNAL_CREATEFENCE_FLAGS
        {
            DXGI_INTERNAL_CREATEFENCE_SHARED = 0x1,
            DXGI_INTERNAL_CREATEFENCE_SHARED_CROSS_ADAPTER = 0x2,
            DXGI_INTERNAL_CREATEFENCE_DEFERRED_WAITS = 0x4,
            DXGI_INTERNAL_CREATEFENCE_MONITORED = 0x10
        } DXGI_INTERNAL_CREATEFENCE_FLAGS;

        D3D12TranslationLayer::FENCE_FLAGS TranslationFlags =
            ((DXGIInternalFenceFlags & DXGI_INTERNAL_CREATEFENCE_DEFERRED_WAITS)       ? D3D12TranslationLayer::FENCE_FLAG_DEFERRED_WAITS       : D3D12TranslationLayer::FENCE_FLAG_NONE) |
            ((DXGIInternalFenceFlags & DXGI_INTERNAL_CREATEFENCE_SHARED)               ? D3D12TranslationLayer::FENCE_FLAG_SHARED               : D3D12TranslationLayer::FENCE_FLAG_NONE) |
            ((DXGIInternalFenceFlags & DXGI_INTERNAL_CREATEFENCE_SHARED_CROSS_ADAPTER) ? D3D12TranslationLayer::FENCE_FLAG_SHARED_CROSS_ADAPTER : D3D12TranslationLayer::FENCE_FLAG_NONE) |
            ((DXGIInternalFenceFlags & DXGI_INTERNAL_CREATEFENCE_MONITORED) == 0       ? D3D12TranslationLayer::FENCE_FLAG_NON_MONITORED        : D3D12TranslationLayer::FENCE_FLAG_NONE);

        auto spUnderlyingFence = std::make_shared<D3D12TranslationLayer::Fence>(&GetImmediateContextNoFlush(), TranslationFlags, InitialValue);
        *ppFence = new Fence(*this, std::move(spUnderlyingFence));
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::OpenFence(HANDLE hSharedFence, _Out_ bool* pbMonitored, _COM_Outptr_ ID3D11On12DDIFence** ppFence) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto spUnderlyingFence = std::make_shared<D3D12TranslationLayer::Fence>(&GetImmediateContextNoFlush(), hSharedFence);
        *pbMonitored = spUnderlyingFence->IsMonitored();
        *ppFence = new Fence(*this, std::move(spUnderlyingFence));
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::Wait(_In_ ID3D11On12DDIFence* pFence, UINT64 Value) noexcept
    {
        // Note: Batching has already been accounted for, this is on the translation layer immediate context thread.
        D3D11on12_DDI_ENTRYPOINT_START();
        GetImmediateContextNoFlush().Wait(static_cast<Fence*>(pFence)->GetUnderlying(), Value);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::Signal(_In_ ID3D11On12DDIFence* pFence, UINT64 Value) noexcept
    {
        // Note: Batching has already been accounted for, this is on the translation layer immediate context thread.
        D3D11on12_DDI_ENTRYPOINT_START();
        GetImmediateContextNoFlush().Signal(static_cast<Fence*>(pFence)->GetUnderlying().get(), Value);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    STDMETHODIMP Device::CreateVertexShader(D3D10DDI_HSHADER hShader, SHADER_DESC const* pDesc) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        T11On12VertexShader::Create(this, hShader, pDesc); // throws
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::CreatePixelShader(D3D10DDI_HSHADER hShader, SHADER_DESC const* pDesc) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        T11On12PixelShader::Create(this, hShader, pDesc); // throws
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::CreateGeometryShader(D3D10DDI_HSHADER hShader, GEOMETRY_SHADER_DESC const* pDesc, D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT const* pCGSWSOArgs) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        if (pCGSWSOArgs)
        {
            StreamOutShader::Create(this, hShader, pDesc, pCGSWSOArgs); // throws
        }
        else
        {
            T11On12GeometryShader::Create(this, hShader, pDesc); // throws
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::CreateHullShader(D3D10DDI_HSHADER hShader, SHADER_DESC const* pDesc) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        T11On12HullShader::Create(this, hShader, pDesc); // throws
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::CreateDomainShader(D3D10DDI_HSHADER hShader, SHADER_DESC const* pDesc) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        T11On12DomainShader::Create(this, hShader, pDesc); // throws
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }
    STDMETHODIMP Device::CreateComputeShader(D3D10DDI_HSHADER hShader, SHADER_DESC const* pDesc) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        T11On12ComputeShader::Create(this, hShader, pDesc); // throws
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    STDMETHODIMP_(void) DeviceBase::SetMarker(_In_opt_z_ const wchar_t* name) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        GetBatchedContext().SetMarker(name);
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
        assert(SUCCEEDED(EntryPointHr));
    }
    STDMETHODIMP_(void) DeviceBase::BeginEvent(_In_opt_z_ const wchar_t* name) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        GetBatchedContext().BeginEvent(name);
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
        assert(SUCCEEDED(EntryPointHr));
    }
    STDMETHODIMP_(void) DeviceBase::EndEvent() noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        GetBatchedContext().EndEvent();
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
        assert(SUCCEEDED(EntryPointHr));
    }


};