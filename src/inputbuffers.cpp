// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void APIENTRY DeviceBase::IaSetVertexBuffers(D3D10DDI_HDEVICE hDevice, UINT StartSlot, __in_range(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT) UINT NumBuffers, const D3D10DDI_HRESOURCE* pVBs, const UINT* pStrides, const UINT* pOffsets)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        D3D12TranslationLayer::Resource* pUnderlying[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
        Resource::GatherImmediateResourcesFromHandles(pVBs, pUnderlying, NumBuffers);

        pDevice->GetBatchedContext().IaSetVertexBuffers(StartSlot, NumBuffers, pUnderlying, pStrides, pOffsets);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::IaSetIndexBuffer(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hIB, DXGI_FORMAT fmt, UINT offset)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        D3D12_INDEX_BUFFER_STRIP_CUT_VALUE neededProperties = (fmt == DXGI_FORMAT_R32_UINT) ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        if (pDevice->m_PSODesc.IBStripCutValue != neededProperties)
        {
            pDevice->m_PSODesc.IBStripCutValue = neededProperties;
            pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
        }

        pDevice->GetBatchedContext().IaSetIndexBuffer(Resource::CastFromAndGetImmediateResource(hIB), fmt, offset);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }


    void APIENTRY DeviceBase::SoSetTargets(D3D10DDI_HDEVICE hDevice, _In_range_(0, 4) UINT NumTargets, _In_range_(0, 4) UINT ClearSlots, _In_reads_(NumTargets) const D3D10DDI_HRESOURCE* pBuffers, _In_reads_(NumTargets) const UINT* offsets)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        D3D12TranslationLayer::Resource* pUnderlying[4];
        Resource::GatherImmediateResourcesFromHandles(pBuffers, pUnderlying, NumTargets);

        pDevice->GetBatchedContext().SoSetTargets(NumTargets, ClearSlots, pUnderlying, offsets);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}