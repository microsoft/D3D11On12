// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class Query : public DeviceChild
    {
    public:
        Query(Device& parent, std::unique_ptr<D3D12TranslationLayer::Async> spAsync)
            : DeviceChild(parent)
            , m_Underlying(parent.GetBatchedContext(), spAsync.release(), true /*ownsAsync*/)
        {
        }
        Query(DeviceDeferred& parent, Query* pBaseAsync)
            : DeviceChild(*parent.m_pBaseDevice)
            , m_Underlying(parent.GetBatchedContext(), pBaseAsync->m_Underlying.GetImmediateNoFlush(), false /*ownsAsync*/)
        {
        }

        static Query* CastFrom(D3D10DDI_HQUERY Handle) { return static_cast<Query*>(Handle.pDrvPrivate); }

        static void APIENTRY QueryGetData(D3D10DDI_HDEVICE, D3D10DDI_HQUERY, void*, UINT, UINT);
        static SIZE_T APIENTRY CalcPrivateQuerySize(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATEQUERY*);
        static VOID APIENTRY  CreateQuery(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATEQUERY*, D3D10DDI_HQUERY, D3D10DDI_HRTQUERY);
        static VOID APIENTRY  CreateDeferred(D3D10DDI_HDEVICE, _In_ CONST D3DWDDM2_0DDIARG_CREATEQUERY*, D3D10DDI_HQUERY, D3D10DDI_HRTQUERY);
        static VOID APIENTRY  DestroyQuery(D3D10DDI_HDEVICE, D3D10DDI_HQUERY);

        D3D12TranslationLayer::BatchedQuery m_Underlying;
    };
};