// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    // Remove template and inline.
    void ConvertShaderToDxil(
        Device* pDevice, std::unique_ptr<BYTE[]>& spBytecode, SIZE_T bytecodeSize, bool bInterfaces,
        D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader>& spCreatedShader) noexcept(false);

    template <typename TDesc>
    D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader> CreateUnderlyingShader(Device* pDevice, const TDesc* pDesc)
    {
        D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader> spUnderlying(nullptr, { pDevice->GetBatchedContext() });
        if (pDesc)
        {
            SIZE_T bytecodeSize = pDesc->SizeInBytes;
            std::unique_ptr<BYTE[]> spBytecode(new BYTE[bytecodeSize]);
            memcpy(spBytecode.get(), pDesc->pFunction, bytecodeSize);

            if (pDevice->GetAdapter()->m_ShaderModelCaps.HighestShaderModel >= D3D_SHADER_MODEL_6_0)
            {
                ConvertShaderToDxil(pDevice, spBytecode, bytecodeSize, pDesc->pLinkage != nullptr, spUnderlying); // throw( _com_error, bad_alloc )
            }
            else
            {
                spUnderlying.reset(new D3D12TranslationLayer::Shader(&pDevice->GetImmediateContextNoFlush(), std::move(spBytecode), bytecodeSize));
            }
        }
        return spUnderlying;
    }

    template<typename TIface>
    void Shader<TIface>::Create(Device* pDevice, THandle handle, const TDesc* pDesc) noexcept(false)
    {
        new (handle.pDrvPrivate) Shader<TIface>(pDevice, CreateUnderlyingShader(pDevice, pDesc)); // throw( _com_error, bad_alloc )
    }

    template<typename TIface>
    Shader<TIface>::Shader(Device* pDevice, D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader> spUnderlying) noexcept(false)
        : m_bIsStreamOutput(false)
        , m_pUnderlying(std::move(spUnderlying))
        , PipelineStateCacheKeyComponent(*pDevice)
    {
        if (m_pUnderlying)
        {
            ParseInputSemantics(); // throw( _com_error )
            ParseOutputSemantics(); // throw( _com_error )
        }
    }

    template<typename TIface>
    SIZE_T TRANSLATION_API Shader<TIface>::CalcPrivateSize(const UINT*, const TSignatures*)
    {
        return sizeof(Shader<TIface>);
    }

    template<> inline void Shader<ID3D11VertexShader>::ParseInputSemantics() noexcept(false)
    {
        CSignatureParser &SigParser = m_IOSemantics.m_InputSignature;
        HRESULT hr = DXBCGetInputSignature(m_pUnderlying->GetByteCode().pShaderBytecode, &SigParser, true);
        ThrowFailure(hr); // throw( _com_error )
    }

    template<typename TIface>
    inline void Shader<TIface>::ParseInputSemantics() noexcept(false) { }

    template<> inline void Shader<ID3D11VertexShader>::ParseOutputSemantics() noexcept(false)
    {
        CSignatureParser &SigParser = m_IOSemantics.m_OutputSignature;
        HRESULT hr = DXBCGetOutputSignature(m_pUnderlying->GetByteCode().pShaderBytecode, &SigParser, true);
        ThrowFailure(hr); // throw( _com_error )
    }

    template<> inline void Shader<ID3D11DomainShader>::ParseOutputSemantics() noexcept(false)
    {
        CSignatureParser &SigParser = m_IOSemantics.m_OutputSignature;
        HRESULT hr = DXBCGetOutputSignature(m_pUnderlying->GetByteCode().pShaderBytecode, &SigParser, true);
        ThrowFailure(hr); // throw( _com_error )
    }

    template<typename TIface>
    inline void Shader<TIface>::ParseOutputSemantics() noexcept(false) { }

    template< D3D12TranslationLayer::EShaderStage eShader>
    struct SShaderTraits;

#define SHADER_TRAITS_11ON12( initial, nameLower ) \
        template<> \
        struct SShaderTraits<D3D12TranslationLayer::e_##initial##S> \
        { \
        typedef Shader<ID3D11##nameLower##Shader> TShader; \
        static auto GetQueryCb(D3DWDDM2_6DDI_CORELAYER_DEVICECALLBACKS const* pCallbacks) { return pCallbacks->pfnState##initial##sConstBufCb; } \
        }
    SHADER_TRAITS_11ON12(V, Vertex);
    SHADER_TRAITS_11ON12(P, Pixel);
    SHADER_TRAITS_11ON12(G, Geometry);
    SHADER_TRAITS_11ON12(D, Domain);
    SHADER_TRAITS_11ON12(H, Hull);
    SHADER_TRAITS_11ON12(C, Compute);


#define GRAPHICS_SHADER_TRAITS_11ON12(initial, nameLower) \
    template<> struct DeviceBase::SSupplementalShaderTraits<D3D12TranslationLayer::e_##initial##S> \
    { \
    static constexpr UINT c_PSODirtyBit = e_GraphicsPipelineStateDirty; \
    static PSODesc& GetPSODesc(DeviceBase* pDevice) { return pDevice->m_PSODesc; } \
    static SShaderTraits<D3D12TranslationLayer::e_##initial##S>::TShader** GetPSODescEntry(PSODesc& PSODesc) { return &PSODesc.p##nameLower##Shader; } \
    }

    GRAPHICS_SHADER_TRAITS_11ON12(V, Vertex);
    GRAPHICS_SHADER_TRAITS_11ON12(P, Pixel);
    GRAPHICS_SHADER_TRAITS_11ON12(G, Geometry);
    GRAPHICS_SHADER_TRAITS_11ON12(D, Domain);
    GRAPHICS_SHADER_TRAITS_11ON12(H, Hull);

    template<> struct DeviceBase::SSupplementalShaderTraits<D3D12TranslationLayer::e_CS>
    {
        static constexpr UINT c_PSODirtyBit = e_ComputePipelineStateDirty;
        static ComputePSOKey& GetPSODesc(DeviceBase* pDevice) { return pDevice->m_ComputePSODesc; }
        static T11On12ComputeShader** GetPSODescEntry(ComputePSOKey& PSODesc) { return &PSODesc.pCS; }
    };

    template<D3D12TranslationLayer::EShaderStage eShader>
    inline void APIENTRY DeviceBase::SetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        auto pShader = SShaderTraits<eShader>::TShader::CastFrom(hShader);
        *SSupplementalShaderTraits<eShader>::GetPSODescEntry(
            SSupplementalShaderTraits<eShader>::GetPSODesc(pDevice)) = pShader;
        pDevice->m_DirtyStates |= SSupplementalShaderTraits<eShader>::c_PSODirtyBit;

        // We're binding a non-interface shader, unbind the interface buffer (by asking the runtime to reassert
        // its binding for that slot, which is almost definitely null).
        if (pDevice->m_bInterfaceBufferBound[eShader])
        {
            SShaderTraits<eShader>::GetQueryCb(pDevice->m_pCallbacks)(pDevice->m_hRTCoreLayer, 14, 1);
            pDevice->m_bInterfaceBufferBound[eShader] = false;
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    template<D3D12TranslationLayer::EShaderStage eShader>
    inline void APIENTRY DeviceDeferred::SetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
    {
        DeviceBase::SetShader<eShader>(hDevice, GetImmediate(hShader));
    }

    template<D3D12TranslationLayer::EShaderStage eShader>
    inline void APIENTRY DeviceBase::SetShaderWithIfaces(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader, UINT NumClassInstances, __in_ecount(NumClassInstances) const UINT* pInterfaces, __in_ecount(NumClassInstances) const D3D11DDIARG_POINTERDATA* pData)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        pDevice->InitInterfacesBuffer(eShader);

        auto pShader = SShaderTraits<eShader>::TShader::CastFrom(hShader);
        *SSupplementalShaderTraits<eShader>::GetPSODescEntry(
            SSupplementalShaderTraits<eShader>::GetPSODesc(pDevice)) = pShader;
        pDevice->m_DirtyStates |= SSupplementalShaderTraits<eShader>::c_PSODirtyBit;

        if (NumClassInstances != pDevice->m_InterfaceInstances[eShader].size() ||
            memcmp(pInterfaces, pDevice->m_InterfaceInstances[eShader].data(), sizeof(*pInterfaces) * NumClassInstances) != 0 ||
            memcmp(pData, pDevice->m_InterfacePointerData[eShader].data(), sizeof(*pData) * NumClassInstances) != 0)
        {
            pDevice->m_bInterfaceBufferDirty[eShader] = true;
            pDevice->m_InterfaceInstances[eShader].assign(pInterfaces, pInterfaces + NumClassInstances);
            pDevice->m_InterfacePointerData[eShader].assign(pData, pData + NumClassInstances);
        }

        if (!pDevice->m_bInterfaceBufferBound[eShader])
        {
            auto pInterfaceBuffer = pDevice->m_spInterfaceBuffers[eShader]->m_pResource;
            pDevice->GetBatchedContext().SetConstantBuffers<eShader>(14, 1, &pInterfaceBuffer, nullptr, nullptr);
            pDevice->m_bInterfaceBufferBound[eShader] = true;
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    template<D3D12TranslationLayer::EShaderStage eShader>
    inline void APIENTRY DeviceDeferred::SetShaderWithIfaces(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader, UINT NumClassInstances, __in_ecount(NumClassInstances) const UINT* pInterfaces, __in_ecount(NumClassInstances) const D3D11DDIARG_POINTERDATA* pData)
    {
        DeviceBase::SetShaderWithIfaces<eShader>(hDevice, GetImmediate(hShader), NumClassInstances, pInterfaces, pData);
    }
}