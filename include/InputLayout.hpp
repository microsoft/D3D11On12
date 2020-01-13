// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
namespace D3D11On12
{
    class Device;

    //==================================================================================================================================
    // InputLayout
    //==================================================================================================================================
    class InputLayout : public PipelineStateCacheKeyComponent<PSODescKey>
    {
    public: // Methods
        InputLayout(Device* pDevice, const D3D10DDIARG_CREATEELEMENTLAYOUT* pDesc) noexcept(false);

        static InputLayout* CastFrom(D3D10DDI_HELEMENTLAYOUT) noexcept;
        D3D10DDI_HELEMENTLAYOUT DDIHandle() noexcept { return MAKE_D3D10DDI_HELEMENTLAYOUT(this); }

        D3D12_INPUT_LAYOUT_DESC ProduceDesc(T11On12VertexShader* pVS, std::vector<D3D12_INPUT_ELEMENT_DESC> &InputElements) noexcept(false);

    public:
        std::vector<UINT> m_Registers;
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_APIElements;
    };
}