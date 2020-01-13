// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    SIZE_T APIENTRY Sampler:: CalcPrivateSamplerSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10_DDI_SAMPLER_DESC* /*pDesc*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        SIZE_T samplerSize = sizeof(Sampler);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, samplerSize);
    }

    VOID APIENTRY Sampler:: CreateSampler(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10_DDI_SAMPLER_DESC* pDesc, D3D10DDI_HSAMPLER hSampler, D3D10DDI_HRTSAMPLER /*hRTSampler*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        Sampler* pSampler = new (hSampler.pDrvPrivate) Sampler(*pDevice);

        pSampler->m_pUnderlying.reset(new D3D12TranslationLayer::Sampler(&pDevice->GetImmediateContextNoFlush(), *reinterpret_cast<D3D12_SAMPLER_DESC const*>(pDesc)));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Sampler::DestroySampler(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSAMPLER hSampler)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        Sampler* pSampler = Sampler::CastFrom(hSampler);
        pSampler->~Sampler();

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    Sampler* Sampler::CastFrom(D3D10DDI_HSAMPLER handle) noexcept
    {
        return reinterpret_cast<Sampler*>(handle.pDrvPrivate);
    }
};