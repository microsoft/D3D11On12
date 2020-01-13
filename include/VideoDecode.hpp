// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class VideoDecode: public DeviceChild
    {
    public:
        VideoDecode(Device& parent,
                    D3D12TranslationLayer::VideoDecodeCreationArgs const& args,
                    GUID const& bitstreamEncryption);

        static VideoDecode* CastFrom(D3D11_1DDI_HDECODE hVideoDecode) noexcept { return reinterpret_cast<VideoDecode*>(hVideoDecode.pDrvPrivate); }

        static SIZE_T APIENTRY CalcPrivateVideoDecoderSize(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEODECODER*) noexcept;
        static HRESULT APIENTRY CreateVideoDecoder(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEODECODER*, _In_ D3D11_1DDI_HDECODE, _In_ D3D11_1DDI_HRTDECODE) noexcept;
        static void APIENTRY DestroyVideoDecoder(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HDECODE) noexcept;
        static HRESULT APIENTRY VideoDecoderExtension(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HDECODE, _In_ CONST D3D11_1DDIARG_VIDEODECODEREXTENSION*) noexcept;
        static HRESULT APIENTRY VideoDecoderBeginFrame(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HDECODE, _In_ CONST D3DWDDM2_4DDIARG_VIDEODECODERBEGINFRAME*) noexcept;
        static void APIENTRY VideoDecoderEndFrame(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HDECODE) noexcept;
        static HRESULT APIENTRY VideoDecoderSubmitBuffers(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HDECODE hDecode, _In_ UINT BufferCount, _In_reads_(BufferCount) CONST D3D11_1DDI_VIDEO_DECODER_BUFFER_DESC* pBufferDesc) noexcept;
        static HRESULT APIENTRY VideoDecoderSubmitBuffers2(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HDECODE hDecode, _In_ UINT BufferCount, _In_reads_(BufferCount) CONST D3DWDDM2_4DDI_VIDEO_DECODER_BUFFER_DESC* pBufferDesc) noexcept;
        static HRESULT APIENTRY VideoDecoderEnableDownsampling(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HDECODE hDecoder, _In_ D3DDDI_COLOR_SPACE_TYPE InputColorSpace, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace, _In_ UINT ReferenceFrameCount);
        static HRESULT APIENTRY VideoDecoderUpdateDownsampling(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HDECODE hDecoder, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace);

        D3D12TranslationLayer::BatchedVideoDecode* UnderlyingVideoDecode()
        {
            return &m_UnderlyingVideoDecode;
        }

        void BeginFrame(_In_ const D3DWDDM2_4DDIARG_VIDEODECODERBEGINFRAME *pVideoDecoderBeginFrame);
        void SubmitBuffer(_In_ const D3DWDDM2_4DDI_VIDEO_DECODER_BUFFER_DESC *pVideoDecoderBufferDesc);
        void EndFrame();
        void EnableDownsampling(_In_ D3DDDI_COLOR_SPACE_TYPE InputColorSpace, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace, _In_ UINT ReferenceFrameCount);
        void UpdateDownsampling(_In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace);

    protected:
        D3D12TranslationLayer::BatchedVideoDecode m_UnderlyingVideoDecode;
        const GUID m_decodeProfile;
        const GUID m_bitstreamEncryption;
        D3D12TranslationLayer::VIDEO_DECODE_INPUT_STREAM_ARGUMENTS m_inputArguments;
        D3D12TranslationLayer::VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS m_outputArguments;
        struct {
            unique_comptr<D3D12TranslationLayer::Resource> frameArguments[D3D12_VIDEO_DECODE_MAX_ARGUMENTS];
            unique_comptr<D3D12TranslationLayer::Resource> compressedBitstream;
            unique_comptr<D3D12TranslationLayer::Resource> output;
            unique_comptr<D3D12TranslationLayer::Resource> histogramBuffers[D3D12TranslationLayer::VIDEO_DECODE_MAX_HISTOGRAM_COMPONENTS];
        } m_inUseResources;
        UINT m_frameNestCount;
    };
};