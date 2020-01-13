// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    D3D12_SHADER_RESOURCE_VIEW_DESC GetTranslationDesc(D3D12TranslationLayer::Resource&, D3DWDDM2_0DDIARG_CREATESHADERRESOURCEVIEW const* pDesc);
    D3D12_RENDER_TARGET_VIEW_DESC GetTranslationDesc(D3D12TranslationLayer::Resource&, D3DWDDM2_0DDIARG_CREATERENDERTARGETVIEW const* pDesc);
    D3D12_DEPTH_STENCIL_VIEW_DESC GetTranslationDesc(D3D12TranslationLayer::Resource&, D3D11DDIARG_CREATEDEPTHSTENCILVIEW const* pDesc);
    D3D12TranslationLayer::D3D12_UNORDERED_ACCESS_VIEW_DESC_WRAPPER GetTranslationDesc(D3D12TranslationLayer::Resource&, D3DWDDM2_0DDIARG_CREATEUNORDEREDACCESSVIEW const* pDesc);
    D3D12TranslationLayer::VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL GetTranslationDesc(D3D12TranslationLayer::Resource&, D3D11_1DDIARG_CREATEVIDEODECODEROUTPUTVIEW const* pDesc);
    D3D12TranslationLayer::VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL GetTranslationDesc(D3D12TranslationLayer::Resource&, D3D11_1DDIARG_CREATEVIDEOPROCESSORINPUTVIEW const* pDesc);
    D3D12TranslationLayer::VIDEO_PROCESSOR_OUTPUT_VIEW_DESC_INTERNAL GetTranslationDesc(D3D12TranslationLayer::Resource&, D3D11_1DDIARG_CREATEVIDEOPROCESSOROUTPUTVIEW const* pDesc);

    template<typename Traits>
    inline View<Traits>::View(Device &device, D3D12TranslationLayer::Resource &resource, typename Traits::DDIDesc const* pDesc) :
        DeviceChild(device),
        m_spView(nullptr, { device.GetBatchedContext() })
    {
        m_spView.reset(new Traits::TranslationLayerView(&device.GetImmediateContextNoFlush(), GetTranslationDesc(resource, pDesc), resource));
    }

    template<D3D12TranslationLayer::EShaderStage ShaderUnit>
    void APIENTRY DeviceBase::SetShaderResources(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT-1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) UINT NumSRVs, _In_reads_(NumSRVs) const D3D10DDI_HSHADERRESOURCEVIEW* phSRVs)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D12TranslationLayer::SRV *ppSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        // SAL annotation doesn't seem enough to keep OACR happy...
        ASSUME(NumSRVs <= D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
        ShaderResourceView::GatherViewsFromHandles(phSRVs, ppSRVs, NumSRVs);

        auto pDevice = CastFrom(hDevice);
        pDevice->GetBatchedContext().SetShaderResources<ShaderUnit>(StartSlot, NumSRVs, ppSRVs);

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    template<D3D12TranslationLayer::EShaderStage ShaderUnit>
    void APIENTRY DeviceDeferred::SetShaderResources(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) UINT NumSRVs, _In_reads_(NumSRVs) const D3D10DDI_HSHADERRESOURCEVIEW* phSRVs)
    {
        D3D10DDI_HSHADERRESOURCEVIEW SRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        std::transform(phSRVs, phSRVs + NumSRVs, SRVs, [](D3D10DDI_HSHADERRESOURCEVIEW h) { return GetImmediate(h); });
        DeviceBase::SetShaderResources<ShaderUnit>(hDevice, StartSlot, NumSRVs, SRVs);
    }

    template<D3D12TranslationLayer::EShaderStage shaderUnit>
    void APIENTRY DeviceBase::SetSamplers(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT-1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) UINT NumSamplers, _In_reads_(NumSamplers) const D3D10DDI_HSAMPLER* phSamplers)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        D3D12TranslationLayer::Sampler* ppSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        // SAL annotation doesn't seem enough to keep OACR happy...
        ASSUME(NumSamplers <= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
        for (size_t i = 0; i < NumSamplers; i++)
        {
            ppSamplers[i] = (phSamplers[i].pDrvPrivate) ? Sampler::CastFrom(phSamplers[i])->Underlying() : nullptr;
        }

        pDevice->GetBatchedContext().SetSamplers<shaderUnit>(StartSlot, NumSamplers, ppSamplers);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    template<D3D12TranslationLayer::EShaderStage shaderUnit>
    void APIENTRY DeviceDeferred::SetSamplers(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) UINT NumSamplers, _In_reads_(NumSamplers) const D3D10DDI_HSAMPLER* phSamplers)
    {
        D3D10DDI_HSAMPLER Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        std::transform(phSamplers, phSamplers + NumSamplers, Samplers, [](D3D10DDI_HSAMPLER h) { return GetImmediate(h); });
        DeviceBase::SetSamplers<shaderUnit>(hDevice, StartSlot, NumSamplers, Samplers);
    }

    template<D3D12TranslationLayer::EShaderStage shaderUnit>
    void APIENTRY DeviceBase::SetConstantBuffers(D3D10DDI_HDEVICE hDevice, _In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT-1) UINT StartSlot, _In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT) UINT NumBuffers, _In_reads_(NumBuffers) const D3D10DDI_HRESOURCE* phCBs, _In_reads_opt_(NumBuffers) CONST UINT* pFirstConstant, _In_reads_opt_(NumBuffers) CONST UINT* pNumConstants)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        D3D12TranslationLayer::Resource* pUnderlying[D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT];
        // SAL annotation doesn't seem enough to keep OACR happy...
        ASSUME(NumBuffers <= D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT);
        Resource::GatherImmediateResourcesFromHandles(phCBs, pUnderlying, NumBuffers);

        pDevice->GetBatchedContext().SetConstantBuffers<shaderUnit>(StartSlot, NumBuffers, pUnderlying, pFirstConstant, pNumConstants);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}