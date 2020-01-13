// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    static inline D3D12TranslationLayer::MAP_TYPE ConvertMapType(D3D10_DDI_MAP ddiType)
    {
        switch (ddiType)
        {
        case D3D10_DDI_MAP_READ:
            return D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READ;
        case D3D10_DDI_MAP_WRITE:
            return D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_WRITE;
        case D3D10_DDI_MAP_READWRITE:
            return D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READWRITE;
        case D3D10_DDI_MAP_WRITE_DISCARD:
            return D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_WRITE_DISCARD;
        case D3D10_DDI_MAP_WRITE_NOOVERWRITE:
            return D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_WRITE_NOOVERWRITE;
        default:
            assert(false);
            return D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READ;
        }
    }

    static inline bool DoNotWait(UINT MapFlags)
    {
        return (MapFlags & D3D10_DDI_MAP_FLAG_DONOTWAIT) ? true : false;
    }

    void APIENTRY DeviceBase::MapRenamedBuffer(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource, D3D10_DDI_MAP MapType, UINT /* MapFlags */, D3D10DDI_MAPPED_SUBRESOURCE* pMappedResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);
        assert(Subresource == 0 && MapType == D3D10_DDI_MAP_WRITE_NOOVERWRITE);
        UNREFERENCED_PARAMETER(Subresource);
        UNREFERENCED_PARAMETER(MapType);

        HRESULT hr = SynchronizedResultToHRESULT(
            pDevice->GetBatchedContext().MapRenamedBuffer(&pResource->BatchedResource(), (D3D12TranslationLayer::MappedSubresource*)pMappedResource));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    void APIENTRY Device::MapUnderlyingSynchronize(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource, D3D10_DDI_MAP MapType, UINT MapFlags, D3D10DDI_MAPPED_SUBRESOURCE* pMappedResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);

        HRESULT hr = SynchronizedResultToHRESULT(
            pDevice->GetBatchedContext().MapUnderlyingSynchronize(&pResource->BatchedResource(), Subresource, ConvertMapType(MapType), DoNotWait(MapFlags), nullptr, (D3D12TranslationLayer::MappedSubresource*)pMappedResource));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    void APIENTRY DeviceBase::RenameAndMapBuffer(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource, D3D10_DDI_MAP MapType, UINT /* MapFlags */, D3D10DDI_MAPPED_SUBRESOURCE* pMappedResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        pDevice->m_bMapDiscardCalled = true;

        Resource* pResource = Resource::CastFrom(hResource);
        assert(Subresource == 0 && MapType == D3D10_DDI_MAP_WRITE_DISCARD);
        UNREFERENCED_PARAMETER(Subresource);
        UNREFERENCED_PARAMETER(MapType);

        HRESULT hr = SynchronizedResultToHRESULT(
            pDevice->GetBatchedContext().RenameAndMapBuffer(&pResource->BatchedResource(), (D3D12TranslationLayer::MappedSubresource*)pMappedResource));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    void APIENTRY DeviceBase::MapDiscardResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource, D3D10_DDI_MAP MapType, UINT /* MapFlags */, D3D10DDI_MAPPED_SUBRESOURCE* pMappedResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);
        assert(MapType == D3D10_DDI_MAP_WRITE_DISCARD);

        HRESULT hr = S_OK;
        if (pResource->ImmediateResource()->AppDesc()->Usage() == D3D12TranslationLayer::RESOURCE_USAGE_DYNAMIC  &&
            pResource->ImmediateResource()->AppDesc()->ResourceDimension() == D3D12_RESOURCE_DIMENSION_BUFFER  &&
            MapType == D3D10_DDI_MAP_WRITE_DISCARD)
        {
            pDevice->m_bMapDiscardCalled = true;
            assert(Subresource == 0);
            hr = SynchronizedResultToHRESULT(
                pDevice->GetBatchedContext().RenameAndMapBuffer(&pResource->BatchedResource(), (D3D12TranslationLayer::MappedSubresource*)pMappedResource));
        }
        else
        {
            hr = SynchronizedResultToHRESULT(
                pDevice->GetBatchedContext().MapForRenameViaCopy(&pResource->BatchedResource(), Subresource, (D3D12TranslationLayer::MappedSubresource*)pMappedResource));
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    void APIENTRY Device::MapDefault(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource, D3D10_DDI_MAP MapType, UINT MapFlags, D3D10DDI_MAPPED_SUBRESOURCE* pMappedResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);

        HRESULT hr = SynchronizedResultToHRESULT(
            pDevice->GetBatchedContext().MapDefault(&pResource->BatchedResource(), Subresource, ConvertMapType(MapType), DoNotWait(MapFlags), nullptr, (D3D12TranslationLayer::MappedSubresource*)pMappedResource));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    void APIENTRY Device::UnmapDefault(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);

        pDevice->GetBatchedContext().UnmapDefault(&pResource->BatchedResource(), Subresource, nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::UnmapRenamedBuffer(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);
        assert(Subresource == 0);
        UNREFERENCED_PARAMETER(Subresource);

        pDevice->GetBatchedContext().UnmapRenamedBuffer(&pResource->BatchedResource(), nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY Device::UnmapUnderlyingStaging(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);

        pDevice->GetBatchedContext().UnmapStaging(&pResource->BatchedResource(), Subresource, nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::UnmapDynamicResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        Resource* pResource = Resource::CastFrom(hResource);

        if (pResource->ImmediateResource()->AppDesc()->Usage() == D3D12TranslationLayer::RESOURCE_USAGE_DYNAMIC  &&
            pResource->ImmediateResource()->AppDesc()->ResourceDimension() == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            assert(Subresource == 0);
            pDevice->GetBatchedContext().UnmapRenamedBuffer(&pResource->BatchedResource(), nullptr);
        }
        else
        {
            pDevice->GetBatchedContext().UnmapAndRenameViaCopy(&pResource->BatchedResource(), Subresource, nullptr);
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::MapDiscardComputeOnlyResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource, D3D10_DDI_MAP MapType, UINT, D3D10DDI_MAPPED_SUBRESOURCE* pMappedResource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        Resource* pResource = Resource::CastFrom(hResource);
        assert(pResource->ImmediateResource()->AppDesc()->Usage() == D3D12TranslationLayer::RESOURCE_USAGE_DYNAMIC);
        assert(MapType == D3D10_DDI_MAP_WRITE_DISCARD);
        UNREFERENCED_PARAMETER(MapType);

        // Note: Compute-only devices do not support binding of CPU-accessible resources. Therefore, DYNAMIC buffers are
        // allocated as DEFAULT buffers, and mapping them is done via copies. As such, no-overwrite semantics are not supported.
        HRESULT hr = SynchronizedResultToHRESULT(
            pDevice->GetBatchedContext().MapForRenameViaCopy(&pResource->BatchedResource(), Subresource, (D3D12TranslationLayer::MappedSubresource*)pMappedResource));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    void APIENTRY DeviceBase::MapNoOverwriteComputeOnlyResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE *)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        // Note: Compute-only devices do not support binding of CPU-accessible resources. Therefore, DYNAMIC buffers are
        // allocated as DEFAULT buffers, and mapping them is done via copies. As such, no-overwrite semantics are not supported.
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, E_NOTIMPL);
    }

    void APIENTRY DeviceBase::UnmapComputeOnlyResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource, UINT Subresource)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        Resource* pResource = Resource::CastFrom(hResource);
        // Note: Compute-only devices do not support binding of CPU-accessible resources. Therefore, DYNAMIC buffers are
        // allocated as DEFAULT buffers, and mapping them is done via copies. As such, no-overwrite semantics are not supported.
        pDevice->GetBatchedContext().UnmapAndRenameViaCopy(&pResource->BatchedResource(), Subresource, nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

}