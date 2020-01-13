// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    SIZE_T APIENTRY Device::CalcPrivateElementLayoutSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10DDIARG_CREATEELEMENTLAYOUT* /*pCreateElementLayout*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        SIZE_T inputLayoutSize =  sizeof(InputLayout);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, inputLayoutSize);
    }

    VOID APIENTRY Device::CreateElementLayout(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10DDIARG_CREATEELEMENTLAYOUT* pCreateElementLayout, D3D10DDI_HELEMENTLAYOUT hElementLayout, D3D10DDI_HRTELEMENTLAYOUT /*hRTElementLayout*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        new (hElementLayout.pDrvPrivate) InputLayout(pDevice, pCreateElementLayout); // throw( bad_alloc )
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyElementLayout(D3D10DDI_HDEVICE hDevice, D3D10DDI_HELEMENTLAYOUT hElementLayout)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        InputLayout::CastFrom(hElementLayout)->~InputLayout();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::IaSetInputLayout(D3D10DDI_HDEVICE hDevice, D3D10DDI_HELEMENTLAYOUT hInputLayout)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->m_PSODesc.pInputLayout = InputLayout::CastFrom(hInputLayout);
        pDevice->m_DirtyStates |= e_GraphicsPipelineStateDirty;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::IaSetInputLayout(D3D10DDI_HDEVICE hDevice, D3D10DDI_HELEMENTLAYOUT hInputLayout)
    {
        DeviceBase::IaSetInputLayout(hDevice, GetImmediate(hInputLayout));
    }
};