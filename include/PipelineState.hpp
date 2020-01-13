// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once


namespace D3D11On12
{
    template <typename Key>
    inline void AddUse(PipelineStateCacheKeyComponent<Key>* pComponent,
                       std::shared_ptr<PipelineStateCacheEntry<Key>> const& sharedPtr)
    {
        if (pComponent) { pComponent->AddPSO(sharedPtr); }
    }

    enum EDirtyBits
    {
        // Pipeline states:
        // Dirty bits are set when a shader or constant in the PSO desc changes, and causes a PSO lookup/compile
        // Reassert bits are set on command list boundaries, on graphics/compute boundaries, and after dirty processing
        e_GraphicsPipelineStateDirty = 0x1,
        e_ComputePipelineStateDirty = 0x2,
    };

    inline void AddUses(const PSODescKey& Key, std::shared_ptr<PipelineStateCacheEntry<PSODescKey>> const& sharedPtr)
    {
        // Called when the key is added to the PSO cache.
        // This ensures that the PSO cache doesn't have dangling pointers to shaders/state objects
        AddUse(Key.pVertexShader, sharedPtr);
        AddUse(Key.pPixelShader, sharedPtr);
        AddUse(Key.pGeometryShader, sharedPtr);
        AddUse(Key.pDomainShader, sharedPtr);
        AddUse(Key.pHullShader, sharedPtr);
        AddUse(Key.pBlendState, sharedPtr);
        AddUse(Key.pDepthStencilState, sharedPtr);
        AddUse(Key.pRasterizerState, sharedPtr);
        AddUse(Key.pInputLayout, sharedPtr);
    }

    // Same methods for compute PSOs
    inline void AddUses(const ComputePSOKey &Key, std::shared_ptr<PipelineStateCacheEntry<ComputePSOKey>> const& sharedPtr)
    {
        AddUse(Key.pCS, sharedPtr);
    }

    inline bool operator==(PSODescKey const& a, PSODescKey const& b)
    {
        return memcmp(&a, &b, sizeof(a)) == 0;
    }

    inline bool operator==(ComputePSOKey const& a, ComputePSOKey const& b)
    {
        return memcmp(&a, &b, sizeof(a)) == 0;
    }

    struct PSODesc : public PSODescKey
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements;

        operator D3D12TranslationLayer::GRAPHICS_PIPELINE_STATE_DESC()
        {
            D3D12TranslationLayer::GRAPHICS_PIPELINE_STATE_DESC Ret = {};
            Ret.pVertexShader = (pVertexShader) ? pVertexShader->Underlying() : nullptr;
            Ret.pPixelShader = (pPixelShader) ? pPixelShader->Underlying() : nullptr;

            // The GS bytecode may be DS or VS bytecode
            // in the case that CreateGeometryShaderWithStreamOut was called
            // In this case, don't pass this bytecode to CreateGraphicsPipelineState()
            Ret.pGeometryShader = (pGeometryShader) ? pGeometryShader->Underlying() : nullptr;
            Ret.pDomainShader = (pDomainShader) ? pDomainShader->Underlying(): nullptr;
            Ret.pHullShader = (pHullShader) ? pHullShader->Underlying() : nullptr;
            Ret.StreamOutput = (pGeometryShader && pGeometryShader->m_bIsStreamOutput) ?
                static_cast<StreamOutShader*>(pGeometryShader)->ProduceDesc(pVertexShader, pDomainShader) : D3D12_STREAM_OUTPUT_DESC{};
            Ret.InputLayout = pInputLayout ? pInputLayout->ProduceDesc(pVertexShader, InputElements) : D3D12_INPUT_LAYOUT_DESC{};
            Ret.BlendState = pBlendState->m_UnderlyingDesc;
            Ret.DepthStencilState = pDepthStencilState->m_UnderlyingDesc;
            Ret.RasterizerState = pRasterizerState->m_UnderlyingDesc;

            Ret.NumRenderTargets = NumRenderTargets;
            Ret.SampleDesc = SampleDesc;
            Ret.SampleMask = SampleMask;
            memcpy(Ret.RTVFormats, RTVFormats, sizeof(RTVFormats));
            Ret.DSVFormat = DSVFormat;
            Ret.IBStripCutValue = IBStripCutValue;
            Ret.PrimitiveTopologyType = PrimitiveTopologyType;
            return Ret;
        }
    };

    template <D3D12TranslationLayer::EPipelineType Type> struct PSOCacheKey;

    template<> struct PSOCacheKey<D3D12TranslationLayer::e_Draw> 
    { 
        typedef PSODescKey KeyType; 
    };

    template<> struct PSOCacheKey<D3D12TranslationLayer::e_Dispatch> 
    { 
        typedef ComputePSOKey KeyType; 
    };
}