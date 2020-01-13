// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void APIENTRY DeviceBase::Draw(D3D10DDI_HDEVICE hDevice, UINT count, UINT vertexStart)
    {
        DrawInstanced(hDevice, count, 1, vertexStart, 0);
    }

    void APIENTRY DeviceBase::DrawAuto(D3D10DDI_HDEVICE hDevice)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (!pDevice->m_PSODesc.pVertexShader) return;

        pDevice->PreDraw();
        pDevice->GetBatchedContext().DrawAuto();

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::DrawIndexed(D3D10DDI_HDEVICE hDevice, UINT indexCount, UINT indexStart, INT vertexStart)
    {
        DrawIndexedInstanced(hDevice, indexCount, 1, indexStart, vertexStart, 0);
    }

    void APIENTRY DeviceBase::DrawIndexedInstanced(D3D10DDI_HDEVICE hDevice, UINT countPerInstance, UINT instanceCount, UINT indexStart, INT vertexStart, UINT instanceStart)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (!pDevice->m_PSODesc.pVertexShader) return;

        pDevice->PreDraw();
        pDevice->GetBatchedContext().DrawIndexedInstanced(countPerInstance, instanceCount, indexStart, vertexStart, instanceStart);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::DrawIndexedInstancedIndirect(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hBuffer, UINT offset)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (!pDevice->m_PSODesc.pVertexShader) return;

        pDevice->PreDraw();
        pDevice->GetBatchedContext().DrawIndexedInstancedIndirect(Resource::CastFromAndGetImmediateResource(hBuffer), offset);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::DrawInstanced(D3D10DDI_HDEVICE hDevice, UINT countPerInstance, UINT instanceCount, UINT vertexStart, UINT instanceStart)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (!pDevice->m_PSODesc.pVertexShader) return;

        pDevice->PreDraw();
        pDevice->GetBatchedContext().DrawInstanced(countPerInstance, instanceCount, vertexStart, instanceStart);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::DrawInstancedIndirect(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hBuffer, UINT offset)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (!pDevice->m_PSODesc.pVertexShader) return;

        pDevice->PreDraw();
        pDevice->GetBatchedContext().DrawInstancedIndirect(Resource::CastFromAndGetImmediateResource(hBuffer), offset);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}