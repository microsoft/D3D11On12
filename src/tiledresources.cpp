// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void APIENTRY DeviceBase::UpdateTileMappings(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hTiledResource, UINT NumTiledResourceRegions, _In_reads_(NumTiledResourceRegions) const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE* pTiledResourceRegionStartCoords, _In_reads_opt_(NumTiledResourceRegions) const D3DWDDM1_3DDI_TILE_REGION_SIZE* pTiledResourceRegionSizes, D3D10DDI_HRESOURCE hTilePool, UINT NumRanges, _In_reads_opt_(NumRanges) const UINT* pRangeFlags, _In_reads_opt_(NumRanges) const UINT* pTilePoolStartOffsets, _In_reads_opt_(NumRanges) const UINT* pRangeTileCounts, UINT Flags)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().UpdateTileMappings(Resource::CastFromAndGetImmediateResource(hTiledResource), NumTiledResourceRegions, (D3D12_TILED_RESOURCE_COORDINATE*)pTiledResourceRegionStartCoords,
            (D3D12_TILE_REGION_SIZE*)pTiledResourceRegionSizes, Resource::CastFromAndGetImmediateResource(hTilePool),
            NumRanges, (D3D12TranslationLayer::ImmediateContext::TILE_RANGE_FLAG*)pRangeFlags, pTilePoolStartOffsets, pRangeTileCounts, (D3D12TranslationLayer::ImmediateContext::TILE_MAPPING_FLAG)Flags);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::CopyTileMappings(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hDstTiledResource, _In_ const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE* pDstStartCoords, D3D10DDI_HRESOURCE hSrcTiledResource, _In_ const  D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE* pSrcStartCoords, _In_ const D3DWDDM1_3DDI_TILE_REGION_SIZE* pTileRegion, UINT Flags)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().CopyTileMappings(Resource::CastFromAndGetImmediateResource(hDstTiledResource), (D3D12_TILED_RESOURCE_COORDINATE*)pDstStartCoords,
                                                      Resource::CastFromAndGetImmediateResource(hSrcTiledResource), (D3D12_TILED_RESOURCE_COORDINATE*)pSrcStartCoords, (D3D12_TILE_REGION_SIZE*)pTileRegion, (D3D12TranslationLayer::ImmediateContext::TILE_MAPPING_FLAG)Flags);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::CopyTiles(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hTiledResource, _In_ const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE* pStartCoords, _In_ const D3DWDDM1_3DDI_TILE_REGION_SIZE* pTileRegion, D3D10DDI_HRESOURCE hBuffer, UINT64 BufferOffset, UINT Flags)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().CopyTiles(Resource::CastFromAndGetImmediateResource(hTiledResource), (D3D12_TILED_RESOURCE_COORDINATE*)pStartCoords, (D3D12_TILE_REGION_SIZE*)pTileRegion,
                                               Resource::CastFromAndGetImmediateResource(hBuffer), BufferOffset, (D3D12TranslationLayer::ImmediateContext::TILE_COPY_FLAG)Flags);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::UpdateTiles(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hTiledResource, _In_ const D3DWDDM1_3DDI_TILED_RESOURCE_COORDINATE* pCoord, _In_ const D3DWDDM1_3DDI_TILE_REGION_SIZE* pRegion, const _In_ VOID* pData, UINT Flags)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().UpdateTiles(Resource::CastFromAndGetImmediateResource(hTiledResource), (D3D12_TILED_RESOURCE_COORDINATE*)pCoord, (D3D12_TILE_REGION_SIZE*)pRegion, pData, Flags);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::TiledResourceBarrier(D3D10DDI_HDEVICE hDevice, D3D11DDI_HANDLETYPE beforeType, _In_opt_ VOID* pBefore, D3D11DDI_HANDLETYPE afterType, _In_opt_ VOID* pAfter)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        // Note: only supports: RTV, UAV, DSV, SRV, resource
        auto pfnGetResource = [=](D3D11DDI_HANDLETYPE handleType, VOID* pHandle) -> D3D12TranslationLayer::Resource*
        {
            assert(pHandle || handleType == D3D10DDI_HT_RESOURCE);
            switch (handleType)
            {
                case D3D10DDI_HT_RENDERTARGETVIEW:
                {
                    D3D12TranslationLayer::ViewBase* ViewBase = RenderTargetView::CastFrom(MAKE_D3D10DDI_HRENDERTARGETVIEW(pHandle))->GetTranslationLayerView();
                    return ViewBase->m_pResource;
                }
                case D3D10DDI_HT_SHADERRESOURCEVIEW:
                {
                    D3D12TranslationLayer::ViewBase* ViewBase = ShaderResourceView::CastFrom(MAKE_D3D10DDI_HSHADERRESOURCEVIEW(pHandle))->GetTranslationLayerView();
                    return ViewBase->m_pResource;
                }
                case D3D10DDI_HT_DEPTHSTENCILVIEW:
                {
                    D3D12TranslationLayer::ViewBase* ViewBase = DepthStencilView::CastFrom(MAKE_D3D10DDI_HDEPTHSTENCILVIEW(pHandle))->GetTranslationLayerView();
                    return ViewBase->m_pResource;
                }
                case D3D11DDI_HT_UNORDEREDACCESSVIEW:
                {
                    D3D12TranslationLayer::ViewBase* ViewBase = UnorderedAccessView::CastFrom(MAKE_D3D11DDI_HUNORDEREDACCESSVIEW(pHandle))->GetTranslationLayerView();
                    return ViewBase->m_pResource;
                }
                case D3D10DDI_HT_RESOURCE:
                {
                    return Resource::CastFromAndGetImmediateResource(MAKE_D3D10DDI_HRESOURCE(pHandle));
                }
                default: assert(false);
            }
            return nullptr;
        };

        pDevice->GetBatchedContext().TiledResourceBarrier(pfnGetResource(beforeType, pBefore), pfnGetResource(afterType, pAfter));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::TiledResourceBarrier(D3D10DDI_HDEVICE hDevice, D3D11DDI_HANDLETYPE beforeType, _In_opt_ VOID* pBefore, D3D11DDI_HANDLETYPE afterType, _In_opt_ VOID* pAfter)
    {
        void* pTranslatedBefore = pBefore, *pTranslatedAfter = pAfter;
        if (beforeType != D3D10DDI_HT_RESOURCE)
        {
            pTranslatedBefore = GetImmediate(MAKE_D3D10DDI_HRENDERTARGETVIEW(pBefore)).pDrvPrivate;
        }
        if (afterType != D3D10DDI_HT_RESOURCE)
        {
            pTranslatedAfter = GetImmediate(MAKE_D3D10DDI_HRENDERTARGETVIEW(pAfter)).pDrvPrivate;
        }
        DeviceBase::TiledResourceBarrier(hDevice, beforeType, pTranslatedBefore, afterType, pTranslatedAfter);
    }

    void APIENTRY DeviceBase::ResizeTilePool(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hTilePool, UINT64 NewSize)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->GetBatchedContext().ResizeTilePool(Resource::CastFromAndGetImmediateResource(hTilePool), NewSize);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}