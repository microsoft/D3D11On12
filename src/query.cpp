// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void APIENTRY DeviceBase::QueryBegin(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        Query* pQuery = Query::CastFrom(hQuery);

        pDevice->GetBatchedContext().QueryBegin(&pQuery->m_Underlying);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::QueryEnd(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        Query* pQuery = Query::CastFrom(hQuery);

        pDevice->GetBatchedContext().QueryEnd(&pQuery->m_Underlying);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY Query::QueryGetData(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery, void* pData, UINT DataSize, UINT GetDataFlags)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        Query* pQuery = Query::CastFrom(hQuery);

        HRESULT hr = SynchronizedResultToHRESULT(pDevice->GetBatchedContext().QueryGetData(&pQuery->m_Underlying, pData, DataSize, GetDataFlags & D3D10_DDI_GET_DATA_DO_NOT_FLUSH));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, hr);
    }

    SIZE_T APIENTRY Query::CalcPrivateQuerySize(D3D10DDI_HDEVICE /*hDevice*/, _In_ CONST D3DWDDM2_0DDIARG_CREATEQUERY* /*pCreateQuery*/)
    {
        return sizeof(Query);
    }

    void APIENTRY DeviceBase::SetPredication(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery, BOOL PredicateValue)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        Query* pQuery = Query::CastFrom(hQuery);

        D3D12TranslationLayer::Query* pUnderlying = (pQuery) ? (D3D12TranslationLayer::Query*)pQuery->m_Underlying.GetImmediateNoFlush() : nullptr;

        pDevice->GetBatchedContext().SetPredication(pUnderlying, PredicateValue);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    static inline D3D12TranslationLayer::EQueryType ConvertQueryType(D3D10DDI_QUERY queryType)
    {
        switch (queryType)
        {
            case D3D10DDI_QUERY_EVENT :
                return D3D12TranslationLayer::EQueryType::e_QUERY_EVENT;
            case D3D10DDI_QUERY_OCCLUSION :
                return D3D12TranslationLayer::EQueryType::e_QUERY_OCCLUSION;
            case D3D10DDI_QUERY_TIMESTAMP :
                return D3D12TranslationLayer::EQueryType::e_QUERY_TIMESTAMP;
            case D3D10DDI_QUERY_TIMESTAMPDISJOINT :
                return D3D12TranslationLayer::EQueryType::e_QUERY_TIMESTAMPDISJOINT;
            case D3D10DDI_QUERY_PIPELINESTATS :
                return D3D12TranslationLayer::EQueryType::e_QUERY_PIPELINESTATS;
            case D3D10DDI_QUERY_OCCLUSIONPREDICATE :
                return D3D12TranslationLayer::EQueryType::e_QUERY_OCCLUSIONPREDICATE;
            case D3D10DDI_QUERY_STREAMOUTPUTSTATS :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOUTPUTSTATS;
            case D3D10DDI_QUERY_STREAMOVERFLOWPREDICATE :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOVERFLOWPREDICATE;
            case D3D11DDI_QUERY_PIPELINESTATS :
                return D3D12TranslationLayer::EQueryType::e_QUERY_PIPELINESTATS;
            case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM0 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOUTPUTSTATS_STREAM0;
            case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM1 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOUTPUTSTATS_STREAM1;
            case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM2 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOUTPUTSTATS_STREAM2;
            case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM3 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOUTPUTSTATS_STREAM3;
            case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM0 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOVERFLOWPREDICATE_STREAM0;
            case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM1 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOVERFLOWPREDICATE_STREAM1;
            case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM2 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOVERFLOWPREDICATE_STREAM2;
            case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM3 :
                return D3D12TranslationLayer::EQueryType::e_QUERY_STREAMOVERFLOWPREDICATE_STREAM3;
            case D3D10DDI_COUNTER_GPU_IDLE :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_GPU_IDLE;
            case D3D10DDI_COUNTER_VERTEX_PROCESSING :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_VERTEX_PROCESSING;
            case D3D10DDI_COUNTER_GEOMETRY_PROCESSING :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_GEOMETRY_PROCESSING;
            case D3D10DDI_COUNTER_PIXEL_PROCESSING :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_PIXEL_PROCESSING;
            case D3D10DDI_COUNTER_OTHER_GPU_PROCESSING :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_OTHER_GPU_PROCESSING;
            case D3D10DDI_COUNTER_HOST_ADAPTER_BANDWIDTH_UTILIZATION :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_HOST_ADAPTER_BANDWIDTH_UTILIZATION;
            case D3D10DDI_COUNTER_LOCAL_VIDMEM_BANDWIDTH_UTILIZATION :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_LOCAL_VIDMEM_BANDWIDTH_UTILIZATION;
            case D3D10DDI_COUNTER_VERTEX_THROUGHPUT_UTILIZATION :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_VERTEX_THROUGHPUT_UTILIZATION;
            case D3D10DDI_COUNTER_TRISETUP_THROUGHPUT_UTILIZATION :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_TRISETUP_THROUGHPUT_UTILIZATION;
            case D3D10DDI_COUNTER_FILLRATE_THROUGHPUT_UTILIZATION :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_FILLRATE_THROUGHPUT_UTILIZATION;
            case D3D10DDI_COUNTER_VERTEXSHADER_MEMORY_LIMITED :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_VERTEXSHADER_MEMORY_LIMITED;
            case D3D10DDI_COUNTER_VERTEXSHADER_COMPUTATION_LIMITED :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_VERTEXSHADER_COMPUTATION_LIMITED;
            case D3D10DDI_COUNTER_GEOMETRYSHADER_MEMORY_LIMITED :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_GEOMETRYSHADER_MEMORY_LIMITED;
            case D3D10DDI_COUNTER_GEOMETRYSHADER_COMPUTATION_LIMITED :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_GEOMETRYSHADER_COMPUTATION_LIMITED;
            case D3D10DDI_COUNTER_PIXELSHADER_MEMORY_LIMITED :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_PIXELSHADER_MEMORY_LIMITED;
            case D3D10DDI_COUNTER_PIXELSHADER_COMPUTATION_LIMITED :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_PIXELSHADER_COMPUTATION_LIMITED;
            case D3D10DDI_COUNTER_POST_TRANSFORM_CACHE_HIT_RATE :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_POST_TRANSFORM_CACHE_HIT_RATE;
            case D3D10DDI_COUNTER_TEXTURE_CACHE_HIT_RATE :
                return D3D12TranslationLayer::EQueryType::e_COUNTER_TEXTURE_CACHE_HIT_RATE;
            default:
                assert(false);
                return D3D12TranslationLayer::EQueryType::e_QUERY_EVENT;
        }
    }

    VOID APIENTRY Query::CreateQuery(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATEQUERY* pCreateQuery, D3D10DDI_HQUERY hQuery, D3D10DDI_HRTQUERY /*hRTQuery*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        UINT commandListTypeMask = Device::CommandListTypeMask(pCreateQuery->ContextType);

        std::unique_ptr<D3D12TranslationLayer::Async> spAsync;
        switch (pCreateQuery->Query)
        {
        case D3D10DDI_QUERY_EVENT:
            spAsync.reset(new D3D12TranslationLayer::EventQuery(&pDevice->GetImmediateContextNoFlush(), commandListTypeMask));
            break;
        case D3D10DDI_QUERY_TIMESTAMPDISJOINT:
            spAsync.reset(new D3D12TranslationLayer::TimestampDisjointQuery(&pDevice->GetImmediateContextNoFlush(), commandListTypeMask));
            break;
        case D3D10DDI_QUERY_TIMESTAMP:
        case D3D10DDI_QUERY_OCCLUSION:
        case D3D10DDI_QUERY_STREAMOUTPUTSTATS:
        case D3D11DDI_QUERY_PIPELINESTATS:
        case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM0:
        case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM1:
        case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM2:
        case D3D11DDI_QUERY_STREAMOUTPUTSTATS_STREAM3:
        case D3D10DDI_QUERY_OCCLUSIONPREDICATE:
        case D3D10DDI_QUERY_STREAMOVERFLOWPREDICATE:
        case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM0:
        case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM1:
        case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM2:
        case D3D11DDI_QUERY_STREAMOVERFLOWPREDICATE_STREAM3: 
            spAsync.reset(new D3D12TranslationLayer::Query(&pDevice->GetImmediateContextNoFlush(), ConvertQueryType(pCreateQuery->Query), commandListTypeMask));
            break;
        default:
            assert(false); // counters are not supported
            ThrowFailure(E_OUTOFMEMORY);
        }

        spAsync->Initialize();

        // No more exceptions
        new (hQuery.pDrvPrivate) Query(*pDevice, std::move(spAsync));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Query::CreateDeferred(D3D10DDI_HDEVICE hDevice, _In_ CONST D3DWDDM2_0DDIARG_CREATEQUERY*, D3D10DDI_HQUERY hQuery, D3D10DDI_HRTQUERY hRTQuery)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        DeviceDeferred* pDevice = DeviceDeferred::CastFrom(hDevice);
        Query* pBaseQuery = CastFrom(MAKE_D3D10DDI_HQUERY(hRTQuery.handle));
        new (hQuery.pDrvPrivate) Query(*pDevice, pBaseQuery);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VOID APIENTRY Query:: DestroyQuery(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        Query* pQuery = Query::CastFrom(hQuery);
        pQuery->~Query();

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}