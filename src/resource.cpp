// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    SIZE_T APIENTRY Resource::CalcPrivateResourceSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11DDIARG_CREATERESOURCE* /*pCreateResource*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T resourceSize = sizeof(Resource);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, resourceSize);
    }

    SIZE_T APIENTRY Resource::CalcPrivateOpenedResourceSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10DDIARG_OPENRESOURCE* /*pOpenResource*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T resourceSize = sizeof(Resource);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, resourceSize);
    }

    UINT Resource::ModifyFlagsForVideoSupport(
        D3D12TranslationLayer::ImmediateContext& context,
        UINT BindFlags, 
        DXGI_FORMAT Format
        )
    {
        // D3D11 allowed the render target flag when a format supported video process 
        // input or output, even if it did not support using the resource as a render target.
        // D3D12 doesn't overload render target flags in the same way and fails if render target
        // is specified on a format that supports it.
        // D3D12 is a superset of D3D11, so simply remove the render target flag if D3D12 doesn't
        // support it for the format.  
        // One example format that excercies this is DXGI_FORMAT_YUY2.
        if (BindFlags & D3D11_BIND_RENDER_TARGET)
        {
            if (!context.SupportsRenderTarget(Format))
            {
                // Check if the format is TYPELESS.  If it is, render target is allowed if any format in its cast set
                // allows render target.
                if (CD3D11FormatHelper::GetParentFormat(Format) == Format)
                {
                    const DXGI_FORMAT* pFormatCastSet = CD3D11FormatHelper::GetFormatCastSet(Format);

                    for (UINT i = 0; pFormatCastSet[i] != DXGI_FORMAT_UNKNOWN; ++i)
                    {
                        if (context.SupportsRenderTarget(pFormatCastSet[i]))
                        {
                            return BindFlags;
                        }
                    }
                }
               
                BindFlags &= ~D3D11_BIND_RENDER_TARGET;
            }
        }

        return BindFlags;
    }

    inline void PropagateToAppResourceDesc(UINT Usage, D3D12TranslationLayer::ResourceCreationArgs& createArgs)
    {
        auto const& Desc12 = createArgs.m_desc12;
        UINT16 ArraySize = (Desc12.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? UINT16(1) : Desc12.DepthOrArraySize;
        UINT16 Depth =     (Desc12.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? UINT16(1) : Desc12.DepthOrArraySize;
        createArgs.m_appDesc = D3D12TranslationLayer::AppResourceDesc(
            Desc12.MipLevels * ArraySize,
            static_cast<UINT8>(CD3D11FormatHelper::NonOpaquePlaneCount(Desc12.Format)),
            Desc12.MipLevels * ArraySize * CD3D11FormatHelper::NonOpaquePlaneCount( Desc12.Format ),
            static_cast<UINT8>(Desc12.MipLevels),
            ArraySize,
            Depth,
            static_cast<UINT>(Desc12.Width),
            Desc12.Height,
            Desc12.Format,
            Desc12.SampleDesc.Count,
            Desc12.SampleDesc.Quality,
            static_cast<D3D12TranslationLayer::RESOURCE_USAGE>(Usage),
            static_cast<D3D12TranslationLayer::RESOURCE_CPU_ACCESS>(createArgs.m_flags11.CPUAccessFlags),
            static_cast<D3D12TranslationLayer::RESOURCE_BIND_FLAGS>(createArgs.m_flags11.BindFlags),
            Desc12.Dimension);
    }

    VOID APIENTRY Resource::CreateResource(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11DDIARG_CREATERESOURCE* pCreateResource, D3D10DDI_HRESOURCE hResource, D3D10DDI_HRTRESOURCE /*hRTResource*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        D3D12TranslationLayer::ImmediateContext& context = pDevice->GetImmediateContextNoFlush();

        D3D12TranslationLayer::ResourceCreationArgs createArgs = {};

        // The translation layer can only do residency management with a constrained amount of threads.
        // However, all initial data upload goes through the batching layer, so there is no multi-threaded
        // access, so we are safe to enable residency management from the start.
        createArgs.m_bManageResidency = true;
        createArgs.m_flags11 = pDevice->GetAdapter()->m_Callbacks.GetResourceFlags(hResource, &createArgs.m_bTriggerDeferredWaits);

        // 11 might have hidden some flags from the API desc: get them now.
        createArgs.m_flags11.BindFlags |=
            ((pCreateResource->BindFlags & D3D10_DDI_BIND_RENDER_TARGET) ? D3D11_BIND_RENDER_TARGET : 0) |
            ((pCreateResource->BindFlags & D3D10_DDI_BIND_SHADER_RESOURCE) ? D3D11_BIND_SHADER_RESOURCE : 0) |
            ((pCreateResource->BindFlags & D3D11_DDI_BIND_UNORDERED_ACCESS) ? D3D11_BIND_UNORDERED_ACCESS : 0);

        createArgs.m_desc12.Width = pCreateResource->pMipInfoList->TexelWidth;
        createArgs.m_desc12.Height = pCreateResource->pMipInfoList->TexelHeight;
        createArgs.m_desc12.DepthOrArraySize = static_cast<UINT16>(
            pCreateResource->ResourceDimension == D3D10DDIRESOURCE_TEXTURE3D ?
                pCreateResource->pMipInfoList->TexelDepth : pCreateResource->ArraySize);
        createArgs.m_desc12.Format = pCreateResource->Format;
        createArgs.m_desc12.MipLevels = static_cast<UINT16>(pCreateResource->MipLevels);
        createArgs.m_desc12.SampleDesc = pCreateResource->SampleDesc;

        if (pCreateResource->ResourceDimension != D3D10DDIRESOURCE_BUFFER)
        {
            createArgs.m_flags11.BindFlags = ModifyFlagsForVideoSupport(context, createArgs.m_flags11.BindFlags, pCreateResource->Format);
            createArgs.m_desc12.Layout = (createArgs.m_flags11.MiscFlags & D3D11_RESOURCE_MISC_TILED) ?
                D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE : D3D12_TEXTURE_LAYOUT_UNKNOWN;
        }
        createArgs.m_desc12.Flags = CD3DX11ON12_RESOURCE_DESC_FROM11::ConvertD3D11Flags(createArgs.m_flags11.BindFlags);

        if (pDevice->GetAdapter()->m_bComputeOnly && pCreateResource->Usage == D3D10_DDI_USAGE_DYNAMIC)
        {
            // Dynamic resources will use dynamically-allocated upload resources for data upload.
            createArgs.m_heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        }
        else
        {
            createArgs.m_heapDesc.Properties = CD3DX11ON12_HEAP_PROPERTIES_FROM11::ConvertD3D11Usage(
                static_cast<D3D11_USAGE>(pCreateResource->Usage),
                createArgs.m_flags11.CPUAccessFlags);
        }
        createArgs.m_heapDesc.Flags = CD3DX11ON12_HEAP_DESC_FROM11::ConvertD3D11MiscFlags(createArgs.m_flags11.MiscFlags);

        if (pCreateResource->pPrimaryDesc)
        {
            createArgs.m_heapDesc.Flags |= D3D12_HEAP_FLAG_ALLOW_DISPLAY;
        }

        if (pDevice->GetImmediateContextNoFlush().IsResidencyManagementEnabled())
        {
            createArgs.m_heapDesc.Flags |= D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT;
        }

        switch (pCreateResource->ResourceDimension)
        {
            case D3D10DDIRESOURCE_BUFFER:
            {
                createArgs.m_desc12.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                createArgs.m_desc12.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

                // Only used for tile pools, which are always buffers
                createArgs.m_heapDesc.SizeInBytes = createArgs.m_desc12.Width;
            } break;
            case D3D10DDIRESOURCE_TEXTURE1D:
                createArgs.m_desc12.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                break;
            case D3D10DDIRESOURCE_TEXTURE2D:
            case D3D10DDIRESOURCE_TEXTURECUBE:
            {
                createArgs.m_desc12.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

                if (pDevice->GetAdapter()->m_bSupportDisplayableTextures)
                {
                    if (createArgs.m_desc12.Format == DXGI_FORMAT_420_OPAQUE)
                    {
                        // 420_OPAQUE doesn't exist in D3D12.
                        createArgs.m_desc12.Format = DXGI_FORMAT_NV12; 
                    }

#define D3D11_RESOURCE_MISC_SHARED_DISPLAYABLE static_cast<D3D11_RESOURCE_MISC_FLAG>(0x100000)
#define D3D11_RESOURCE_MISC_SHARED_EXCLUSIVE_WRITER static_cast<D3D11_RESOURCE_MISC_FLAG>(0x200000)
                    if (createArgs.m_flags11.MiscFlags & D3D11_RESOURCE_MISC_SHARED_DISPLAYABLE)
                    {
                        createArgs.m_heapDesc.Flags |= D3D12_HEAP_FLAG_ALLOW_DISPLAY;
                        if (!(createArgs.m_flags11.MiscFlags & D3D11_RESOURCE_MISC_SHARED_EXCLUSIVE_WRITER))
                        {
                            createArgs.m_desc12.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
                        }
                    }
                }
                else if (createArgs.m_flags11.BindFlags & D3D11_BIND_DECODER)
                {
                    createArgs.m_desc12.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
                }
            } break;
            case D3D10DDIRESOURCE_TEXTURE3D:
                createArgs.m_desc12.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                break;
            default: ASSUME(false);
        }

        if (pCreateResource->MiscFlags & D3DWDDM1_3DDI_RESOURCE_MISC_CROSS_ADAPTER)
        {
            createArgs.m_heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE, D3D12_MEMORY_POOL_L0);
            createArgs.m_heapDesc.Flags |= D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;
            createArgs.m_desc12.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            createArgs.m_desc12.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
            assert(createArgs.m_flags11.MiscFlags & D3D11_RESOURCE_MISC_SHARED_NTHANDLE);

            if (!pDevice->GetAdapter()->m_Caps.CrossAdapterRowMajorTextureSupported)
            {
                createArgs.m_desc12.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
                createArgs.m_flags11.BindFlags &= ~(D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);
            }
        }

        PropagateToAppResourceDesc(pCreateResource->Usage, createArgs);
        ConvertUnderlyingTextureCreationDesc(createArgs, *pDevice, *pCreateResource);

        // After app resource desc is snapped
        if (pCreateResource->ResourceDimension == D3D10DDIRESOURCE_BUFFER)
        {
            // Add extra bytes for BufferFilledSize & Indirect Arguments
            if (createArgs.m_flags11.BindFlags & D3D11_BIND_STREAM_OUTPUT)
            {
                createArgs.m_bBoundForStreamOut = true;

                // Align size to be a multiple of UINT size
                const UINT AlignedByteWidth = (createArgs.m_desc12.Width + 3) & ~3;
                assert(0 == (AlignedByteWidth % 4));

                const UINT FinalByteWidth = AlignedByteWidth + sizeof(D3D12TranslationLayer::SStreamOutputSuffix);

                createArgs.m_OffsetToStreamOutputSuffix = AlignedByteWidth;

                assert((FinalByteWidth - createArgs.m_desc12.Width) >= sizeof(D3D12TranslationLayer::SStreamOutputSuffix));

                createArgs.m_desc12.Width = FinalByteWidth;

                // Part of the suffix will be written to with a UAV
                // and then read via DrawIndirect
                createArgs.m_flags11.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                createArgs.m_flags11.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
                createArgs.m_desc12.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }
            else if (createArgs.m_flags11.BindFlags & D3D11_BIND_CONSTANT_BUFFER)
            {
                // Align size to be a multiple of constant buffer alignment
                createArgs.m_desc12.Width = Align<UINT64>(createArgs.m_desc12.Width, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
            }
        }

        auto CreateFunc = [&]()
        {
            return D3D12TranslationLayer::Resource::CreateResource(
                &pDevice->GetImmediateContextNoFlush(), createArgs, pDevice->GetResourceAllocationContext()); // throw( bad_alloc )
        };
        auto CreateAndRetry = [&]()
        {
            try
            {
                return CreateFunc();
            }
            catch (_com_error&)
            {
                if (pDevice->GetBatchedContext().ProcessBatch())
                {
                    return CreateFunc();
                }
                throw;
            }
        };
        auto spUnderlyingResource = CreateAndRetry();

        // Check if we're supposed to be creating a shared resource
        {
            D3D12TranslationLayer::Resource::SResourceIdentity* pIdentity = spUnderlyingResource->GetIdentity();
            if (pIdentity->m_bOwnsUnderlyingResource)
            {
                pIdentity->m_bSharedResource = pDevice->GetAdapter()->m_Callbacks.NotifySharedResourceCreation(
                    pDevice->GetRTDeviceHandle().handle,
                    spUnderlyingResource->GetUnderlyingResource());
            }
        }

        // Handle initial data upload and constant buffer bloating
        {

            // Handle initial data uploads
            bool bIsDefaultResourceBloated = spUnderlyingResource->IsDefaultResourceBloated();
            if (pCreateResource->pInitialDataUP || bIsDefaultResourceBloated)
            {
                // Initial data upload should never be predicated
                D3D12TranslationLayer::ImmediateContext::CDisablePredication DisablePredication(context.CreatesAndDestroysAreMultithreaded() ? nullptr : &context);
                if (pCreateResource->pInitialDataUP)
                {
                    pDevice->GetBatchedContext().UploadInitialData(
                        spUnderlyingResource.get(),
                        spUnderlyingResource->GetFullSubresourceSubset(),
                        reinterpret_cast<const D3D11_SUBRESOURCE_DATA*>(pCreateResource->pInitialDataUP),
                        nullptr);
                }
                if (bIsDefaultResourceBloated)
                {
                    UINT BloatOffset = spUnderlyingResource->GetOffsetToStreamOutputSuffix();
                    UINT BloatSize = sizeof(UINT);
                    if (!BloatOffset)
                    {
                        assert(pCreateResource->BindFlags & D3D10_DDI_BIND_CONSTANT_BUFFER);
                        BloatOffset = createArgs.m_appDesc.Width();
                        BloatSize =
                            Align<UINT>(BloatOffset, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) - BloatOffset;
                    }

                    D3D12_BOX DstBox = { BloatOffset, 0, 0,
                        BloatOffset + BloatSize, 1, 1 };
                    pDevice->GetBatchedContext().UploadInitialData(
                        spUnderlyingResource.get(),
                        D3D12TranslationLayer::CSubresourceSubset(D3D12TranslationLayer::CBufferView()),
                        nullptr,
                        &DstBox);
                }
            }

            spUnderlyingResource->ZeroConstantBufferPadding();
        }

        // No more exceptions
        auto pResource = new (hResource.pDrvPrivate) Resource(*pDevice, spUnderlyingResource.release(), true /*ownsResource*/);
        if (pCreateResource->pPrimaryDesc)
        {
            pResource->m_VidPnSourceId = pCreateResource->pPrimaryDesc->VidPnSourceId;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Resource::OpenResource(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10DDIARG_OPENRESOURCE* pOpenResource, D3D10DDI_HRESOURCE hResource, D3D10DDI_HRTRESOURCE /*hRTResource*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        D3D12TranslationLayer::ResourceCreationArgs createArgs = {};

        createArgs.m_flags11 = pDevice->GetAdapter()->m_Callbacks.GetResourceFlags(hResource, &createArgs.m_bTriggerDeferredWaits);
        
        D3D12TranslationLayer::ResourceInfo resInfo;
        pDevice->GetSharedResourceHelpers().QueryResourceInfoFromKMTHandle(pOpenResource->hKMResource.handle, nullptr, &resInfo);
        if (resInfo.m_Type == D3D12TranslationLayer::TiledPoolType)
        {
            createArgs.m_heapDesc = resInfo.TiledPool.m_HeapDesc;
            createArgs.m_desc12 = CD3DX12_RESOURCE_DESC::Buffer(resInfo.TiledPool.m_HeapDesc.SizeInBytes);
        }
        else
        {
            createArgs.m_desc12 = resInfo.Resource.m_ResourceDesc;
            createArgs.m_heapDesc = CD3DX12_HEAP_DESC(0, resInfo.Resource.m_HeapProps, 0, resInfo.Resource.m_HeapFlags);
        }

        PropagateToAppResourceDesc(D3D11_USAGE_DEFAULT, createArgs);

        auto spUnderlyingResource = pDevice->GetSharedResourceHelpers().OpenResourceFromKmtHandle(
            createArgs,
            pOpenResource->hKMResource.handle,
            pOpenResource->pPrivateDriverData,
            pOpenResource->PrivateDriverDataSize,
            D3D12_RESOURCE_STATE_COMMON);

        // No more exceptions
        new (hResource.pDrvPrivate) Resource(*pDevice, spUnderlyingResource.release(), true /*ownsResource*/);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Resource::CreateDeferredResource(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11DDIARG_CREATERESOURCE*, D3D10DDI_HRESOURCE hResource, D3D10DDI_HRTRESOURCE hRTResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = DeviceDeferred::CastFrom(hDevice);
        Resource* pBaseResource = Resource::CastFrom(MAKE_D3D10DDI_HRESOURCE(hRTResource.handle));
        new (hResource.pDrvPrivate) Resource(*pDevice->m_pBaseDevice, pBaseResource->ImmediateResource(), false /*ownsResource*/);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Resource::DestroyResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Resource* pResource = Resource::CastFrom(hResource);
        pResource->~Resource();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::ResourceCopy(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hDst, D3D10DDI_HRESOURCE hSrc)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().ResourceCopy(Resource::CastFromAndGetImmediateResource(hDst), Resource::CastFromAndGetImmediateResource(hSrc));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::ResourceResolveSubresource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hDst, UINT DstSubresource, D3D10DDI_HRESOURCE hSrc, UINT SrcSubresource, DXGI_FORMAT Format)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().ResourceResolveSubresource(Resource::CastFromAndGetImmediateResource(hDst), DstSubresource, Resource::CastFromAndGetImmediateResource(hSrc), SrcSubresource, Format);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::SetResourceMinLOD(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, FLOAT MinLOD)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().SetResourceMinLOD(Resource::CastFromAndGetImmediateResource(hResource), MinLOD);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::CopyStructureCount(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT DstAlignedByteOffset, D3D11DDI_HUNORDEREDACCESSVIEW hUAV)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().CopyStructureCount(Resource::CastFromAndGetImmediateResource(hResource), DstAlignedByteOffset, UnorderedAccessView::CastToTranslationView(hUAV));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::CopyStructureCount(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT DstAlignedByteOffset, D3D11DDI_HUNORDEREDACCESSVIEW hUAV)
    {
        DeviceBase::CopyStructureCount(hDevice, hResource, DstAlignedByteOffset, GetImmediate(hUAV));
    }

    void APIENTRY DeviceBase::ResourceCopyRegion(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hDst, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, D3D10DDI_HRESOURCE hSrc, UINT SrcSubresource, const D3D10_DDI_BOX* pSrcBox, UINT /*Flags*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().ResourceCopyRegion(Resource::CastFromAndGetImmediateResource(hDst), DstSubresource, DstX, DstY, DstZ,
                                                        Resource::CastFromAndGetImmediateResource(hSrc), SrcSubresource, (D3D12_BOX*)pSrcBox);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::ResourceUpdateSubresourceUP(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hDst, UINT DstSubresource, _In_opt_ const D3D10_DDI_BOX* pDstBox, _In_ const VOID* pMem, UINT SrcPitch, UINT SrcDepth, UINT /*Flags*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        try
        {
            pDevice->GetBatchedContext().ResourceUpdateSubresourceUP(Resource::CastFromAndGetImmediateResource(hDst), DstSubresource, (D3D12_BOX*)pDstBox, pMem, SrcPitch, SrcDepth);
        }
        catch (_com_error& err)
        {
            if (err.Error() == E_OUTOFMEMORY)
            {
                //We ran out of memory and weren't able to free up enough to complete the operation.
                //flush the batch to sync the worker thread so that we can be more aggressive in how we free up memory.
                D3D12TranslationLayer::ImmediateContext& immCtx = pDevice->GetBatchedContext().FlushBatchAndGetImmediateContext();
                immCtx.ResourceUpdateSubresourceUP(Resource::CastFromAndGetImmediateResource(hDst), DstSubresource, (D3D12_BOX*)pDstBox, pMem, SrcPitch, SrcDepth);
            }
            else
            {
                throw;
            }
            
        }
        
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }


    void APIENTRY Device::GetMipPacking(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, _Out_ UINT* pNumPackedMips, _Out_ UINT* pNumTilesForPackedMips)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetImmediateContextNoFlush().GetMipPacking(Resource::CastFromAndGetImmediateResource(hResource), pNumPackedMips, pNumTilesForPackedMips);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::ShaderResourceViewReadAfterWriteHazard(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW, D3D10DDI_HRESOURCE)
    {
    }

    void APIENTRY DeviceBase::ResourceReadAfterWriteHazard(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE)
    {
    }

    BOOL APIENTRY Device::ResourceIsStagingBusy(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE)
    {
        return FALSE;
    }

    STDMETHODIMP_(ID3D12Resource*) Resource::GetUnderlyingResource() noexcept
    {
        return ImmediateResource()->GetUnderlyingResource();
    }

    STDMETHODIMP_(void) Resource::SetGraphicsCurrentState(D3D12_RESOURCE_STATES State, SetStateReason Reason) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D12TranslationLayer::CCurrentResourceState::ExclusiveState ExclusiveState = {};

        if (Reason == SetStateReason::Create)
        {
            ExclusiveState.FenceValue = 0;
        }
        else
        {
            assert(Reason == SetStateReason::Acquire);
            ExclusiveState.FenceValue = m_parentDevice.FlushBatchAndGetImmediateContext().GetCommandListID(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS);
            m_parentDevice.GetImmediateContextNoFlush().GetCommandListManager(D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS)->SetNeedSubmitFence();
        }
        
        ExclusiveState.CommandListType = D3D12TranslationLayer::COMMAND_LIST_TYPE::GRAPHICS;
        ExclusiveState.State = State;
        ImmediateResource()->GetIdentity()->m_currentState.SetExclusiveResourceState(ExclusiveState);
        CLOSE_TRYCATCH_AND_STORE_HRESULT(S_OK);
        assert(SUCCEEDED(EntryPointHr));
    }
    STDMETHODIMP Resource::CreateSharedHandle(_In_opt_ const SECURITY_ATTRIBUTES *pAttributes, _In_ DWORD dwAccess, _In_opt_ LPCWSTR lpName, _Out_ HANDLE *pHandle) noexcept
    {
        return m_parentDevice.GetImmediateContextNoFlush().m_pDevice12->CreateSharedHandle(GetUnderlyingResource(), pAttributes, dwAccess, lpName, pHandle);
    }
};
