// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    SIZE_T APIENTRY Device::CalcPrivateBlendStateSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1_DDI_BLEND_DESC* /*pBlendDesc*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T blendStateSize = sizeof(BlendState);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, blendStateSize);
    }

    VOID APIENTRY Device::CreateBlendState(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1_DDI_BLEND_DESC* pBlendDesc, D3D10DDI_HBLENDSTATE hBlend, D3D10DDI_HRTBLENDSTATE /*hRTBlend*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        new (hBlend.pDrvPrivate) BlendState(*pDevice, pBlendDesc);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyBlendState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HBLENDSTATE hBlend)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        BlendState::CastFrom(hBlend)->~BlendState();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    BlendState* BlendState::CastFrom(D3D10DDI_HBLENDSTATE hBlend) noexcept
    {
        return static_cast<BlendState*>(hBlend.pDrvPrivate);
    }

    BlendState::BlendState(Device& device, D3D11_1_DDI_BLEND_DESC const* pDDIDesc) noexcept
        : PipelineStateCacheKeyComponent(device)
    {
        // All enums contained within are identical, and memory layout is identical - simple memcpy.
        static_assert(sizeof(m_UnderlyingDesc) == sizeof(*pDDIDesc));
        memcpy(&m_UnderlyingDesc, pDDIDesc, sizeof(m_UnderlyingDesc));
    }

    SIZE_T APIENTRY Device::CalcPrivateDepthStencilStateSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10_DDI_DEPTH_STENCIL_DESC* /*pDepthDesc*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T depthStencilSize = sizeof(DepthStencilState);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, depthStencilSize);
    }

    VOID APIENTRY Device::CreateDepthStencilState(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D10_DDI_DEPTH_STENCIL_DESC* pDepthDesc, D3D10DDI_HDEPTHSTENCILSTATE hDepth, D3D10DDI_HRTDEPTHSTENCILSTATE /*hRTDepth*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        new (hDepth.pDrvPrivate) DepthStencilState(*pDevice, pDepthDesc);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyDepthStencilState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILSTATE hDepth)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        DepthStencilState::CastFrom(hDepth)->~DepthStencilState();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    DepthStencilState* DepthStencilState::CastFrom(D3D10DDI_HDEPTHSTENCILSTATE hDepth) noexcept
    {
        return static_cast<DepthStencilState*>(hDepth.pDrvPrivate);
    }

    DepthStencilState::DepthStencilState(Device& device, D3D10_DDI_DEPTH_STENCIL_DESC const* pDDIDesc) noexcept
        : PipelineStateCacheKeyComponent(device)
    {
        m_UnderlyingDesc.DepthEnable = pDDIDesc->DepthEnable;
        m_UnderlyingDesc.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)pDDIDesc->DepthWriteMask;
        m_UnderlyingDesc.DepthFunc = (D3D12_COMPARISON_FUNC)pDDIDesc->DepthFunc;
        m_UnderlyingDesc.StencilEnable = pDDIDesc->StencilEnable;
        m_UnderlyingDesc.StencilReadMask = pDDIDesc->StencilReadMask;
        m_UnderlyingDesc.StencilWriteMask = pDDIDesc->StencilWriteMask;

        m_UnderlyingDesc.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)pDDIDesc->FrontFace.StencilFailOp;
        m_UnderlyingDesc.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)pDDIDesc->FrontFace.StencilDepthFailOp;
        m_UnderlyingDesc.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)pDDIDesc->FrontFace.StencilPassOp;
        m_UnderlyingDesc.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)pDDIDesc->FrontFace.StencilFunc;

        m_UnderlyingDesc.BackFace.StencilFailOp = (D3D12_STENCIL_OP)pDDIDesc->BackFace.StencilFailOp;
        m_UnderlyingDesc.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)pDDIDesc->BackFace.StencilDepthFailOp;
        m_UnderlyingDesc.BackFace.StencilPassOp = (D3D12_STENCIL_OP)pDDIDesc->BackFace.StencilPassOp;
        m_UnderlyingDesc.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)pDDIDesc->BackFace.StencilFunc;
    }

    SIZE_T APIENTRY Device::CalcPrivateRasterizerStateSize(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDI_RASTERIZER_DESC* /*pDesc*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T rasterizerStateSize = sizeof(RasterizerState);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, rasterizerStateSize);
    }

    VOID APIENTRY Device::CreateRasterizerState(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDI_RASTERIZER_DESC* pDesc, D3D10DDI_HRASTERIZERSTATE hRaster, D3D10DDI_HRTRASTERIZERSTATE /*hRTRaster*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        new (hRaster.pDrvPrivate) RasterizerState(*pDevice, pDesc);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Device::DestroyRasterizerState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRASTERIZERSTATE hRaster)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        RasterizerState::CastFrom(hRaster)->~RasterizerState();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    RasterizerState* RasterizerState::CastFrom(D3D10DDI_HRASTERIZERSTATE hRaster) noexcept
    {
        return static_cast<RasterizerState*>(hRaster.pDrvPrivate);
    }

    RasterizerState::RasterizerState(Device& device, D3DWDDM2_0DDI_RASTERIZER_DESC const* pDDIDesc) noexcept
        : PipelineStateCacheKeyComponent(device)
        , m_bScissorEnabled(pDDIDesc->ScissorEnable != 0)
    {
        m_UnderlyingDesc.FillMode = (D3D12_FILL_MODE)pDDIDesc->FillMode;
        m_UnderlyingDesc.CullMode = (D3D12_CULL_MODE)pDDIDesc->CullMode;
        m_UnderlyingDesc.FrontCounterClockwise = pDDIDesc->FrontCounterClockwise;
        m_UnderlyingDesc.DepthBias = pDDIDesc->DepthBias;
        m_UnderlyingDesc.DepthBiasClamp = pDDIDesc->DepthBiasClamp;
        m_UnderlyingDesc.SlopeScaledDepthBias = pDDIDesc->SlopeScaledDepthBias;
        m_UnderlyingDesc.DepthClipEnable = pDDIDesc->DepthClipEnable;
        m_UnderlyingDesc.MultisampleEnable = pDDIDesc->MultisampleEnable;
        m_UnderlyingDesc.AntialiasedLineEnable = pDDIDesc->AntialiasedLineEnable;
        m_UnderlyingDesc.ForcedSampleCount = pDDIDesc->ForcedSampleCount;
        m_UnderlyingDesc.ConservativeRaster = (D3D12_CONSERVATIVE_RASTERIZATION_MODE)pDDIDesc->ConservativeRasterizationMode;
    }

};