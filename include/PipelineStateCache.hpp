// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    template<typename TIface> class Shader;
    class InputLayout;
    class BlendState;
    class DepthStencilState;
    class RasterizerState;
    struct PSODescKey
    {
        Shader<ID3D11VertexShader>* pVertexShader;
        Shader<ID3D11PixelShader>* pPixelShader;
        Shader<ID3D11GeometryShader>* pGeometryShader;
        Shader<ID3D11DomainShader>* pDomainShader;
        Shader<ID3D11HullShader>* pHullShader;
        InputLayout* pInputLayout;
        BlendState* pBlendState;
        DepthStencilState* pDepthStencilState;
        RasterizerState* pRasterizerState;

        decltype(D3D12_GRAPHICS_PIPELINE_STATE_DESC::NumRenderTargets) NumRenderTargets;
        decltype(D3D12_GRAPHICS_PIPELINE_STATE_DESC::SampleMask) SampleMask;
        decltype(D3D12_GRAPHICS_PIPELINE_STATE_DESC::SampleDesc) SampleDesc;

        decltype(D3D12_GRAPHICS_PIPELINE_STATE_DESC::RTVFormats) RTVFormats;
        decltype(D3D12_GRAPHICS_PIPELINE_STATE_DESC::DSVFormat) DSVFormat;
        decltype(D3D12_GRAPHICS_PIPELINE_STATE_DESC::IBStripCutValue) IBStripCutValue;
        decltype(D3D12_GRAPHICS_PIPELINE_STATE_DESC::PrimitiveTopologyType) PrimitiveTopologyType;

        PSODescKey()
        {
            pVertexShader = nullptr;
            pPixelShader = nullptr;
            pGeometryShader = nullptr;
            pDomainShader = nullptr;
            pHullShader = nullptr;
            pInputLayout = nullptr;
            pBlendState = nullptr;
            pDepthStencilState = nullptr;
            pRasterizerState = nullptr;
            NumRenderTargets = 0;
            SampleMask = 0xffffffff;
            SampleDesc.Count = 1;
            SampleDesc.Quality = 0;
            ZeroMemory(RTVFormats, sizeof(RTVFormats));
            DSVFormat = DXGI_FORMAT_UNKNOWN;
            IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
            PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        }
    };

    struct ComputePSOKey
    {
        Shader<ID3D11ComputeShader>* pCS = nullptr;
    };
}

namespace std
{
    template<>
    struct hash<D3D11On12::PSODescKey>
    {
        size_t operator()(D3D11On12::PSODescKey const& desc) const
        {
            size_t seed = 0;
            D3D12TranslationLayer::hash_combine(seed, desc.pVertexShader);
            D3D12TranslationLayer::hash_combine(seed, desc.pPixelShader);
            D3D12TranslationLayer::hash_combine(seed, desc.pDomainShader);
            D3D12TranslationLayer::hash_combine(seed, desc.pHullShader);
            D3D12TranslationLayer::hash_combine(seed, desc.pGeometryShader);
            D3D12TranslationLayer::hash_combine(seed, desc.pBlendState);
            D3D12TranslationLayer::hash_combine(seed, desc.SampleMask);
            D3D12TranslationLayer::hash_combine(seed, desc.pRasterizerState);
            D3D12TranslationLayer::hash_combine(seed, desc.pDepthStencilState);
            D3D12TranslationLayer::hash_combine(seed, desc.pInputLayout);
            D3D12TranslationLayer::hash_combine(seed, (UINT)desc.PrimitiveTopologyType);
            D3D12TranslationLayer::hash_combine(seed, (UINT)desc.IBStripCutValue);
            for (UINT i = 0; i < ARRAYSIZE(desc.RTVFormats); ++i)
            {
                D3D12TranslationLayer::hash_combine(seed, (UINT)desc.RTVFormats[i]);
            }
            D3D12TranslationLayer::hash_combine(seed, (UINT)desc.DSVFormat);
            D3D12TranslationLayer::hash_combine(seed, desc.SampleDesc.Count);
            D3D12TranslationLayer::hash_combine(seed, desc.SampleDesc.Quality);
            return seed;
        }
    };

    template<>
    struct hash<D3D11On12::ComputePSOKey>
    {
        size_t operator()(D3D11On12::ComputePSOKey const& desc) const
        {
            size_t seed = 0;
            D3D12TranslationLayer::hash_combine(seed, desc.pCS);
            return seed;
        }
    };
};

namespace D3D11On12
{
    template <typename Key>
    struct PipelineStateCacheEntry
    {
        std::unique_ptr<D3D12TranslationLayer::PipelineState, D3D12TranslationLayer::BatchedDeleter> pState;
        Key const* pKey = nullptr;

        PipelineStateCacheEntry(D3D12TranslationLayer::BatchedContext& Context)
            : pState(nullptr, D3D12TranslationLayer::BatchedDeleter{ Context })
        {
        }
    };

    template <typename Key> using PipelineStateCache = std::unordered_map<Key, std::shared_ptr<PipelineStateCacheEntry<Key>>>;
    typedef PipelineStateCache<PSODescKey> GraphicsPipelineStateCache;
    typedef PipelineStateCache<ComputePSOKey> ComputePipelineStateCache;
    template <typename Key> using PipelineStateCacheLocked =
    typename D3D12TranslationLayer::COptLockedContainer<PipelineStateCache<Key>>::LockedAccess;

    // State objects additionally inherit from this
    template <typename Key>
    struct PipelineStateCacheKeyComponent : public DeviceChild
    {
        using CacheEntry = PipelineStateCacheEntry<Key>;
        std::vector<std::weak_ptr<CacheEntry>> m_pPSOComponents;
        PipelineStateCacheLocked<Key> GetCache()
        {
            return m_parentDevice.GetPSOCache<Key>();
        }

        // Immovable
        PipelineStateCacheKeyComponent(Device& device)
            : DeviceChild(device)
        {
        }
        PipelineStateCacheKeyComponent(PipelineStateCacheKeyComponent const&) = delete;
        PipelineStateCacheKeyComponent& operator=(PipelineStateCacheKeyComponent const&) = delete;
        PipelineStateCacheKeyComponent(PipelineStateCacheKeyComponent&&) = delete;
        PipelineStateCacheKeyComponent& operator=(PipelineStateCacheKeyComponent&&) = delete;

        ~PipelineStateCacheKeyComponent()
        {
            auto Cache = GetCache();
            for (std::weak_ptr<CacheEntry>& weakPtr : m_pPSOComponents)
            {
                auto sharedPtr = weakPtr.lock();
                if (sharedPtr)
                {
                    Cache->erase(*sharedPtr->pKey);
                }
            }
        }

        void AddPSO(std::shared_ptr<CacheEntry> const& sharedPtr)
        {
            for (auto iter = m_pPSOComponents.begin(); iter != m_pPSOComponents.end(); )
            {
                auto locked = iter->lock();
                if (!locked)
                {
                    iter = m_pPSOComponents.erase(iter);
                    continue;
                }
                ++iter;
            }
            m_pPSOComponents.push_back(sharedPtr);
        }
    };
}