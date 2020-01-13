// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class BlendState : public PipelineStateCacheKeyComponent<PSODescKey>
    {
    public:
        BlendState(Device &device, D3D11_1_DDI_BLEND_DESC const* pDesc) noexcept;
        static BlendState* CastFrom(D3D10DDI_HBLENDSTATE) noexcept;
        D3D12_BLEND_DESC m_UnderlyingDesc;
    };
    class RasterizerState : public PipelineStateCacheKeyComponent<PSODescKey>
    {
    public:
        RasterizerState(Device &device, D3DWDDM2_0DDI_RASTERIZER_DESC const* pDesc) noexcept;
        static RasterizerState* CastFrom(D3D10DDI_HRASTERIZERSTATE) noexcept;
        D3D12_RASTERIZER_DESC m_UnderlyingDesc;
        const bool m_bScissorEnabled;
    };
    class DepthStencilState : public PipelineStateCacheKeyComponent<PSODescKey>
    {
    public:
        DepthStencilState(Device &device, D3D10_DDI_DEPTH_STENCIL_DESC const* pDesc) noexcept;
        static DepthStencilState* CastFrom(D3D10DDI_HDEPTHSTENCILSTATE) noexcept;
        D3D12_DEPTH_STENCIL_DESC m_UnderlyingDesc;
    };
}
