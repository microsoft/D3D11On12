// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void APIENTRY DeviceBase::Dispatch(D3D10DDI_HDEVICE hDevice, UINT x, UINT y, UINT z)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (!pDevice->m_ComputePSODesc.pCS) return;

        pDevice->PreDispatch();
        pDevice->GetBatchedContext().Dispatch(x, y, z);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::DispatchIndirect(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hBuffer, UINT offset)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (!pDevice->m_ComputePSODesc.pCS) return;

        pDevice->PreDispatch();
        pDevice->GetBatchedContext().DispatchIndirect(Resource::CastFromAndGetImmediateResource(hBuffer), offset);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}
