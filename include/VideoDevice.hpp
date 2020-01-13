// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class VideoDevice : public DeviceChild
    {
    public:
        VideoDevice(Device& parent);

        void FillVideoDDIFunctions(D3DWDDM2_4DDI_VIDEODEVICEFUNCS* pFuncs);

        // Video DDI Entry points
        static void APIENTRY GetVideoDecoderProfileCount(_In_ D3D10DDI_HDEVICE, _Out_ UINT*);
        static void APIENTRY GetVideoDecoderProfile(_In_ D3D10DDI_HDEVICE, _In_ UINT, _Out_ GUID*);
        static void APIENTRY CheckVideoDecoderFormat(_In_ D3D10DDI_HDEVICE, _In_ CONST GUID*, _In_ DXGI_FORMAT, _Out_ BOOL*);
        static void APIENTRY GetVideoDecoderConfigCount(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC*, _Out_ UINT*);
        static void APIENTRY GetVideoDecoderConfig(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC*, _In_ UINT, _Out_ D3D11_1DDI_VIDEO_DECODER_CONFIG*);
        static void APIENTRY GetVideoDecoderBufferTypeCount(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC*, _Out_ UINT *);
        static void APIENTRY GetVideoDecoderBufferInfo(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC*, _In_ UINT, _Out_ D3D11_1DDI_VIDEO_DECODER_BUFFER_INFO*);

        static SIZE_T APIENTRY CalcPrivateVideoDecoderOutputViewSize(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEODECODEROUTPUTVIEW*);
        static HRESULT APIENTRY CreateVideoDecoderOutputView(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEODECODEROUTPUTVIEW*, _In_ D3D11_1DDI_HVIDEODECODEROUTPUTVIEW, _In_ D3D11_1DDI_HRTVIDEODECODEROUTPUTVIEW);
        static void APIENTRY DestroyVideoDecoderOutputView(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEODECODEROUTPUTVIEW);
        static SIZE_T APIENTRY CalcPrivateVideoProcessorInputViewSize(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORINPUTVIEW*);
        static HRESULT APIENTRY CreateVideoProcessorInputView(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORINPUTVIEW*, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW, _In_ D3D11_1DDI_HRTVIDEOPROCESSORINPUTVIEW);
        static void APIENTRY DestroyVideoProcessorInputView(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW);
        static SIZE_T APIENTRY CalcPrivateVideoProcessorOutputViewSize(_In_ D3D10DDI_HDEVICE, _In_  CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOROUTPUTVIEW*);
        static HRESULT APIENTRY CreateVideoProcessorOutputView(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOROUTPUTVIEW*, _In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW, _In_ D3D11_1DDI_HRTVIDEOPROCESSOROUTPUTVIEW);
        static void APIENTRY DestroyVideoProcessorOutputView(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW);

        static void APIENTRY QueryVideoCapabilities(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3DWDDM2_0DDI_VIDEO_CAPABILITY_QUERY QueryType, _In_ UINT DataSize, _Inout_ VOID* pData);

        static HRESULT APIENTRY GetContentProtectionCaps(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST GUID* pCryptoType, _In_ CONST GUID* pDecodeProfile, _Out_ D3D11_1DDI_VIDEO_CONTENT_PROTECTION_CAPS* pCaps);
        static HRESULT APIENTRY GetCryptoKeyExchangeType(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST GUID* pCryptoType, _In_ CONST GUID* pDecodeProfile, _In_ UINT Index, _Out_ GUID* pKeyExchangeType);
        static void APIENTRY GetCryptoSessionPrivateDataSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST GUID* pCryptoType, _In_opt_ CONST GUID* pDecoderProfile, _In_ CONST GUID* pKeyExchangeType, _Out_ UINT* pPrivateInputSize, _Out_ UINT* pPrivateOutputSize);

#if 0   // TODO: this contains the remaining functions to be implemented. Should be empty when we are done.
        // video processor
        static void APIENTRY VideoProcessorInputViewReadAfterWriteHazard(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW, _In_ D3D10DDI_HRESOURCE);

        static void APIENTRY GetCertificateSize(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDI_CERTIFICATE_INFO*, _Out_ UINT*);
        static void APIENTRY GetCertificate(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDI_CERTIFICATE_INFO*, _In_ UINT CertificateSize, _Out_writes_bytes_(CertificateSize) BYTE*);
        static SIZE_T APIENTRY CalcPrivateAuthenticatedChannelSize(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEAUTHENTICATEDCHANNEL*);
        static HRESULT APIENTRY CreateAuthenticatedChannel(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDIARG_CREATEAUTHENTICATEDCHANNEL*, _In_ D3D11_1DDI_HAUTHCHANNEL, _In_ D3D11_1DDI_HRTAUTHCHANNEL);
        static void APIENTRY DestroyAuthenticatedChannel(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HAUTHCHANNEL);
        static HRESULT APIENTRY NegotiateAuthenticatedChannelKeyExchange(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HAUTHCHANNEL, _In_ UINT DataSize, _Inout_updates_(DataSize) VOID*);
        static HRESULT APIENTRY QueryAuthenticatedChannel(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HAUTHCHANNEL, _In_ UINT InputDataSize, _In_reads_bytes_(InputDataSize) CONST VOID*, _In_ UINT OutputDataSize, _Out_writes_bytes_(OutputDataSize) PVOID);
        static HRESULT APIENTRY ConfigureAuthenticatedChannel(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HAUTHCHANNEL, _In_ UINT InputDataSize, _In_reads_bytes_(InputDataSize) CONST VOID*, _Out_ D3D11_1DDI_AUTHENTICATED_CONFIGURE_OUTPUT*);
        static HRESULT APIENTRY VideoDecoderGetHandle(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HDECODE, _Out_ HANDLE*);
        static void APIENTRY GetCaptureHandle(_In_ D3D10DDI_HDEVICE hDevice, _Inout_ D3D11_1DDI_GETCAPTUREHANDLEDATA* pHandleData);

        static HRESULT APIENTRY VideoProcessorGetBehaviorHints(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcessor, _In_ UINT OutputWidth, _In_ UINT OutputHeight, _In_ DXGI_FORMAT OutputFormat, _In_ UINT StreamCount, _In_reads_(StreamCount) const D3DWDDM2_0DDI_VIDEO_PROCESSOR_STREAM_BEHAVIOR_HINT *pStreams, _Out_ UINT* pBehaviorHints);

        // End Video DDI entry points
#endif
        D3D12TranslationLayer::VideoDevice* UnderlyingVideoDevice()
        {
            return &m_UnderlyingVideoDevice;
        }

    private:
        static bool CheckHardwareDRMSupport(_In_ Device *pDevice, _In_ const GUID* pCryptoType, _In_ const GUID* pDecodeProfile);
        static void GetContentProtectionSystems(_In_ Device *pDevice, _In_ const GUID *pCryptoType, std::vector<GUID> &contentProtectionSystems);
        static void GetDecodeProfiles(_In_ Device *pDevice, _In_ const GUID *pDecodeProfile, std::vector<GUID> &decodeProfiles);

        D3D12TranslationLayer::VideoDevice m_UnderlyingVideoDevice;
    };

};