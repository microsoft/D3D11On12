// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class Sampler : public DeviceChild
    {
    public:

        Sampler(Device& parent)
            : DeviceChild(parent)
            , m_pUnderlying(nullptr, { parent.GetBatchedContext() })
        {
        }

        static Sampler* CastFrom(D3D10DDI_HSAMPLER) noexcept;

        static SIZE_T APIENTRY  CalcPrivateSamplerSize(D3D10DDI_HDEVICE, _In_ CONST D3D10_DDI_SAMPLER_DESC*);
        static VOID APIENTRY  CreateSampler(D3D10DDI_HDEVICE, _In_ CONST D3D10_DDI_SAMPLER_DESC*, D3D10DDI_HSAMPLER, D3D10DDI_HRTSAMPLER);
        static VOID APIENTRY  DestroySampler(D3D10DDI_HDEVICE, D3D10DDI_HSAMPLER);

        D3D12TranslationLayer::Sampler* Underlying() { return m_pUnderlying.get(); }

    private:
        D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Sampler> m_pUnderlying;
    };
};