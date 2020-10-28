// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class VideoDevice;
    class Resource;

    // Base class for both immediate context "device" and deferred context devices.
    // Each instance of these owns a batched context, and is capable of recording tokenized commands.
    class DeviceBase : public ID3D11On12DDIDevice
    {
    public:
        typedef D3DWDDM2_6DDI_DEVICEFUNCS DDITableLatest;

        template <typename TArgs>
        DeviceBase(Adapter* pAdapter,
                   D3D12TranslationLayer::ImmediateContext& ImmCtx,
                   D3D12TranslationLayer::BatchedContext::Callbacks batchedCtxCallbacks,
                   D3D12TranslationLayer::COptLockedContainer<GraphicsPipelineStateCache>& GraphicsPSOCache,
                   D3D12TranslationLayer::COptLockedContainer<ComputePipelineStateCache>& ComputePSOCache,
                   TArgs const& Args);
        ~DeviceBase() = default;

        static void APIENTRY RelocateDeviceFuncs(D3D10DDI_HDEVICE hDevice, __in DDITableLatest* pFuncs)
        {
            auto pThis = CastFrom(hDevice);
            DDITableLatest*& pDDITable = *const_cast<DDITableLatest**>(&pThis->m_pDDITable);
            pDDITable = pFuncs;
        }

        static DeviceBase* CastFrom(D3D10DDI_HDEVICE h) noexcept
        {
            return reinterpret_cast<DeviceBase*>(h.pDrvPrivate);
        }

        static UINT CommandListTypeMask(UINT ContextType)
        {
            UINT commandListTypeMask = 0;
            if (ContextType == D3DWDDM2_0DDI_CONTEXTTYPE_ALL)
            {
                commandListTypeMask |= D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK;
            }
            else
            {
                if (ContextType & D3DWDDM2_0DDI_CONTEXTTYPE_VIDEO)
                {
                    commandListTypeMask |= D3D12TranslationLayer::COMMAND_LIST_TYPE_VIDEO_MASK;
                }
                if (ContextType & (D3DWDDM2_0DDI_CONTEXTTYPE_3D | D3DWDDM2_0DDI_CONTEXTTYPE_COMPUTE | D3DWDDM2_0DDI_CONTEXTTYPE_COPY))
                {
                    commandListTypeMask |= D3D12TranslationLayer::COMMAND_LIST_TYPE_GRAPHICS_MASK;
                }
            }
            return commandListTypeMask;
        }

        D3D12TranslationLayer::BatchedContext &GetBatchedContext()
        {
            return m_BatchedContext;
        }

        D3D12TranslationLayer::ImmediateContext &GetImmediateContextNoFlush()
        {
            return m_BatchedContext.GetImmediateContextNoFlush();
        }

        Adapter* const  GetAdapter() 
        { 
            return m_pAdapter;
        }

        void ReportError(HRESULT hr) noexcept;
        void PreDraw();
        void PreDispatch();

        template<D3D12TranslationLayer::EPipelineType Type>
        D3D12TranslationLayer::PipelineState* CreateOrRetrievePSO() noexcept(false);

        template <D3D12TranslationLayer::EPipelineType Type>
        typename PSOCacheKey<Type>::KeyType const& GetPSOCacheKey() const noexcept
        {
            if constexpr (Type == D3D12TranslationLayer::e_Draw)
                return m_PSODesc;
            else
                return m_ComputePSODesc;
        }

        template <D3D12TranslationLayer::EPipelineType Type> auto GetPSOCache() noexcept
        {
            if constexpr(Type == D3D12TranslationLayer::e_Draw)
                return m_GraphicsPSOCache.GetLocked();
            else
                return m_ComputePSOCache.GetLocked();
        }

        template <typename Key> auto GetPSOCache() noexcept
        {
            if constexpr(std::is_same<Key, PSODescKey>::value)
                return m_GraphicsPSOCache.GetLocked();
            else
                return m_ComputePSOCache.GetLocked();
        }

        void InitInterfacesBuffer(D3D12TranslationLayer::EShaderStage eShaderStage) noexcept(false);
        void UploadInterfaceDataIfNecessary(D3D12TranslationLayer::EShaderStage eShaderStage) noexcept;

        template <typename TDevice> void FillContextDDIs();

    public:
        template<D3D12TranslationLayer::EShaderStage eShader>
        static void APIENTRY SetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER);

        template<D3D12TranslationLayer::EShaderStage eShader>
        static void APIENTRY SetShaderWithIfaces(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER, UINT NumClassInstances, __in_ecount(NumClassInstances) const UINT*, __in_ecount(NumClassInstances) const D3D11DDIARG_POINTERDATA*);

        template<D3D12TranslationLayer::EShaderStage ShaderUnit>
        static void APIENTRY SetShaderResources(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) UINT NumSRVs, _In_reads_(NumSRVs) const D3D10DDI_HSHADERRESOURCEVIEW* phSRVs);

        template<D3D12TranslationLayer::EShaderStage ShaderUnit>
        static void APIENTRY SetConstantBuffers(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT - 1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT) UINT NumBuffers, _In_reads_(NumBuffers) const D3D10DDI_HRESOURCE* phCBs, _In_reads_opt_(NumBuffers) CONST UINT* pFirstConstant, _In_reads_opt_(NumBuffers) CONST UINT* pNumConstants);

        template<D3D12TranslationLayer::EShaderStage shaderUnit>
        static void APIENTRY SetSamplers(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) UINT NumSamplers, _In_reads_(NumSamplers) const D3D10DDI_HSAMPLER* phSamplers);

        static void APIENTRY Draw(D3D10DDI_HDEVICE hDevice, UINT count, UINT vertexStart);
        static void APIENTRY DrawAuto(D3D10DDI_HDEVICE hDevice);
        static void APIENTRY DrawIndexed(D3D10DDI_HDEVICE hDevice, UINT indexCount, UINT indexStart, INT vertexStart);
        static void APIENTRY DrawIndexedInstanced(D3D10DDI_HDEVICE hDevice, UINT countPerInstance, UINT instanceCount, UINT indexStart, INT vertexStart, UINT instanceStart);
        static void APIENTRY DrawIndexedInstancedIndirect(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hBuffer, UINT offset);
        static void APIENTRY DrawInstanced(D3D10DDI_HDEVICE hDevice, UINT countPerInstance, UINT instanceCount, UINT vertexStart, UINT instanceStart);
        static void APIENTRY DrawInstancedIndirect(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hBuffer, UINT offset);
        static void APIENTRY Dispatch(D3D10DDI_HDEVICE hDevice, UINT x, UINT y, UINT z);
        static void APIENTRY DispatchIndirect(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hBuffer, UINT offset);

        static void APIENTRY SetBlendState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HBLENDSTATE hBlendState, const FLOAT BlendFactor[4], UINT SampleMask);
        static void APIENTRY SetDepthStencilState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILSTATE hDepthState, UINT StencilRef);
        static void APIENTRY SetRasterizerState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRASTERIZERSTATE hRasterizerState);

        static void APIENTRY IaSetTopology(D3D10DDI_HDEVICE hDevice, D3D10_DDI_PRIMITIVE_TOPOLOGY topology);
        static void APIENTRY IaSetInputLayout(D3D10DDI_HDEVICE hDevice, D3D10DDI_HELEMENTLAYOUT hInputLayout);
        static void APIENTRY IaSetVertexBuffers(D3D10DDI_HDEVICE hDevice, UINT StartSlot, __in_range(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT) UINT NumBuffers, const D3D10DDI_HRESOURCE* pVBs, const UINT*, const UINT*);
        static void APIENTRY IaSetIndexBuffer(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hIB, DXGI_FORMAT fmt, UINT);

        static void APIENTRY SoSetTargets(D3D10DDI_HDEVICE hDevice, _In_range_(0, 4) UINT NumTargets, _In_range_(0, 4) UINT ClearSlots, _In_reads_(NumTargets) const D3D10DDI_HRESOURCE* pBuffers, _In_reads_(NumTargets) const UINT* offsets);
        static void APIENTRY OMSetRenderTargets(D3D10DDI_HDEVICE hDevice, CONST __in_ecount(NumRTVs) D3D10DDI_HRENDERTARGETVIEW* pRTVs, __in_range(0, 8) UINT NumRTVs, UINT, __in_opt D3D10DDI_HDEPTHSTENCILVIEW hDSV, CONST __in_ecount(NumUavs) D3D11DDI_HUNORDEREDACCESSVIEW* pUavs, CONST UINT* pInitialCounts, UINT UAVStartSlot, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumUavs, UINT, UINT);
        static void APIENTRY CsSetUnorderedAccessViews(D3D10DDI_HDEVICE hDevice, UINT Start, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumViews, __in_ecount(NumViews) CONST D3D11DDI_HUNORDEREDACCESSVIEW* pUAVs, __in_ecount(NumViews) CONST UINT* pInitialCounts);

        static void APIENTRY SetViewports(D3D10DDI_HDEVICE hDevice, UINT NumViewports, UINT, const D3D10_DDI_VIEWPORT* pViewports);
        static void APIENTRY SetScissorRects(D3D10DDI_HDEVICE hDevice, UINT NumRects, UINT, const D3D10_DDI_RECT* pRects);

        static void APIENTRY QueryBegin(D3D10DDI_HDEVICE, D3D10DDI_HQUERY);
        static void APIENTRY QueryEnd(D3D10DDI_HDEVICE, D3D10DDI_HQUERY);
        static void APIENTRY SetPredication(D3D10DDI_HDEVICE, D3D10DDI_HQUERY, BOOL);

        static void APIENTRY ClearView(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, VOID*, const FLOAT[4], const D3D10_DDI_RECT*, UINT);
        static void APIENTRY ClearRenderTargetView(D3D10DDI_HDEVICE, D3D10DDI_HRENDERTARGETVIEW, FLOAT[4]);
        static void APIENTRY ClearDepthStencilView(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILVIEW, UINT, FLOAT, UINT8);
        static void APIENTRY ClearUnorderedAccessViewUint(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW, CONST UINT[4]);
        static void APIENTRY ClearUnorderedAccessViewFloat(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW, CONST FLOAT[4]);

        static void APIENTRY Discard(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, VOID*, const D3D10_DDI_RECT*, UINT);

        static void APIENTRY GenMips(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW);

        static void APIENTRY MapRenamedBuffer(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
        static void APIENTRY RenameAndMapBuffer(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
        static void APIENTRY MapDiscardResource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
        static void APIENTRY UnmapRenamedBuffer(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);
        static void APIENTRY UnmapDynamicResource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);

        // Compute-only map routines
        static void APIENTRY MapDiscardComputeOnlyResource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
        static void APIENTRY MapNoOverwriteComputeOnlyResource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
        static void APIENTRY UnmapComputeOnlyResource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);

        static void APIENTRY ResourceCopy(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, D3D10DDI_HRESOURCE);
        static void APIENTRY ResourceResolveSubresource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10DDI_HRESOURCE, UINT, DXGI_FORMAT);

        static void APIENTRY SetResourceMinLOD(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, FLOAT);

        static void APIENTRY CopyStructureCount(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D11DDI_HUNORDEREDACCESSVIEW);

        static void APIENTRY ResourceCopyRegion(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, UINT, UINT, UINT, D3D10DDI_HRESOURCE, UINT, const D3D10_DDI_BOX*, UINT);
        static void APIENTRY ResourceUpdateSubresourceUP(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, _In_opt_ const D3D10_DDI_BOX*, _In_ const VOID*, UINT, UINT, UINT);

        static void APIENTRY UpdateTileMappings(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hTiledResource, UINT NumTiledResourceRegions, _In_reads_(NumTiledResourceRegions) const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE* pTiledResourceRegionStartCoords, _In_reads_opt_(NumTiledResourceRegions) const D3DWDDM1_3DDI_TILE_REGION_SIZE* pTiledResourceRegionSizes, D3D10DDI_HRESOURCE hTilePool, UINT NumRanges, _In_reads_opt_(NumRanges) const UINT* pRangeFlags, _In_reads_opt_(NumRanges) const UINT* pTilePoolStartOffsets, _In_reads_opt_(NumRanges) const UINT* pRangeTileCounts, UINT Flags);
        static void APIENTRY CopyTileMappings(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, _In_ const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE*, D3D10DDI_HRESOURCE, _In_ const  D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE*, _In_ const D3DWDDM1_3DDI_TILE_REGION_SIZE*, UINT);
        static void APIENTRY CopyTiles(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, _In_ const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE*, _In_ const D3DWDDM1_3DDI_TILE_REGION_SIZE*, D3D10DDI_HRESOURCE, UINT64, UINT);
        static void APIENTRY UpdateTiles(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, _In_ const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE*, _In_ const D3DWDDM1_3DDI_TILE_REGION_SIZE*, const _In_ VOID*, UINT);
        static void APIENTRY TiledResourceBarrier(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, _In_opt_ VOID*, D3D11DDI_HANDLETYPE, _In_opt_ VOID*);
        static void APIENTRY ResizeTilePool(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT64);

        static void APIENTRY ShaderResourceViewReadAfterWriteHazard(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW, D3D10DDI_HRESOURCE);
        static void APIENTRY ResourceReadAfterWriteHazard(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE);

        static void APIENTRY SetHardwareProtection(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, INT);
        static void APIENTRY SetHardwareProtectionState(D3D10DDI_HDEVICE hDevice, BOOL);

        static void APIENTRY ExecuteCommandList(D3D10DDI_HDEVICE, D3D11DDI_HCOMMANDLIST) noexcept;

        static void APIENTRY SetShaderCache(D3D10DDI_HDEVICE hThis, D3DWDDM2_2DDI_HCACHESESSION hCache) { CastFrom(hThis)->m_pCurrentShaderCache = ShaderCache::CastFrom(hCache); }
        ShaderCache* GetShaderCache() const { return m_pCurrentShaderCache; }
        const D3DWDDM2_6DDI_CORELAYER_DEVICECALLBACKS *GetRuntimeCallbacks() const { return m_pCallbacks; }

        void ClearTrackedState();

        bool m_bMapDiscardCalled = false;

        // ID3D11On12DDIDevice interface:
        STDMETHOD_(void, SetMarker)(_In_opt_z_ const wchar_t* name) noexcept final;
        STDMETHOD_(void, BeginEvent)(_In_opt_z_ const wchar_t* name) noexcept final;
        STDMETHOD_(void, EndEvent)() noexcept final;


    protected:
        DDITableLatest *const m_pDDITable;
        const D3DWDDM2_6DDI_CORELAYER_DEVICECALLBACKS *const m_pCallbacks;
        const D3D10DDI_HRTCORELAYER m_hRTCoreLayer;

        Adapter *const m_pAdapter;

        D3D12TranslationLayer::BatchedContext m_BatchedContext;
        D3D12TranslationLayer::BatchedContext& m_PrimaryBatchedContext;

        PSODesc m_PSODesc;
        ComputePSOKey m_ComputePSODesc;

        D3D12TranslationLayer::COptLockedContainer<GraphicsPipelineStateCache>& m_GraphicsPSOCache;
        D3D12TranslationLayer::COptLockedContainer<ComputePipelineStateCache>& m_ComputePSOCache;

        D3D12TranslationLayer::PipelineState *m_pGraphicsPSO;
        D3D12TranslationLayer::PipelineState *m_pComputePSO;

        ShaderCache* m_pCurrentShaderCache = nullptr;

        // Dirty states are marked during sets and converted to command list operations at draw time, to avoid multiple costly conversions due to 11/12 API differences
        UINT m_DirtyStates = 0;
        UINT m_StatesToReassert = 0;

        std::unique_ptr<D3D12TranslationLayer::BatchedResource> m_spInterfaceBuffers[D3D12TranslationLayer::ShaderStageCount];
        bool m_bInterfaceBufferBound[D3D12TranslationLayer::ShaderStageCount] = {};
        bool m_bInterfaceBufferDirty[D3D12TranslationLayer::ShaderStageCount] = {};
        std::vector<UINT> m_InterfaceInstances[D3D12TranslationLayer::ShaderStageCount];
        std::vector<D3D11DDIARG_POINTERDATA> m_InterfacePointerData[D3D12TranslationLayer::ShaderStageCount];

        template<D3D12TranslationLayer::EShaderStage eShader>
        struct SSupplementalShaderTraits;
    };

    // The device object that underlies the D3D11 device and immediate context, and owns the D3D12TranslationLayer::ImmediateContext
    class Device : public DeviceBase
    {
    public:
        typedef DXGI1_6_1_DDI_BASE_FUNCTIONS DXGITableLatest;

        Device(Adapter* pAdapter, D3D10DDIARG_CREATEDEVICE* pArgs);
        ~Device();

        /* Start Backdoor funnctions used by D3D11 Runtime */
        static Device* CastFrom(D3D10DDI_HDEVICE h) noexcept { return reinterpret_cast<Device*>(h.pDrvPrivate); }
        static Device* CastFrom(DXGI_DDI_HDEVICE h) noexcept 
        { 
            return CastFrom(MAKE_D3D10DDI_HDEVICE(reinterpret_cast<void*>(h))); 
        }

        ID3D12CommandQueue* GetCommandQueue(D3D12TranslationLayer::COMMAND_LIST_TYPE commandListType)
        {
            return m_ImmediateContext.GetCommandQueue(commandListType);
        }

        // Returns true if synchronization was successful, false likely means device is removed
        bool WaitForCompletion(D3D12TranslationLayer::COMMAND_LIST_TYPE commandListType) noexcept
        {
            return FlushBatchAndGetImmediateContext().WaitForCompletion(commandListType);
        }

        bool WaitForCompletion(UINT commandListTypeMask) noexcept
        {
            return FlushBatchAndGetImmediateContext().WaitForCompletion(commandListTypeMask);
        }

        D3D12TranslationLayer::ImmediateContext &FlushBatchAndGetImmediateContext()
        {
            return m_BatchedContext.FlushBatchAndGetImmediateContext();
        }

        // ID3D11On12DDIDevice interface:
        STDMETHOD(GetD3D12Device)(REFIID riid, void** ppv) noexcept final;
        STDMETHOD(GetGraphicsQueue)(REFIID riid, void** ppv) noexcept final;
        STDMETHOD(EnqueueSetEvent)(_In_ HANDLE hEvent) noexcept final;
        STDMETHOD_(UINT, GetNodeMask)() noexcept final;
        STDMETHOD(Present)(_In_ D3DKMT_PRESENT* pArgs) noexcept override;
        STDMETHOD_(void, SetMaximumFrameLatency)(UINT MaxFrameLatency) noexcept override;
        STDMETHOD_(bool, IsMaximumFrameLatencyReached)() noexcept override;

        // Resource wrapping helpers
        STDMETHOD_(UINT, GetResourcePrivateDataSize)() noexcept final { return D3D12TranslationLayer::SharedResourceHelpers::cPrivateResourceDriverDataSize; }
        STDMETHOD(OpenSharedHandle)(_In_ HANDLE hSharedHandle, _Out_writes_bytes_(PrivateDriverDataSize) void* pPrivateDriverData, UINT PrivateDriverDataSize, _Out_ D3DKMT_HANDLE* hKMTHandle) noexcept final;
        STDMETHOD(CreateWrappingHandle)(_In_ IUnknown* pResource, WrapReason reason, _Out_writes_bytes_(PrivateDriverDataSize) void* pPrivateDriverData, UINT PrivateDriverDataSize, _Out_ D3DKMT_HANDLE* hKMTHandle) noexcept final;
        STDMETHOD(FillResourceInfo)(D3DKMT_HANDLE hKMTHandle, _In_ D3D11_RESOURCE_FLAGS const* pFlagOverrides, _Out_ ResourceInfo* pResourceInfo) noexcept final;
        STDMETHOD_(void, DestroyKMTHandle)(D3DKMT_HANDLE) noexcept final;
        STDMETHOD(AddResourceWaitsToQueue)(_In_ ID3D11On12DDIResource* p11on12DDIResource, _In_ ID3D12CommandQueue* pCommmandQueue) noexcept final;
        STDMETHOD(AddDeferredWaitsToResource)(_In_ ID3D11On12DDIResource* p11on12DDIResource, UINT NumSync, _In_reads_(NumSync) UINT64* pSignalValues, _In_reads_(NumSync) ID3D12Fence** ppFences) noexcept final;

        // Interop helpers
        STDMETHOD_(void, TransitionResourceForRelease)(_In_ ID3D11On12DDIResource* pResource, D3D12_RESOURCE_STATES State) noexcept final;
        STDMETHOD_(void, ApplyAllResourceTransitions)() noexcept final;

        // Fence helpers
        STDMETHOD(CreateFence)(UINT64 InitialValue, UINT DXGIInternalFenceFlags, _COM_Outptr_ ID3D11On12DDIFence** ppFence) noexcept final;
        STDMETHOD(OpenFence)(HANDLE hSharedFence, _Out_ bool* pbMonitored, _COM_Outptr_ ID3D11On12DDIFence** ppFence) noexcept final;
        STDMETHOD(Wait)(_In_ ID3D11On12DDIFence* pFence, UINT64 Value) noexcept final;
        STDMETHOD(Signal)(_In_ ID3D11On12DDIFence* pFence, UINT64 Value) noexcept final;

        // Shader creates which take the full containers instead of driver bytecode
        STDMETHOD(CreateVertexShader)(D3D10DDI_HSHADER hShader, _In_ SHADER_DESC const* pDesc) noexcept override;
        STDMETHOD(CreatePixelShader)(D3D10DDI_HSHADER hShader, _In_ SHADER_DESC const* pDesc) noexcept override;
        STDMETHOD(CreateGeometryShader)(D3D10DDI_HSHADER hShader, _In_ GEOMETRY_SHADER_DESC const* pDesc, _In_opt_ D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT const* pCGSWSOArgs) noexcept override;
        STDMETHOD(CreateHullShader)(D3D10DDI_HSHADER hShader, _In_ SHADER_DESC const* pDesc) noexcept override;
        STDMETHOD(CreateDomainShader)(D3D10DDI_HSHADER hShader, _In_ SHADER_DESC const* pDesc) noexcept override;
        STDMETHOD(CreateComputeShader)(D3D10DDI_HSHADER hShader, _In_ SHADER_DESC const* pDesc) noexcept override;

        STDMETHOD_(void, WriteToSubresource)(D3D10DDI_HRESOURCE hDstResource, UINT DstSubresource, _In_opt_ const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) noexcept final;
        STDMETHOD_(void, ReadFromSubresource)(void* pDstData, UINT DstRowPitch, UINT DstDepthPitch, D3D10DDI_HRESOURCE hSrcResource, UINT SrcSubresource, _In_opt_ const D3D11_BOX* pSrcBox) noexcept final;

        // Called in response to SetPrivateData(WKPDID_DXAnalysisPresentBoundary) on a resource or fence
        STDMETHOD_(void, SharingContractPresent)(_In_ ID3D11On12DDIResource* pResource) noexcept final;

        /* End Backdoor funnctions used by D3D11 Runtime */

        /* Start DDI Entry points*/

        // DDI Entry points

        static BOOL APIENTRY Flush(D3D10DDI_HDEVICE hDevice, UINT /*ContextType*/, UINT FlushFlags);
        static HRESULT APIENTRY RetrieveShaderComment(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader, _Out_writes_z_(*CharacterCountIncludingNullTerminator) WCHAR * pBuffer, _Inout_ SIZE_T * CharacterCountIncludingNullTerminator);

        static void APIENTRY MapUnderlyingSynchronize(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
        static void APIENTRY MapDefault(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
        static void APIENTRY UnmapDefault(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);
        static void APIENTRY UnmapUnderlyingStaging(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);

        static void APIENTRY GetMipPacking(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, _Out_ UINT*, _Out_ UINT*);

        static BOOL APIENTRY ResourceIsStagingBusy(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE);

        static void APIENTRY CheckFormatSupport(D3D10DDI_HDEVICE, DXGI_FORMAT, _Out_ UINT*);
        static void APIENTRY CheckMultisampleQualityLevels(D3D10DDI_HDEVICE, DXGI_FORMAT, UINT, UINT, _Out_ UINT*);
        static void APIENTRY CheckCounterInfo(D3D10DDI_HDEVICE, _Out_ D3D10DDI_COUNTER_INFO*);
        static void APIENTRY CheckCounter(D3D10DDI_HDEVICE hDevice, D3D10DDI_QUERY Query, _Out_ D3D10DDI_COUNTER_TYPE *pCounterType,
            _Out_ UINT *pActiveCounters, _Out_writes_opt_(*pNameLength) LPSTR pName, _Inout_opt_  UINT *pNameLength, _Out_writes_opt_(*pUnitsLength) LPSTR pUnits,
            _Inout_opt_  UINT *pUnitsLength, _Out_writes_opt_(*pDescriptionLength) LPSTR pDescription, _Inout_opt_  UINT *pDescriptionLength);

        static void APIENTRY DestroyDevice(D3D10DDI_HDEVICE);

        static SIZE_T APIENTRY CalcPrivateBlendStateSize(D3D10DDI_HDEVICE, _In_ CONST D3D11_1_DDI_BLEND_DESC*);
        static VOID APIENTRY CreateBlendState(D3D10DDI_HDEVICE, _In_ CONST D3D11_1_DDI_BLEND_DESC*, D3D10DDI_HBLENDSTATE, D3D10DDI_HRTBLENDSTATE);
        static VOID APIENTRY DestroyBlendState(D3D10DDI_HDEVICE, D3D10DDI_HBLENDSTATE);

        static SIZE_T APIENTRY CalcPrivateDepthStencilStateSize(D3D10DDI_HDEVICE, _In_ CONST D3D10_DDI_DEPTH_STENCIL_DESC*);
        static VOID APIENTRY CreateDepthStencilState(D3D10DDI_HDEVICE, _In_ CONST D3D10_DDI_DEPTH_STENCIL_DESC*, D3D10DDI_HDEPTHSTENCILSTATE, D3D10DDI_HRTDEPTHSTENCILSTATE);
        static VOID APIENTRY DestroyDepthStencilState(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILSTATE);

        static SIZE_T APIENTRY CalcPrivateRasterizerStateSize(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDI_RASTERIZER_DESC*);
        static VOID APIENTRY CreateRasterizerState(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDI_RASTERIZER_DESC*, D3D10DDI_HRASTERIZERSTATE, D3D10DDI_HRTRASTERIZERSTATE);
        static VOID APIENTRY DestroyRasterizerState(D3D10DDI_HDEVICE, D3D10DDI_HRASTERIZERSTATE);

        static SIZE_T APIENTRY CalcPrivateDepthStencilViewSize(D3D10DDI_HDEVICE, _In_ CONST D3D11DDIARG_CREATEDEPTHSTENCILVIEW*);
        static VOID APIENTRY CreateDepthStencilView(D3D10DDI_HDEVICE, _In_ CONST D3D11DDIARG_CREATEDEPTHSTENCILVIEW*, D3D10DDI_HDEPTHSTENCILVIEW, D3D10DDI_HRTDEPTHSTENCILVIEW);
        static VOID APIENTRY DestroyDepthStencilView(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILVIEW);

        static SIZE_T APIENTRY CalcPrivateRenderTargetViewSize(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATERENDERTARGETVIEW*);
        static VOID APIENTRY CreateRenderTargetView(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATERENDERTARGETVIEW*, D3D10DDI_HRENDERTARGETVIEW, D3D10DDI_HRTRENDERTARGETVIEW);
        static VOID APIENTRY DestroyRenderTargetView(D3D10DDI_HDEVICE, D3D10DDI_HRENDERTARGETVIEW);

        static SIZE_T APIENTRY CalcPrivateShaderResourceViewSize(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATESHADERRESOURCEVIEW*);
        static VOID APIENTRY CreateShaderResourceView(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATESHADERRESOURCEVIEW*, D3D10DDI_HSHADERRESOURCEVIEW, D3D10DDI_HRTSHADERRESOURCEVIEW);
        static VOID APIENTRY DestroyShaderResourceView(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW);

        static SIZE_T APIENTRY CalcPrivateUnorderedAccessViewSize(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATEUNORDEREDACCESSVIEW*);
        static VOID APIENTRY CreateUnorderedAccessView(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATEUNORDEREDACCESSVIEW*, D3D11DDI_HUNORDEREDACCESSVIEW, D3D11DDI_HRTUNORDEREDACCESSVIEW);
        static VOID APIENTRY DestroyUnorderedAccessView(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW);

        static SIZE_T APIENTRY CalcPrivateElementLayoutSize(D3D10DDI_HDEVICE, _In_ CONST D3D10DDIARG_CREATEELEMENTLAYOUT*);
        static VOID APIENTRY CreateElementLayout(D3D10DDI_HDEVICE, _In_ CONST D3D10DDIARG_CREATEELEMENTLAYOUT*, D3D10DDI_HELEMENTLAYOUT, D3D10DDI_HRTELEMENTLAYOUT);
        static VOID APIENTRY DestroyElementLayout(D3D10DDI_HDEVICE, D3D10DDI_HELEMENTLAYOUT);

        static SIZE_T APIENTRY CalcPrivateShaderSize(D3D10DDI_HDEVICE, _In_reads_(pShaderCode[1]) CONST UINT* pShaderCode, _In_ CONST D3D11_1DDIARG_STAGE_IO_SIGNATURES*);
        static SIZE_T APIENTRY CalcPrivateGeometryShaderWithStreamOutput(D3D10DDI_HDEVICE, _In_ CONST D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT*, _In_ CONST D3D11_1DDIARG_STAGE_IO_SIGNATURES*);
        static SIZE_T APIENTRY CalcPrivateTessellationShaderSize(D3D10DDI_HDEVICE, _In_reads_(pShaderCode[1]) CONST UINT* pShaderCode, _In_ CONST D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES*);
        static VOID APIENTRY DestroyShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);

        static void APIENTRY CheckDeferredContextHandleSizes(D3D10DDI_HDEVICE, UINT* pNumSizes, D3D11DDI_HANDLESIZE* pSizes) noexcept;
        static SIZE_T APIENTRY CalcDeferredContextHandleSize(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, void*) noexcept;
        static SIZE_T APIENTRY CalcPrivateDeferredContextSize(D3D10DDI_HDEVICE, D3D11DDIARG_CALCPRIVATEDEFERREDCONTEXTSIZE const*) noexcept;
        static void APIENTRY CreateDeferredContext(D3D10DDI_HDEVICE, D3D11DDIARG_CREATEDEFERREDCONTEXT const*) noexcept;

        static HRESULT APIENTRY Present1(DXGI1_6_1_DDI_ARG_PRESENT* pArgs);
        static HRESULT APIENTRY Blt(DXGI_DDI_ARG_BLT* pArgs);
        static HRESULT APIENTRY Blt1(DXGI_DDI_ARG_BLT1* pArgs);
        static HRESULT APIENTRY RotateResourceIdentities(DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* pArgs);
        static HRESULT APIENTRY ResolveSharedResource(DXGI_DDI_ARG_RESOLVESHAREDRESOURCE* pArgs);
        static HRESULT APIENTRY TrimResidencySet(DXGI_DDI_ARG_TRIMRESIDENCYSET* pArgs);
        static HRESULT APIENTRY SetResourcePriority(DXGI_DDI_ARG_SETRESOURCEPRIORITY* pArgs);
        static HRESULT APIENTRY CheckMultiplaneOverlayColorSpaceSupport(DXGI_DDI_ARG_CHECKMULTIPLANEOVERLAYCOLORSPACESUPPORT* pArgs);
        static HRESULT APIENTRY PresentMultiplaneOverlay1(DXGI1_6_1_DDI_ARG_PRESENTMULTIPLANEOVERLAY* pArgs);

        static HRESULT APIENTRY RetrieveSubObject(D3D10DDI_HDEVICE hDevice, UINT32 SubDeviceID, SIZE_T ParamSize, void *pParams, SIZE_T OutputParamSize, void *pOutputParamsBuffer);

        static void APIENTRY AcquireResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, HANDLE hSyncToken) noexcept;
        static void APIENTRY ReleaseResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, HANDLE hSyncToken) noexcept;

        static void APIENTRY QueryScanoutCaps(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT Subresource, D3DDDI_VIDEO_PRESENT_SOURCE_ID VidPnSourceId, UINT PlaneIdx, _Out_ D3DWDDM2_6DDI_SCANOUT_FLAGS* pFlags) noexcept;
        static void APIENTRY PrepareScanoutTransformation(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT Subresource, D3DDDI_VIDEO_PRESENT_SOURCE_ID VidPnSourceId, UINT PlaneIdx, _In_opt_ RECT* pRect) noexcept;


        void ReportError(HRESULT hr) noexcept;
        /* End DDI Entry points*/

        VideoDevice *GetVideoDevice();
        virtual HRESULT EnsureVideoDevice();

        D3D12TranslationLayer::ResourceAllocationContext GetResourceAllocationContext() const noexcept { return m_ResourceAllocationContext; }
        D3D10DDI_HRTDEVICE GetRTDeviceHandle() const { return m_hRTDevice; }

        D3D12TranslationLayer::SharedResourceHelpers& GetSharedResourceHelpers()
        {
            return m_SharedResourceHelpers;
        }

    protected:
        void PostSubmit();

        // Deferred context grabs references to these, but otherwise all access should be through the DeviceBase references.
        friend class DeviceDeferred;
        D3D12TranslationLayer::COptLockedContainer<GraphicsPipelineStateCache> m_GraphicsPSOCache;
        D3D12TranslationLayer::COptLockedContainer<ComputePipelineStateCache> m_ComputePSOCache;

        static HRESULT BltImpl(DXGI_DDI_ARG_BLT1* pArgs);

        // Helpers which allow immediate context to be constructed in initializer list.
        D3D12TranslationLayer::TranslationLayerCallbacks GetTranslationLayerCallbacks();
        D3D12TranslationLayer::BatchedContext::Callbacks GetBatchedContextCallbacks();

        DXGITableLatest *const m_pDXGITable;
        const DXGI_DDI_BASE_CALLBACKS *const m_pDXGICallbacks;
        const D3DDDI_DEVICECALLBACKS *const m_pKTCallbacks;
        const D3D10DDI_HRTDEVICE m_hRTDevice;

        const D3D12TranslationLayer::TranslationLayerCallbacks m_translationCallbacks;
        D3D12TranslationLayer::ImmediateContext m_ImmediateContext;
        D3D12TranslationLayer::SharedResourceHelpers m_SharedResourceHelpers;

        const D3D12TranslationLayer::ResourceAllocationContext m_ResourceAllocationContext;

        std::unique_ptr<VideoDevice> m_pVideoDevice;

        struct PresentExtensionData
        {
            Resource* pSrc;
            void* pDXGIContext;
        };
        PresentExtensionData const* m_pPresentArgs;

        struct PresentExtension : D3D12TranslationLayer::BatchedExtension
        {
            Device* const m_Device;
            PresentExtension(Device* pD) : m_Device(pD) {}
            void Dispatch(D3D12TranslationLayer::ImmediateContext&, const void* pData, size_t) final;
        };
        PresentExtension m_PresentExt = { this };
        std::mutex m_SwapChainManagerMutex;
        std::shared_ptr<class SwapChainManager> m_SwapChainManager;
        struct SyncTokenExtension : D3D12TranslationLayer::BatchedExtension
        {
            Device* const m_Device;
            PFND3DDDI_SYNCTOKENCB D3DDDI_DEVICECALLBACKS::* const m_pCallback;
            SyncTokenExtension(Device* pD, PFND3DDDI_SYNCTOKENCB D3DDDI_DEVICECALLBACKS::* pCB)
                : m_Device(pD), m_pCallback(pCB) { }
            void Dispatch(D3D12TranslationLayer::ImmediateContext& ImmCtx, const void* pData, size_t) final
            {
                ImmCtx.Flush(D3D12TranslationLayer::COMMAND_LIST_TYPE_ALL_MASK);
                D3DDDICB_SYNCTOKEN SyncTokenCB = {};
                SyncTokenCB.hSyncToken = *reinterpret_cast<HANDLE const*>(pData);
                (m_Device->m_pKTCallbacks->*m_pCallback)(m_Device->m_hRTDevice.handle, &SyncTokenCB);
            }
        };
        SyncTokenExtension m_AcquireResourceExt = { this, &D3DDDI_DEVICECALLBACKS::pfnAcquireResourceCb };
        SyncTokenExtension m_ReleaseResourceExt = { this, &D3DDDI_DEVICECALLBACKS::pfnReleaseResourceCb };
    };

    // Encapsulating these two function parameters into a struct, so it can be passed to the DeviceBase constructor as a single arg,
    // and then further used to tag dispatch different construction behavior, while having access to the base device.
    struct DeferredCtxArgs : D3D11DDIARG_CREATEDEFERREDCONTEXT
    {
        Device* pBaseDevice;
        DeferredCtxArgs(D3D11DDIARG_CREATEDEFERREDCONTEXT const* pBase, Device* pDevice) : D3D11DDIARG_CREATEDEFERREDCONTEXT(*pBase), pBaseDevice(pDevice) { }
    };

    // The "device" object that underlies a D3D11 deferred context. This owns a batched context (through the DeviceBase), which references
    // the parent Device, along with its immediate and batched contexts.
    class DeviceDeferred : public DeviceBase
    {
    public:
        DeviceDeferred(Device* pBaseDevice, D3D11DDIARG_CREATEDEFERREDCONTEXT const* pArgs);

        static DeviceDeferred* CastFrom(D3D10DDI_HDEVICE h) noexcept { return reinterpret_cast<DeviceDeferred*>(h.pDrvPrivate); }

        D3D12TranslationLayer::BatchedContext::Callbacks GetBatchedContextCallbacks();

        template<D3D12TranslationLayer::EShaderStage eShader>
        static void APIENTRY SetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER);

        template<D3D12TranslationLayer::EShaderStage eShader>
        static void APIENTRY SetShaderWithIfaces(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER, UINT NumClassInstances, __in_ecount(NumClassInstances) const UINT*, __in_ecount(NumClassInstances) const D3D11DDIARG_POINTERDATA*);

        template<D3D12TranslationLayer::EShaderStage ShaderUnit>
        static void APIENTRY SetShaderResources(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) UINT NumSRVs, _In_reads_(NumSRVs) const D3D10DDI_HSHADERRESOURCEVIEW* phSRVs);

        template<D3D12TranslationLayer::EShaderStage shaderUnit>
        static void APIENTRY SetSamplers(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) UINT NumSamplers, _In_reads_(NumSamplers) const D3D10DDI_HSAMPLER* phSamplers);

        static void APIENTRY SetBlendState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HBLENDSTATE hBlendState, const FLOAT BlendFactor[4], UINT SampleMask);
        static void APIENTRY SetDepthStencilState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILSTATE hDepthState, UINT StencilRef);
        static void APIENTRY SetRasterizerState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRASTERIZERSTATE hRasterizerState);

        static void APIENTRY IaSetInputLayout(D3D10DDI_HDEVICE hDevice, D3D10DDI_HELEMENTLAYOUT hInputLayout);

        static void APIENTRY OMSetRenderTargets(D3D10DDI_HDEVICE hDevice, CONST __in_ecount(NumRTVs) D3D10DDI_HRENDERTARGETVIEW* pRTVs, __in_range(0, 8) UINT NumRTVs, UINT, __in_opt D3D10DDI_HDEPTHSTENCILVIEW hDSV, CONST __in_ecount(NumUavs) D3D11DDI_HUNORDEREDACCESSVIEW* pUavs, CONST UINT* pInitialCounts, UINT UAVStartSlot, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumUavs, UINT, UINT);
        static void APIENTRY CsSetUnorderedAccessViews(D3D10DDI_HDEVICE hDevice, UINT Start, __in_range(0, D3D11_1_UAV_SLOT_COUNT) UINT NumViews, __in_ecount(NumViews) CONST D3D11DDI_HUNORDEREDACCESSVIEW* pUAVs, __in_ecount(NumViews) CONST UINT* pInitialCounts);

        static void APIENTRY ClearView(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, VOID*, const FLOAT[4], const D3D10_DDI_RECT*, UINT);
        static void APIENTRY ClearRenderTargetView(D3D10DDI_HDEVICE, D3D10DDI_HRENDERTARGETVIEW, FLOAT[4]);
        static void APIENTRY ClearDepthStencilView(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILVIEW, UINT, FLOAT, UINT8);
        static void APIENTRY ClearUnorderedAccessViewUint(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW, CONST UINT[4]);
        static void APIENTRY ClearUnorderedAccessViewFloat(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW, CONST FLOAT[4]);

        static void APIENTRY Discard(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, VOID*, const D3D10_DDI_RECT*, UINT);

        static void APIENTRY GenMips(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW);

        static void APIENTRY CopyStructureCount(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D11DDI_HUNORDEREDACCESSVIEW);

        static void APIENTRY TiledResourceBarrier(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, _In_opt_ VOID*, D3D11DDI_HANDLETYPE, _In_opt_ VOID*);
        
        static void APIENTRY SetShaderCache(D3D10DDI_HDEVICE hThis, D3DWDDM2_2DDI_HCACHESESSION hCache) { DeviceBase::SetShaderCache(hThis, GetImmediate(hCache)); }

        static HRESULT APIENTRY RecycleCreateDeferredContext(D3D10DDI_HDEVICE, D3D11DDIARG_CREATEDEFERREDCONTEXT const*) noexcept;

        static void APIENTRY DestroyDevice(D3D10DDI_HDEVICE);

        // ID3D11On12DDIDevice interface:
        STDMETHOD(GetD3D12Device)(REFIID, void**) noexcept final { return E_NOTIMPL; }
        STDMETHOD(GetGraphicsQueue)(REFIID, void**) noexcept final { return E_NOTIMPL; }
        STDMETHOD(EnqueueSetEvent)(HANDLE) noexcept final { return E_NOTIMPL; }
        STDMETHOD_(UINT, GetNodeMask)() noexcept final { return 0; }
        STDMETHOD(Present)(D3DKMT_PRESENT*) noexcept final { return E_NOTIMPL; }
        STDMETHOD_(void, SetMaximumFrameLatency)(UINT) noexcept final { }
        STDMETHOD_(bool, IsMaximumFrameLatencyReached)() noexcept final { return false; }

        // Resource wrapping helpers
        STDMETHOD_(UINT, GetResourcePrivateDataSize)() noexcept final { return UINT_MAX; }
        STDMETHOD(OpenSharedHandle)(HANDLE, void*, UINT, D3DKMT_HANDLE*) noexcept final { return E_NOTIMPL; }
        STDMETHOD(CreateWrappingHandle)(IUnknown*, WrapReason, void*, UINT, D3DKMT_HANDLE*) noexcept final { return E_NOTIMPL; }
        STDMETHOD(FillResourceInfo)(D3DKMT_HANDLE, D3D11_RESOURCE_FLAGS const*, ResourceInfo*) noexcept final { return E_NOTIMPL; }
        STDMETHOD_(void, DestroyKMTHandle)(D3DKMT_HANDLE) noexcept final { }
        STDMETHOD(AddResourceWaitsToQueue)(_In_ ID3D11On12DDIResource*, _In_ ID3D12CommandQueue*) noexcept final {return E_NOTIMPL; }
        STDMETHOD(AddDeferredWaitsToResource)(_In_ ID3D11On12DDIResource*, UINT, UINT64*, ID3D12Fence**) noexcept final { return E_NOTIMPL; }

        // Interop helpers
        STDMETHOD_(void, TransitionResourceForRelease)(ID3D11On12DDIResource*, D3D12_RESOURCE_STATES) noexcept final { }
        STDMETHOD_(void, ApplyAllResourceTransitions)() noexcept final { }

        // Fence helpers
        STDMETHOD(CreateFence)(UINT64, UINT, ID3D11On12DDIFence**) noexcept final { return E_NOTIMPL; }
        STDMETHOD(OpenFence)(HANDLE, bool*, ID3D11On12DDIFence**) noexcept final { return E_NOTIMPL; }
        STDMETHOD(Wait)(ID3D11On12DDIFence*, UINT64) noexcept final { return E_NOTIMPL; }
        STDMETHOD(Signal)(ID3D11On12DDIFence*, UINT64) noexcept final { return E_NOTIMPL; }

        // Shader creates which take the full containers instead of driver bytecode
        STDMETHOD(CreateVertexShader)(D3D10DDI_HSHADER, SHADER_DESC const*) noexcept final { return E_NOTIMPL; }
        STDMETHOD(CreatePixelShader)(D3D10DDI_HSHADER, SHADER_DESC const*) noexcept final { return E_NOTIMPL; }
        STDMETHOD(CreateGeometryShader)(D3D10DDI_HSHADER, GEOMETRY_SHADER_DESC const*, D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT const*) noexcept final { return E_NOTIMPL; }
        STDMETHOD(CreateHullShader)(D3D10DDI_HSHADER, SHADER_DESC const*) noexcept final { return E_NOTIMPL; }
        STDMETHOD(CreateDomainShader)(D3D10DDI_HSHADER, SHADER_DESC const*) noexcept final { return E_NOTIMPL; }
        STDMETHOD(CreateComputeShader)(D3D10DDI_HSHADER, SHADER_DESC const*) noexcept final { return E_NOTIMPL; }

        STDMETHOD_(void, WriteToSubresource)(D3D10DDI_HRESOURCE, UINT, const D3D11_BOX*, const void*, UINT, UINT) noexcept final { }
        STDMETHOD_(void, ReadFromSubresource)(void*, UINT, UINT, D3D10DDI_HRESOURCE, UINT, const D3D11_BOX*) noexcept final { }

        STDMETHOD_(void, SharingContractPresent)(_In_ ID3D11On12DDIResource*) noexcept final { }

        Device* const m_pBaseDevice;
    };

    class CommandList : public DeviceChild
    {
    public:
        static SIZE_T APIENTRY CalcPrivateSize(D3D10DDI_HDEVICE, D3D11DDIARG_CREATECOMMANDLIST const*);
        static void APIENTRY Create(D3D10DDI_HDEVICE, D3D11DDIARG_CREATECOMMANDLIST const*, D3D11DDI_HCOMMANDLIST, D3D11DDI_HRTCOMMANDLIST);
        static void APIENTRY Destroy(D3D10DDI_HDEVICE, D3D11DDI_HCOMMANDLIST);

        static void APIENTRY Recycle(D3D10DDI_HDEVICE, D3D11DDI_HCOMMANDLIST);
        static HRESULT APIENTRY RecycleCreate(D3D10DDI_HDEVICE, D3D11DDIARG_CREATECOMMANDLIST const*, D3D11DDI_HCOMMANDLIST, D3D11DDI_HRTCOMMANDLIST);

        CommandList(Device& parent) : DeviceChild(parent) {}
        static CommandList* CastFrom(D3D11DDI_HCOMMANDLIST h) { return static_cast<CommandList*>(h.pDrvPrivate); }

        std::unique_ptr<D3D12TranslationLayer::BatchedContext::Batch> m_CommandList;
        UINT m_ExecutionCount = 0;
        bool m_bMapDiscardCalled;
    };
};
