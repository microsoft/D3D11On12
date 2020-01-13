// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void APIENTRY DeviceBase::SetBlendState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HBLENDSTATE hBlendState, const FLOAT BlendFactor[4], UINT SampleMask)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        if (!pDevice->GetAdapter()->m_bComputeOnly)
        {
            pDevice->m_PSODesc.SampleMask = SampleMask;
            pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;

            pDevice->m_PSODesc.pBlendState = BlendState::CastFrom(hBlendState);
            pDevice->GetBatchedContext().OMSetBlendFactor(BlendFactor);
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::SetBlendState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HBLENDSTATE hBlendState, const FLOAT BlendFactor[4], UINT SampleMask)
    {
        DeviceBase::SetBlendState(hDevice, GetImmediate(hBlendState), BlendFactor, SampleMask);
    }

    void APIENTRY DeviceBase::SetDepthStencilState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILSTATE hDepthState, UINT StencilRef)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        if (!pDevice->GetAdapter()->m_bComputeOnly)
        {
            pDevice->m_PSODesc.pDepthStencilState = DepthStencilState::CastFrom(hDepthState);
            pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
            pDevice->GetBatchedContext().OMSetStencilRef(StencilRef);
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::SetDepthStencilState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILSTATE hDepthState, UINT StencilRef)
    {
        DeviceBase::SetDepthStencilState(hDevice, GetImmediate(hDepthState), StencilRef);
    }

    void APIENTRY DeviceBase::SetRasterizerState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRASTERIZERSTATE hRasterizerState)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        if (!pDevice->GetAdapter()->m_bComputeOnly)
        {
            pDevice->m_PSODesc.pRasterizerState = RasterizerState::CastFrom(hRasterizerState);
            bool bScissorEnabled = pDevice->m_PSODesc.pRasterizerState ? pDevice->m_PSODesc.pRasterizerState->m_bScissorEnabled : false;
            pDevice->GetBatchedContext().SetScissorRectEnable(bScissorEnabled);
            pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::SetRasterizerState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRASTERIZERSTATE hRasterizerState)
    {
        DeviceBase::SetRasterizerState(hDevice, GetImmediate(hRasterizerState));
    }

    inline D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertTopologyToType( D3D10_DDI_PRIMITIVE_TOPOLOGY Value )
    {
        static_assert(D3D11_DDI_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST == 33);
        if ( Value > 32 )
        {
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        }
        switch(Value)
        {
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_UNDEFINED:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_POINTLIST:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINELIST:
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINESTRIP:
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        case D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            ASSUME(false);
        }
    }

    void APIENTRY DeviceBase::IaSetTopology(D3D10DDI_HDEVICE hDevice, D3D10_DDI_PRIMITIVE_TOPOLOGY topology)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        if (!pDevice->GetAdapter()->m_bComputeOnly)
        {
            auto &CurrentTopologyType = pDevice->m_PSODesc.PrimitiveTopologyType;
            auto NewTopologyType = ConvertTopologyToType(topology);
            if (CurrentTopologyType != NewTopologyType)
            {
                CurrentTopologyType = NewTopologyType;
                pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
            }

            pDevice->GetBatchedContext().IaSetTopology(static_cast<D3D_PRIMITIVE_TOPOLOGY>(topology));
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}