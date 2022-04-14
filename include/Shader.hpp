// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <dxbcutils.h>
#include <shaderbinary.h>

namespace D3D11On12
{
    //==================================================================================================================================
    // Shader
    //==================================================================================================================================

    template<typename TIface>
    struct CShaderMapper;

    template <typename TIface> struct ShaderDescTraits { using Type = SHADER_DESC; };
    template<> struct ShaderDescTraits<ID3D11GeometryShader> { using Type = GEOMETRY_SHADER_DESC; };

#undef DOMAIN
#define DECLARE_SHADER_MAPPER(Stage, StageCaps, Initial)\
    template<> struct CShaderMapper<ID3D11##Stage##Shader> \
    { \
    typedef ID3D11##Stage##Shader TUnderlyingIface; \
    typedef typename ShaderDescTraits<ID3D11##Stage##Shader>::Type TDesc; \
    static decltype(&ID3D11Device2::Create##Stage##Shader) GetCreate() { return &ID3D11Device2::Create##Stage##Shader; } \
    static constexpr D3D12TranslationLayer::EShaderStage eShaderStage = D3D12TranslationLayer::e_##Initial##S; \
    }
    DECLARE_SHADER_MAPPER(Vertex, VERTEX, V);
    DECLARE_SHADER_MAPPER(Geometry, GEOMETRY, G);
    DECLARE_SHADER_MAPPER(Hull, HULL, H);
    DECLARE_SHADER_MAPPER(Domain, DOMAIN, D);
    DECLARE_SHADER_MAPPER(Pixel, PIXEL, P);
    DECLARE_SHADER_MAPPER(Compute, COMPUTE, C);
#undef DECLARE_SHADER_MAPPER

    template<typename TIface> struct CIOSignatures { typedef D3D11_1DDIARG_STAGE_IO_SIGNATURES TSignatures; };
    template<> struct CIOSignatures<ID3D11DomainShader> { typedef D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES TSignatures; };
    template<> struct CIOSignatures<ID3D11HullShader> { typedef D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES TSignatures; };

    struct SShaderSemantics
    {
        CSignatureParser m_InputSignature;
        CSignatureParser m_OutputSignature;
    };

    template <typename TIface> struct PSOCacheKeyType { typedef PSODescKey KeyType; };
    template <> struct PSOCacheKeyType<ID3D11ComputeShader> { typedef ComputePSOKey KeyType; };

    template<typename TIface>
    class Shader : public PipelineStateCacheKeyComponent<typename PSOCacheKeyType<TIface>::KeyType>
    {
    public: // Types
        typedef typename CShaderMapper<TIface>::TUnderlyingIface TIface11;
        typedef typename CShaderMapper<TIface>::TDesc TDesc;
        typedef typename CIOSignatures<TIface>::TSignatures TSignatures;
        static constexpr auto eShaderStage = CShaderMapper<TIface>::eShaderStage;
        typedef D3D10DDI_HSHADER THandle;
        typedef D3D10DDI_HRTSHADER TRTHandle;

    public: // Methods
        Shader(Device* pDevice, D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader> spUnderlying) noexcept(false);
        virtual ~Shader() noexcept { }

        static Shader<TIface>* CastFrom(THandle handle) noexcept
        {
            return reinterpret_cast<Shader<TIface>*>(handle.pDrvPrivate);
        }

        static SIZE_T TRANSLATION_API CalcPrivateSize(const UINT*, const TSignatures*);
        static void Create(Device* pDevice, THandle, const TDesc*) noexcept(false);

        void ParseInputSemantics() noexcept(false);
        void ParseOutputSemantics() noexcept(false);

        D3D12TranslationLayer::Shader* Underlying() { return m_pUnderlying.get(); }
        void UploadInterfaceDataIfNecessary(D3D12TranslationLayer::BatchedContext& Context, D3D12TranslationLayer::BatchedResource* pResource);

    public:
        bool m_bIsStreamOutput;

        SShaderSemantics m_IOSemantics;

    protected:
        D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader> m_pUnderlying;
    };

    typedef Shader<ID3D11VertexShader> T11On12VertexShader;
    typedef Shader<ID3D11GeometryShader> T11On12GeometryShader;
    typedef Shader<ID3D11HullShader> T11On12HullShader;
    typedef Shader<ID3D11DomainShader> T11On12DomainShader;
    typedef Shader<ID3D11PixelShader> T11On12PixelShader;
    typedef Shader<ID3D11ComputeShader> T11On12ComputeShader;

    class StreamOutShader : public Shader<ID3D11GeometryShader>
    {
    public: // Types
        typedef D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT TDDIDesc;

        StreamOutShader(Device* pDevice, const TDesc* pDesc, const TDDIDesc* pDDIDesc,
            D3D12TranslationLayer::unique_batched_ptr<D3D12TranslationLayer::Shader> spUnderlying) noexcept(false);
        static void Create(Device* pDevice, THandle, const TDesc*, const TDDIDesc* pDDIDesc) noexcept(false);

        static SIZE_T TRANSLATION_API CalcPrivateSize(const TDDIDesc*, const TSignatures*);

        D3D12_STREAM_OUTPUT_DESC ProduceDesc(T11On12VertexShader* pVS, T11On12DomainShader* pDS) noexcept;

    public:
        std::vector<D3D11DDIARG_STREAM_OUTPUT_DECLARATION_ENTRY> m_DDISODecls;
        std::vector<D3D12_SO_DECLARATION_ENTRY> m_APISODecls;
        std::vector<UINT> m_Strides;
        std::vector<std::string> m_Semantics; // Only used if the GS contains the semantics
        UINT m_RasterizedStream;
    };

    class ShaderCache : public DeviceChild
    {
    public:
        static SIZE_T APIENTRY CalcPrivateSize(D3D10DDI_HDEVICE) { return sizeof(ShaderCache); }
        static void APIENTRY Create(D3D10DDI_HDEVICE hDevice, D3DWDDM2_2DDI_HCACHESESSION hThis, D3DWDDM2_2DDI_HRTCACHESESSION hRTSession);
        static ShaderCache* CastFrom(D3DWDDM2_2DDI_HCACHESESSION hThis) { return static_cast<ShaderCache*>(hThis.pDrvPrivate); }
        static void APIENTRY Destroy(D3D10DDI_HDEVICE, D3DWDDM2_2DDI_HCACHESESSION hThis) { CastFrom(hThis)->~ShaderCache(); }

        HRESULT Load(_In_reads_(16) const BYTE* pHash, _Out_writes_bytes_opt_(*pValueLen) void* pValue, _Inout_ SIZE_T* pValueLen);
        HRESULT Store(_In_reads_(16) const BYTE* pHash, _In_reads_bytes_(ValueLen) const void* pValue, SIZE_T ValueLen);

    private:
        D3DWDDM2_2DDI_HRTCACHESESSION m_hRTCacheSession;
        ShaderCache(Device* pDevice, D3DWDDM2_2DDI_HRTCACHESESSION hRTSession)
            : DeviceChild(*pDevice)
            , m_hRTCacheSession(hRTSession)
        {
        }
        ~ShaderCache() = default;
    };
};
