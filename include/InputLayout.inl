// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    //----------------------------------------------------------------------------------------------------------------------------------
    inline InputLayout::InputLayout(Device* pDevice, const D3D10DDIARG_CREATEELEMENTLAYOUT* pDesc) noexcept(false)
        : PipelineStateCacheKeyComponent(*pDevice)
    {
        m_Registers.resize(pDesc->NumElements); // throw( bad_alloc )
        m_APIElements.resize(pDesc->NumElements); // throw( bad_alloc )
        for (UINT i = 0; i < pDesc->NumElements; ++i)
        {
            m_Registers[i] = pDesc->pVertexElements[i].InputRegister;

            m_APIElements[i].AlignedByteOffset = pDesc->pVertexElements[i].AlignedByteOffset;
            m_APIElements[i].Format = pDesc->pVertexElements[i].Format;
            m_APIElements[i].InputSlot = pDesc->pVertexElements[i].InputSlot;
            m_APIElements[i].InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(pDesc->pVertexElements[i].InputSlotClass);
            m_APIElements[i].InstanceDataStepRate = pDesc->pVertexElements[i].InstanceDataStepRate;
            // Semantic is late-filled by VS input decl in GetDesc()
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    inline D3D12_INPUT_LAYOUT_DESC InputLayout::ProduceDesc(T11On12VertexShader* pVS, std::vector<D3D12_INPUT_ELEMENT_DESC> &InputElements) noexcept(false)
    {
        InputElements.clear();

        // Find the semantics in the input signature
        if (!pVS)
        {
            return{};
        }

        // Get the semantics that the VS expects
        const D3D11_SIGNATURE_PARAMETER* pParameters;
        UINT NumParameters = pVS->m_IOSemantics.m_InputSignature.GetParameters(&pParameters);

        // Find each input element's semantic by matching registers
        for (size_t i = 0; i < m_APIElements.size(); ++i)
        {
            UINT Register = m_Registers[i];
            auto iter = std::find_if(pParameters, pParameters + NumParameters,
                [=](D3D11_SIGNATURE_PARAMETER const& Param) { return Param.Register == Register; });

            if (iter == pParameters + NumParameters)
            {
                continue;
            }
            else
            {
                InputElements.push_back(m_APIElements[i]); // throw( bad_alloc )
                auto& Element = InputElements.back();
                Element.SemanticName = iter->SemanticName;
                Element.SemanticIndex = iter->SemanticIndex;
            }
        }

        D3D12_INPUT_LAYOUT_DESC Desc;
        Desc.pInputElementDescs = InputElements.data();
        Desc.NumElements = static_cast<UINT>(InputElements.size());

        return Desc;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    inline InputLayout* InputLayout::CastFrom(D3D10DDI_HELEMENTLAYOUT handle) noexcept
    {
        if (handle.pDrvPrivate == nullptr) return nullptr;
        // CDDIElementLayout has no ValidThis()
        return reinterpret_cast<InputLayout*>(handle.pDrvPrivate);
    }
}