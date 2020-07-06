// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"
#include <dxc\Dxcapi.h>
#include <DxbcConverter.h>
#include <dxc\DxilContainer\dxilpipelinestatevalidation.h>

namespace D3D11On12
{
    extern decltype(&DxcCreateInstance) pfnDxcCreateInstance;
    struct DxilConvLoader
    {
        static DxilConvLoader s_this;
        std::mutex m_lock;
        bool m_bTriedLoad = false;
        HMODULE m_hDxilConv = nullptr;
        static HRESULT __stdcall DxcCreateInstance(REFCLSID rclsid, REFIID riid, void** ppv)
        {
            {
                std::lock_guard lock(s_this.m_lock);
                if (s_this.m_bTriedLoad)
                {
                    if (pfnDxcCreateInstance == &DxcCreateInstance)
                    {
                        return E_NOTIMPL;
                    }
                }
                else
                {
                    s_this.m_bTriedLoad = true;
                    s_this.m_hDxilConv = LoadLibraryW(L"dxilconv.dll");
                    if (!s_this.m_hDxilConv)
                    {
                        return HRESULT_FROM_WIN32(GetLastError());
                    }
                    pfnDxcCreateInstance = reinterpret_cast<decltype(&::DxcCreateInstance)>(GetProcAddress(s_this.m_hDxilConv, "DxcCreateInstance"));
                    if (!pfnDxcCreateInstance)
                    {
                        pfnDxcCreateInstance = &DxcCreateInstance;
                        return HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }
            return pfnDxcCreateInstance(rclsid, riid, ppv);
        }
        ~DxilConvLoader()
        {
            if (m_hDxilConv)
            {
                FreeLibrary(m_hDxilConv);
                m_hDxilConv = nullptr;
            }
        }
    };
    DxilConvLoader DxilConvLoader::s_this;
    decltype(&DxcCreateInstance) pfnDxcCreateInstance = &DxilConvLoader::DxcCreateInstance;

    SIZE_T APIENTRY Device::CalcPrivateShaderSize(D3D10DDI_HDEVICE hDevice, _In_reads_(pShaderCode[1]) CONST UINT* pShaderCode, _In_ CONST D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        const SIZE_T shaderSize = T11On12VertexShader::CalcPrivateSize(pShaderCode, pSignatures);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, shaderSize);
    }

    SIZE_T APIENTRY Device::CalcPrivateGeometryShaderWithStreamOutput(D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT* pCreateGeoShaderWithSO, _In_ CONST D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        const SIZE_T shaderSize = StreamOutShader::CalcPrivateSize(pCreateGeoShaderWithSO, pSignatures);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, shaderSize);

    }

    SIZE_T APIENTRY Device::CalcPrivateTessellationShaderSize(D3D10DDI_HDEVICE hDevice, _In_reads_(pShaderCode[1]) CONST UINT* pShaderCode, _In_ CONST D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        const SIZE_T shaderSize = T11On12HullShader::CalcPrivateSize(pShaderCode, pSignatures);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, shaderSize);
    }

    VOID APIENTRY Device::DestroyShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        T11On12VertexShader* pShader = Shader<ID3D11VertexShader>::CastFrom(hShader);
        pShader->~Shader();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    HRESULT APIENTRY Device::RetrieveShaderComment(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER /*hShader*/, _Out_writes_z_(*CharacterCountIncludingNullTerminator) WCHAR * /*pBuffer*/, _Inout_ SIZE_T * /*CharacterCountIncludingNullTerminator*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        pDevice->ReportError(E_NOTIMPL);

        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    void StreamOutShader::Create(Device* pDevice, THandle handle, const TDesc* pDesc, const TDDIDesc* pDDIDesc) noexcept(false)
    {
        new (handle.pDrvPrivate) StreamOutShader(pDevice, pDesc, pDDIDesc, CreateUnderlyingShader(pDevice, pDDIDesc->pShaderCode ? pDesc : nullptr)); // throw( _com_error, bad_alloc )
    }

    StreamOutShader::StreamOutShader(Device* pDevice, const TDesc* pDesc, const TDDIDesc* pDDIDesc,
        D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader> spUnderlying) noexcept(false)
        : Shader(pDevice, std::move(spUnderlying)) // throw( _com_error, bad_alloc )
    {
        m_bIsStreamOutput = true;
        bool bIsGS = pDDIDesc->pShaderCode != nullptr;
        m_APISODecls.resize(pDesc->NumElements); // throw( bad_alloc )
        m_Strides.assign(pDesc->pBufferStrides, pDesc->pBufferStrides + pDesc->NumStrides); // throw( bad_alloc )
        if (bIsGS)
        {
            m_Semantics.reserve(pDesc->NumElements); // throw( bad_alloc )
        }
        else
        {
            m_DDISODecls.resize(pDDIDesc->NumEntries); // throw( bad_alloc )
        }
        for (UINT i = 0; i < pDesc->NumElements; ++i)
        {
            m_APISODecls[i] = reinterpret_cast<const D3D12_SO_DECLARATION_ENTRY*>(pDesc->pDeclaration)[i];

            // Null semantic indicates a gap
            if (bIsGS && m_APISODecls[i].SemanticName)
            {
                // When the GS is used to construct the stream out desc, we know the semantics are valid,
                // because we will provide the same bytecode to D3D12. Store them here now.
                m_Semantics.emplace_back(m_APISODecls[i].SemanticName); // throw( bad_alloc )
                m_APISODecls[i].SemanticName = m_Semantics.back().c_str();
            }
            else
            {
                if (!bIsGS)
                {
                    m_DDISODecls[i] = pDDIDesc->pOutputStreamDecl[i];
                }
                // Semantic is late-filled by VS/DS output decl in GetDesc()
                m_APISODecls[i].SemanticName = nullptr;
            }
        }

        m_RasterizedStream = pDesc->RasterizedStream;
        // If the specified rasterized stream is not output by the shader
        // then change it to D3D12_SO_NO_RASTERIZED_STREAM.
        // D3D12 has tighter validation than D3D11 here.
        if (m_RasterizedStream != D3D12_SO_NO_RASTERIZED_STREAM)
        {
            UINT OutputStreamMask = 1; // If there's no underlying shader, then there's only 1 stream.
            if (m_pUnderlying)
            {
                OutputStreamMask = m_pUnderlying->OutputStreamMask();
            }
            if (0 == ((1 << m_RasterizedStream) & OutputStreamMask))
            {
                m_RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;
            }
        }
    }

    D3D12_STREAM_OUTPUT_DESC StreamOutShader::ProduceDesc(T11On12VertexShader* pVS, T11On12DomainShader* pDS) noexcept
    {
        D3D12_STREAM_OUTPUT_DESC SODesc;
        SODesc.NumEntries = static_cast<UINT>(m_APISODecls.size());
        SODesc.pSODeclaration = m_APISODecls.data();
        SODesc.NumStrides = static_cast<UINT>(m_Strides.size());
        SODesc.pBufferStrides = m_Strides.data();
        SODesc.RasterizedStream = m_RasterizedStream;

        if (m_Semantics.capacity() > 0)
        {
            // Semantics came from the GS
            return SODesc;
        }

        auto MakeUnused = [](D3D12_SO_DECLARATION_ENTRY& APIDecl)
        {
            // Treat it as more gap at the API
            APIDecl.SemanticName = nullptr;
            APIDecl.SemanticIndex = 0;
        };
        auto MakeAllUnused = [this, &MakeUnused]()
        {
            for (auto& APIDecl : m_APISODecls) { MakeUnused(APIDecl); }
        };

        // Find the semantics in the VS/DS output signature
        const D3D11_SIGNATURE_PARAMETER* pParameters = nullptr;
        UINT NumParameters = 0;
        if (pDS)
        {
            NumParameters = pDS->m_IOSemantics.m_OutputSignature.GetParameters(&pParameters);
        }
        else if (pVS)
        {
            NumParameters = pVS->m_IOSemantics.m_OutputSignature.GetParameters(&pParameters);
        }
        else
        {
            MakeAllUnused();
            return SODesc;
        }

        // API and DDI decls might be different sizes, because the DDI only allows gaps 4 components at a time
        // Walk the DDI decls, and increment the API decl when: 1. We have a non-gap, or
        // 2. When we've consumed all the DDI decls related to an API gap
        UINT CurrentAPIDecl = 0, CurrentDDIDecl = 0;
        UINT CurrentGapOffset = 0;
        for (; CurrentAPIDecl < m_APISODecls.size() && CurrentDDIDecl < m_DDISODecls.size(); ++CurrentDDIDecl)
        {
            auto& APIDecl = m_APISODecls[CurrentAPIDecl];
            auto& DDIDecl = m_DDISODecls[CurrentDDIDecl];
            if (DDIDecl.RegisterIndex == D3D11_SO_DDI_REGISTER_INDEX_DENOTING_GAP)
            {
                assert(APIDecl.SemanticName == nullptr);
                ULONG GapSize = 0;
                unsigned char BitScanResult = _BitScanForward(&GapSize, DDIDecl.RegisterMask + 1);
                UNREFERENCED_PARAMETER(BitScanResult);
                assert(BitScanResult != 0);
                CurrentGapOffset += GapSize;
                if (CurrentGapOffset == APIDecl.ComponentCount)
                {
                    ++CurrentAPIDecl;
                    CurrentGapOffset = 0;
                }
                continue;
            }
            else
            {
                // Iteration should match up these descs
                assert(APIDecl.OutputSlot == DDIDecl.OutputSlot);
                assert(APIDecl.Stream == DDIDecl.Stream);

                UINT Register = DDIDecl.RegisterIndex;
                UINT RegisterMask = DDIDecl.RegisterMask;

                // It's only a matching semantic if the semantic covers at least
                // the components required by the register mask
                auto iter = std::find_if(pParameters, pParameters + NumParameters,
                    [=](D3D11_SIGNATURE_PARAMETER const& Param)
                {
                    return Param.Register == Register &&
                        (Param.Mask & RegisterMask) == RegisterMask;
                });

                if (iter == pParameters + NumParameters)
                {
                    MakeUnused(APIDecl);
                }
                else
                {
                    APIDecl.SemanticName = iter->SemanticName;
                    APIDecl.SemanticIndex = iter->SemanticIndex;
                }
                ++CurrentAPIDecl;
            }
        }

        return SODesc;
    }

    inline SIZE_T TRANSLATION_API StreamOutShader::CalcPrivateSize(const TDDIDesc*, const TSignatures*)
    {
        return sizeof(StreamOutShader);
    }

    void ConvertShaderToDxil(
        Device* pDevice, std::unique_ptr<BYTE[]>& spBytecode, SIZE_T bytecodeSize, bool bInterfaces,
        D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader>& spCreatedShader) noexcept(false)
    {
        const DXBCHash* pHash = nullptr;
        D3D12TranslationLayer::SShaderDecls Decls;
        {
            // Parse the original DXBC bytecode to get the shader decls
            CDXBCParser DXBCParser;
            ThrowFailure(DXBCParser.ReadDXBCAssumingValidSize(spBytecode.get()));
            UINT BlobIndex = DXBCParser.FindNextMatchingBlob(DXBC_GenericShaderEx, 0);
            if (DXBC_BLOB_NOT_FOUND == BlobIndex)
            {
                BlobIndex = DXBCParser.FindNextMatchingBlob(DXBC_GenericShader, 0);
            }
            if (DXBC_BLOB_NOT_FOUND == BlobIndex)
            {
                ThrowFailure(E_FAIL);
            }
            pHash = DXBCParser.GetHash();
            const UINT* pDriverBytecode = (const UINT *)DXBCParser.GetBlob(BlobIndex);
            Decls.Parse(pDriverBytecode);
        }

        auto pCache = pDevice->GetShaderCache();
        CComHeapPtr<void> spDxil;
        UINT DxilSize = 0;
        if (pCache)
        {
            SIZE_T DxilSizeSizeT = 0;
            if (SUCCEEDED(pCache->Load(pHash->Digest, nullptr, &DxilSizeSizeT)))
            {
                if (!spDxil.AllocateBytes(DxilSizeSizeT))
                {
                    throw std::bad_alloc();
                }
                ThrowFailure(pCache->Load(pHash->Digest, spDxil.m_pData, &DxilSizeSizeT));
                DxilSize = (UINT)DxilSizeSizeT;
            }
        }
        if (spDxil.m_pData == nullptr)
        {
            CComPtr<IDxbcConverter> spConverter;
            ThrowFailure(pfnDxcCreateInstance(CLSID_DxbcConverter, IID_PPV_ARGS(&spConverter)));

            ThrowFailure(spConverter->Convert(spBytecode.get(), (UINT)bytecodeSize, nullptr, &spDxil, &DxilSize, nullptr));

#if 0
            static UniqueHMODULE hDXIL(L"dxil.dll");
            if (hDXIL)
            {
                auto pfnCreateValidator = reinterpret_cast<decltype(&DxcCreateInstance)>(GetProcAddress(hDXIL, "DxcCreateInstance"));
                CComPtr<IDxcValidator> spValidator;
                ThrowFailure(pfnCreateValidator(CLSID_DxcValidator, IID_PPV_ARGS(&spValidator)));
                struct Blob : IDxcBlob
                {
                    void* pBlob;
                    UINT Size;
                    Blob(void* p, UINT s) : pBlob(p), Size(s) { }
                    STDMETHOD(QueryInterface)(REFIID, void** ppv) { *ppv = this; return S_OK; }
                    STDMETHOD_(ULONG, AddRef)() { return 1; }
                    STDMETHOD_(ULONG, Release)() { return 0; }
                    STDMETHOD_(void*, GetBufferPointer)() override { return pBlob; }
                    STDMETHOD_(SIZE_T, GetBufferSize)() override { return Size; }
                } Blob = { spDxil.m_pData, DxilSize };
                CComPtr<IDxcOperationResult> spResult;
                ThrowFailure(spValidator->Validate(&Blob, 0, &spResult));
                HRESULT hrStatus = S_OK;
                ThrowFailure(spResult->GetStatus(&hrStatus));
                assert(SUCCEEDED(hrStatus));
            }
#endif

            if (pCache)
            {
                (void)pCache->Store(pHash->Digest, spDxil.m_pData, DxilSize);
            }
        }

        if (bInterfaces)
        {
            Decls.m_NumCBs = 15;
            Decls.m_NumSamplers = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            Decls.m_ResourceDecls.resize(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, D3D12TranslationLayer::RESOURCE_DIMENSION::UNKNOWN);
            Decls.m_bUsesInterfaces = true;
        }
        Decls.m_NumSRVSpacesUsed = 1;

        if (bInterfaces)
        {
            // Pull out the DXIL validation info to figure out how many SRVs need to be bound
            CDXBCParser DXBCParser;
            ThrowFailure(DXBCParser.ReadDXBCAssumingValidSize(spDxil.m_pData));

#define DXBC_FOURCC(ch0, ch1, ch2, ch3)                              \
            ((UINT)(BYTE)(ch0) | ((UINT)(BYTE)(ch1) << 8) |   \
            ((UINT)(BYTE)(ch2) << 16) | ((UINT)(BYTE)(ch3) << 24 ))

            const UINT DXBC_PipelineStateValidation = DXBC_FOURCC('P','S','V','0');
            UINT BlobIndex = DXBCParser.FindNextMatchingBlob((DXBCFourCC)DXBC_PipelineStateValidation);
            if (DXBC_BLOB_NOT_FOUND != BlobIndex)
            {
                const void* pValidationBits = DXBCParser.GetBlob(BlobIndex);
                UINT ValidationBitsSize = DXBCParser.GetBlobSize(BlobIndex);
                DxilPipelineStateValidation Validation;
                Validation.InitFromPSV0(pValidationBits, ValidationBitsSize);

                for (UINT i = 0; i < Validation.GetBindCount(); ++i)
                {
                    auto pResource = Validation.GetPSVResourceBindInfo0(i);
                    if (pResource->ResType == (UINT)PSVResourceType::SRVRaw ||
                        pResource->ResType == (UINT)PSVResourceType::SRVTyped ||
                        pResource->ResType == (UINT)PSVResourceType::SRVStructured)
                    {
                        Decls.m_NumSRVSpacesUsed = std::max(Decls.m_NumSRVSpacesUsed, pResource->Space + 1);
                    }
                }
            }
        }

        spCreatedShader.reset(new D3D12TranslationLayer::Shader(
            &pDevice->GetImmediateContextNoFlush(), std::move(spBytecode), spDxil, DxilSize, std::move(Decls)));
    }

    void ShaderCache::Create(D3D10DDI_HDEVICE hDevice, D3DWDDM2_2DDI_HCACHESESSION hThis, D3DWDDM2_2DDI_HRTCACHESESSION hRTSession)
    {
        new (hThis.pDrvPrivate) ShaderCache(Device::CastFrom(hDevice), hRTSession);
    }

    _Use_decl_annotations_
    HRESULT ShaderCache::Load(const BYTE* pHash, void* pValue, SIZE_T* pValueSize)
    {
        return m_parentDevice.GetRuntimeCallbacks()->pfnShaderCacheGetValueCb(
            m_hRTCacheSession, reinterpret_cast<const D3DWDDM2_2DDI_SHADERCACHE_HASH*>(pHash), pHash, 16, pValue, pValueSize);
    }

    _Use_decl_annotations_
    HRESULT ShaderCache::Store(const BYTE* pHash, const void* pValue, SIZE_T ValueSize)
    {
        return m_parentDevice.GetRuntimeCallbacks()->pfnShaderCacheStoreValueCb(
            m_hRTCacheSession, reinterpret_cast<const D3DWDDM2_2DDI_SHADERCACHE_HASH*>(pHash), pHash, 16, pValue, ValueSize);
    }
}