// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    void SetStreamRotationDummy(D3D10DDI_HDEVICE , D3D11_1DDI_HVIDEOPROCESSOR , UINT , BOOL , D3D11_1DDI_VIDEO_PROCESSOR_ROTATION )
    {
        // TODO: remove. For now, needs to be present to avoid crash in runtime.
    }

    void VideoDevice::FillVideoDDIFunctions(D3DWDDM2_4DDI_VIDEODEVICEFUNCS *pFuncs)
    {
        ZeroMemory(pFuncs, sizeof(*pFuncs));

        // video device functionality
        pFuncs->pfnGetVideoDecoderProfileCount = GetVideoDecoderProfileCount;
        pFuncs->pfnGetVideoDecoderProfile = GetVideoDecoderProfile;
        pFuncs->pfnCheckVideoDecoderFormat = CheckVideoDecoderFormat;
        pFuncs->pfnGetVideoDecoderConfigCount = GetVideoDecoderConfigCount;
        pFuncs->pfnGetVideoDecoderConfig = GetVideoDecoderConfig;
        pFuncs->pfnGetVideoDecoderBufferTypeCount = GetVideoDecoderBufferTypeCount;
        pFuncs->pfnGetVideoDecoderBufferInfo = GetVideoDecoderBufferInfo;
        pFuncs->pfnQueryVideoCapabilities = QueryVideoCapabilities;
        pFuncs->pfnVideoDecoderEnableDownsampling = VideoDecode::VideoDecoderEnableDownsampling;
        pFuncs->pfnVideoDecoderUpdateDownsampling = VideoDecode::VideoDecoderUpdateDownsampling;

        // video decoder
        pFuncs->pfnCalcPrivateVideoDecoderSize = VideoDecode::CalcPrivateVideoDecoderSize;
        pFuncs->pfnCreateVideoDecoder = VideoDecode::CreateVideoDecoder;
        pFuncs->pfnDestroyVideoDecoder = VideoDecode::DestroyVideoDecoder;
        pFuncs->pfnVideoDecoderExtension = VideoDecode::VideoDecoderExtension;
        pFuncs->pfnVideoDecoderBeginFrame = VideoDecode::VideoDecoderBeginFrame;
        pFuncs->pfnVideoDecoderEndFrame = VideoDecode::VideoDecoderEndFrame;
        pFuncs->pfnVideoDecoderSubmitBuffers = VideoDecode::VideoDecoderSubmitBuffers;
        pFuncs->pfnVideoDecoderSubmitBuffers2 = VideoDecode::VideoDecoderSubmitBuffers2;

        // view functionality
        pFuncs->pfnCalcPrivateVideoDecoderOutputViewSize = VideoDevice::CalcPrivateVideoDecoderOutputViewSize;
        pFuncs->pfnCreateVideoDecoderOutputView = VideoDevice::CreateVideoDecoderOutputView;
        pFuncs->pfnDestroyVideoDecoderOutputView = VideoDevice::DestroyVideoDecoderOutputView;
        pFuncs->pfnCalcPrivateVideoProcessorInputViewSize = VideoDevice::CalcPrivateVideoProcessorInputViewSize;
        pFuncs->pfnCreateVideoProcessorInputView = VideoDevice::CreateVideoProcessorInputView;
        pFuncs->pfnDestroyVideoProcessorInputView = VideoDevice::DestroyVideoProcessorInputView;
        pFuncs->pfnCalcPrivateVideoProcessorOutputViewSize = VideoDevice::CalcPrivateVideoProcessorOutputViewSize;
        pFuncs->pfnCreateVideoProcessorOutputView = VideoDevice::CreateVideoProcessorOutputView;
        pFuncs->pfnDestroyVideoProcessorOutputView = VideoDevice::DestroyVideoProcessorOutputView;

        // video processor enumerator
        pFuncs->pfnCalcPrivateVideoProcessorEnumSize = VideoProcessEnum::CalcPrivateVideoProcessorEnumSize;
        pFuncs->pfnCreateVideoProcessorEnum = VideoProcessEnum::CreateVideoProcessorEnum;
        pFuncs->pfnDestroyVideoProcessorEnum = VideoProcessEnum::DestroyVideoProcessorEnum;
        pFuncs->pfnCheckVideoProcessorFormat = VideoProcessEnum::CheckVideoProcessorFormat;
        pFuncs->pfnGetVideoProcessorCaps = VideoProcessEnum::GetVideoProcessorCaps;
        pFuncs->pfnGetVideoProcessorRateConversionCaps = VideoProcessEnum::GetVideoProcessorRateConversionCaps;
        pFuncs->pfnGetVideoProcessorCustomRate = VideoProcessEnum::GetVideoProcessorCustomRate;
        pFuncs->pfnGetVideoProcessorFilterRange = VideoProcessEnum::GetVideoProcessorFilterRange;
        pFuncs->pfnCheckVideoProcessorFormatConversion = VideoProcessEnum::CheckVideoProcessorFormatConversion;

        // video processor
        pFuncs->pfnCalcPrivateVideoProcessorSize = VideoProcess::CalcPrivateVideoProcessorSize;
        pFuncs->pfnCreateVideoProcessor = VideoProcess::CreateVideoProcessor;
        pFuncs->pfnDestroyVideoProcessor = VideoProcess::DestroyVideoProcessor;

        pFuncs->pfnVideoProcessorSetOutputTargetRect = VideoProcess::VideoProcessorSetOutputTargetRect;
        pFuncs->pfnVideoProcessorSetOutputBackgroundColor = VideoProcess::VideoProcessorSetOutputBackgroundColor;
        pFuncs->pfnVideoProcessorSetOutputColorSpace = nullptr;
        pFuncs->pfnVideoProcessorSetOutputAlphaFillMode = VideoProcess::VideoProcessorSetOutputAlphaFillMode;
        pFuncs->pfnVideoProcessorSetOutputConstriction = VideoProcess::VideoProcessorSetOutputConstriction;
        pFuncs->pfnVideoProcessorSetOutputStereoMode = VideoProcess::VideoProcessorSetOutputStereoMode;
        pFuncs->pfnVideoProcessorSetOutputExtension = VideoProcess::VideoProcessorSetOutputExtension;
        pFuncs->pfnVideoProcessorGetOutputExtension = VideoProcess::VideoProcessorGetOutputExtension;
        pFuncs->pfnVideoProcessorSetStreamFrameFormat = VideoProcess::VideoProcessorSetStreamFrameFormat;
        pFuncs->pfnVideoProcessorSetStreamColorSpace = nullptr;
        pFuncs->pfnVideoProcessorSetStreamOutputRate = VideoProcess::VideoProcessorSetStreamOutputRate;
        pFuncs->pfnVideoProcessorSetStreamSourceRect = VideoProcess::VideoProcessorSetStreamSourceRect;
        pFuncs->pfnVideoProcessorSetStreamDestRect = VideoProcess::VideoProcessorSetStreamDestRect;
        pFuncs->pfnVideoProcessorSetStreamAlpha = VideoProcess::VideoProcessorSetStreamAlpha;
        pFuncs->pfnVideoProcessorSetStreamPalette = nullptr;
        pFuncs->pfnVideoProcessorSetStreamPixelAspectRatio = VideoProcess::VideoProcessorSetStreamPixelAspectRatio;
        pFuncs->pfnVideoProcessorSetStreamLumaKey = VideoProcess::VideoProcessorSetStreamLumaKey;
        pFuncs->pfnVideoProcessorSetStreamStereoFormat = VideoProcess::VideoProcessorSetStreamStereoFormat;
        pFuncs->pfnVideoProcessorSetStreamAutoProcessingMode = VideoProcess::VideoProcessorSetStreamAutoProcessingMode;
        pFuncs->pfnVideoProcessorSetStreamFilter = VideoProcess::VideoProcessorSetStreamFilter;
        pFuncs->pfnVideoProcessorSetStreamExtension = VideoProcess::VideoProcessorSetStreamExtension;
        pFuncs->pfnVideoProcessorGetStreamExtension = VideoProcess::VideoProcessorGetStreamExtension;
        pFuncs->pfnVideoProcessorSetStreamRotation = VideoProcess::VideoProcessorSetStreamRotation;
        pFuncs->pfnVideoProcessorSetOutputColorSpace1 = VideoProcess::VideoProcessorSetOutputColorSpace1;
        pFuncs->pfnVideoProcessorSetOutputShaderUsage = VideoProcess::VideoProcessorSetOutputShaderUsage;
        pFuncs->pfnVideoProcessorSetStreamColorSpace1 = VideoProcess::VideoProcessorSetStreamColorSpace1;
        pFuncs->pfnVideoProcessorSetStreamMirror = VideoProcess::VideoProcessorSetStreamMirror;
        pFuncs->pfnVideoProcessorBlt = VideoProcess::VideoProcessorBlt;

        // crypto functionality
        pFuncs->pfnCalcPrivateCryptoSessionSize = [](D3D10DDI_HDEVICE, CONST D3D11_1DDIARG_CREATECRYPTOSESSION*)->SIZE_T { return 0; }; // Runtime allows pfnCreateCryptoSession to be nullptr, but not pfnCalcPrivateCryptoSessionSize
        pFuncs->pfnCreateCryptoSession = nullptr;
        pFuncs->pfnDestroyCryptoSession = nullptr;
        pFuncs->pfnGetContentProtectionCaps = VideoDevice::GetContentProtectionCaps;
        pFuncs->pfnGetCryptoKeyExchangeType = VideoDevice::GetCryptoKeyExchangeType;
        pFuncs->pfnCryptoSessionGetHandle = nullptr;
        pFuncs->pfnCheckCryptoSessionStatus = nullptr;
        pFuncs->pfnGetCryptoSessionPrivateDataSize = VideoDevice::GetCryptoSessionPrivateDataSize;
        pFuncs->pfnGetDataForNewHardwareKey = nullptr;
        pFuncs->pfnNegotiateCryptoSessionKeyExchange = nullptr;
        pFuncs->pfnNegotiateCryptoSessionKeyExchangeMT = nullptr;

#if 0   // TODO: this contains the remaining functions to be implemented. Should be empty when we are done.
        // video decoder
        pFuncs->pfnVideoDecoderGetHandle = VideoDecoder::VideoDecoderGetHandle;

        // video processor
        pFuncs->pfnVideoProcessorGetBehaviorHints = VideoProcessor::GetBehaviorHints;

        // view functionality
        pFuncs->pfnVideoProcessorInputViewReadAfterWriteHazard = Device::VideoProcessorInputViewReadAfterWriteHazard;

        // crypto functionality
        pFuncs->pfnGetCertificateSize = GetCertificateSize;
        pFuncs->pfnGetCertificate = GetCertificate;
        pFuncs->pfnEncryptionBlt = EncryptionBlt;
        pFuncs->pfnDecryptionBlt = DecryptionBlt;
        pFuncs->pfnStartSessionKeyRefresh = StartSessionKeyRefresh;
        pFuncs->pfnFinishSessionKeyRefresh = FinishSessionKeyRefresh;
        pFuncs->pfnGetEncryptionBltKey = GetEncryptionBltKey;
        pFuncs->pfnCalcPrivateAuthenticatedChannelSize = CalcPrivateAuthenticatedChannelSize;
        pFuncs->pfnCreateAuthenticatedChannel = CreateAuthenticatedChannel;
        pFuncs->pfnDestroyAuthenticatedChannel = DestroyAuthenticatedChannel;
        pFuncs->pfnNegotiateAuthenticatedChannelKeyExchange = NegotiateAuthenticatedChannelKeyExchange;
        pFuncs->pfnQueryAuthenticatedChannel = QueryAuthenticatedChannel;
        pFuncs->pfnConfigureAuthenticatedChannel = ConfigureAuthenticatedChannel;
        pFuncs->pfnGetCaptureHandle = GetCaptureHandle;
#endif
    }

    VideoDevice::VideoDevice(Device& parent) :
        DeviceChild(parent),
        m_UnderlyingVideoDevice(&parent.GetImmediateContextNoFlush())
    {
    }

    void APIENTRY VideoDevice::GetVideoDecoderProfileCount(_In_ D3D10DDI_HDEVICE hDevice, _Out_ UINT *pProfileCount)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
        pVideoDevice->UnderlyingVideoDevice()->GetVideoDecoderProfileCount(pProfileCount);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoDevice::GetVideoDecoderProfile(_In_ D3D10DDI_HDEVICE hDevice, _In_ UINT Index, _Out_ GUID *pDecoderProfile)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
        pVideoDevice->UnderlyingVideoDevice()->GetVideoDecoderProfile(Index, pDecoderProfile);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoDevice::CheckVideoDecoderFormat(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST GUID *pDecoderProfile, _In_ DXGI_FORMAT format, _Out_ BOOL *pSupported)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
        pVideoDevice->UnderlyingVideoDevice()->CheckVideoDecoderFormat(pDecoderProfile, format, pSupported);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoDevice::GetVideoDecoderConfigCount(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC *pDesc, _Out_ UINT *pConfigCount)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
        D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
        VideoTranslate::VideoDecodeDesc(pDesc, &desc);
        pVideoDevice->UnderlyingVideoDevice()->GetVideoDecoderConfigCount(&desc, pConfigCount);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoDevice::GetVideoDecoderConfig(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC *pDesc, _In_ UINT Index, _Out_ D3D11_1DDI_VIDEO_DECODER_CONFIG *pConfig)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
        D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
        D3D12TranslationLayer::VIDEO_DECODE_CONFIG config = {};
        VideoTranslate::VideoDecodeDesc(pDesc, &desc);
        pVideoDevice->UnderlyingVideoDevice()->GetVideoDecoderConfig(&desc, Index, &config);
        VideoTranslate::VideoDecodeConfig(&desc, &config, pConfig);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoDevice::GetVideoDecoderBufferTypeCount(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC *pDesc, _Out_ UINT *pBufferTypeCount)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
        D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
        VideoTranslate::VideoDecodeDesc(pDesc, &desc);
        pVideoDevice->UnderlyingVideoDevice()->GetVideoDecoderBufferTypeCount(&desc, pBufferTypeCount);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoDevice::GetVideoDecoderBufferInfo(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC *pDesc, _In_ UINT Index, _Out_ D3D11_1DDI_VIDEO_DECODER_BUFFER_INFO *pBufferInfo)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();

        UINT Size;
        D3D12TranslationLayer::VIDEO_DECODE_BUFFER_TYPE Type;
        D3D12TranslationLayer::VIDEO_DECODE_DESC desc = {};
        VideoTranslate::VideoDecodeDesc(pDesc, &desc);
        pVideoDevice->UnderlyingVideoDevice()->GetVideoDecoderBufferInfo(&desc, Index, &Type, &Size);
        VideoTranslate::VideoDecodeBufferInfo(Type, Size, pBufferInfo);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoDevice::QueryVideoCapabilities(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3DWDDM2_0DDI_VIDEO_CAPABILITY_QUERY QueryType, _In_ UINT DataSize, _Inout_ VOID* pData)
    {
        UNREFERENCED_PARAMETER(DataSize);
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        VideoDevice *pVideoDevice = pDevice->GetVideoDevice();

        //
        // As D3D12 API for output conversion/decoder caps is more specific than D3D11, to be able to have the D3D11 results for the various query types, 
        // we need to try common combinations with D3D12.
        //

        D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT defaultDecodeConversionSupport = {};
        defaultDecodeConversionSupport.Configuration.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
        defaultDecodeConversionSupport.Configuration.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
        defaultDecodeConversionSupport.DecodeSample.Width = 1920;
        defaultDecodeConversionSupport.DecodeSample.Height = 1080;
        defaultDecodeConversionSupport.FrameRate.Numerator = 30;
        defaultDecodeConversionSupport.FrameRate.Denominator = 1;

        const D3D12_VIDEO_FORMAT videoFormats[] = {
            { DXGI_FORMAT_NV12, DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709 },
            { DXGI_FORMAT_P010, DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020 },
            { DXGI_FORMAT_P016, DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020 },
            { DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020},
            { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 }
        };

        switch (QueryType)
        {
            case D3DWDDM2_0DDI_VIDEO_CAPABILITY_QUERY_DECODER_CAPS:
            {
                assert(DataSize == sizeof(D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_CAPS));
                D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_CAPS *pDecoderCaps = (D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_CAPS *)pData;
                pDecoderCaps->Caps = D3DWDDM2_0DDI_VIDEO_DECODER_CAP_UNSUPPORTED;

                if (pDecoderCaps->pCryptoType == nullptr ||
                    InlineIsEqualGUID(*pDecoderCaps->pCryptoType, DXVA_NoEncrypt))
                {
                    D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT videoDecodeSupport = {};
                    videoDecodeSupport.Configuration.DecodeProfile = pDecoderCaps->DecodeProfile;
                    videoDecodeSupport.Configuration.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
                    videoDecodeSupport.Configuration.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
                    videoDecodeSupport.Width = pDecoderCaps->SampleWidth;
                    videoDecodeSupport.Height = pDecoderCaps->SampleHeight;
                    videoDecodeSupport.FrameRate = pDecoderCaps->FrameRate;
                    videoDecodeSupport.BitRate = pDecoderCaps->BitRate;
                    for (DWORD i = 0; i < _countof(videoFormats); i++)
                    {
                        videoDecodeSupport.DecodeFormat = videoFormats[i].Format;
                        pVideoDevice->UnderlyingVideoDevice()->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_SUPPORT, &videoDecodeSupport, sizeof(videoDecodeSupport));
                        if (videoDecodeSupport.SupportFlags != D3D12_VIDEO_DECODE_SUPPORT_FLAG_NONE)
                        {
                            pDecoderCaps->Caps &= ~D3DWDDM2_0DDI_VIDEO_DECODER_CAP_UNSUPPORTED;
                            break;
                        }
                    }
                }

                // If format is supported, verify if we support downsampling
                if (!(pDecoderCaps->Caps & D3DWDDM2_0DDI_VIDEO_DECODER_CAP_UNSUPPORTED))
                {
                    D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT decodeConversionSupport = defaultDecodeConversionSupport;
                    decodeConversionSupport.Configuration.DecodeProfile = pDecoderCaps->DecodeProfile;
                    decodeConversionSupport.Configuration.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
                    decodeConversionSupport.DecodeSample.Width = pDecoderCaps->SampleWidth;
                    decodeConversionSupport.DecodeSample.Height = pDecoderCaps->SampleHeight;
                    decodeConversionSupport.FrameRate = pDecoderCaps->FrameRate;
                    decodeConversionSupport.BitRate = pDecoderCaps->BitRate;
                    bool done = false;

                    for (DWORD i = 0; i < _countof(videoFormats)  &&  !done; i++)
                    {
                        decodeConversionSupport.DecodeSample.Format = videoFormats[i];
                        for (DWORD j = 0; j < _countof(videoFormats)  &&  !done; j++)
                        {
                            if (i != j )
                            {
                                decodeConversionSupport.OutputFormat = videoFormats[j];
                                pVideoDevice->UnderlyingVideoDevice()->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_CONVERSION_SUPPORT, &decodeConversionSupport, sizeof(decodeConversionSupport));
                                if ((decodeConversionSupport.SupportFlags & D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAG_SUPPORTED) != 0
                                    && D3D12TranslationLayer::IsScaleSupported(decodeConversionSupport.ScaleSupport, (pDecoderCaps->SampleWidth + 1) / 2, (pDecoderCaps->SampleHeight + 1) / 2))
                                {
                                    pDecoderCaps->Caps |= D3DWDDM2_0DDI_VIDEO_DECODER_CAP_DOWNSAMPLE;
                                    done = true;
                                }
                            }
                        }
                    }
                }
            }
            break;

            case D3DWDDM2_0DDI_VIDEO_CAPABILITY_QUERY_DECODER_DOWNSAMPLING:
            {
                assert(DataSize == sizeof(D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_DOWNSAMPLING));
                D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_DOWNSAMPLING *pDownsampling = (D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_DOWNSAMPLING *)pData;

                D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT decodeConversionSupport = defaultDecodeConversionSupport;
                decodeConversionSupport.Configuration.DecodeProfile = pDownsampling->pInputDesc->Guid;
                decodeConversionSupport.Configuration.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
                decodeConversionSupport.DecodeSample.Width = pDownsampling->pInputDesc->SampleWidth;
                decodeConversionSupport.DecodeSample.Height = pDownsampling->pInputDesc->SampleHeight;
                decodeConversionSupport.DecodeSample.Format.Format = pDownsampling->pInputDesc->OutputFormat;
                decodeConversionSupport.DecodeSample.Format.ColorSpace = VideoTranslate::ColorSpaceType(pDownsampling->InputColorSpace);
                decodeConversionSupport.OutputFormat.Format = pDownsampling->pOutputDesc->OutputFormat;
                decodeConversionSupport.OutputFormat.ColorSpace = VideoTranslate::ColorSpaceType(pDownsampling->OutputColorSpace);
                pVideoDevice->UnderlyingVideoDevice()->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_CONVERSION_SUPPORT, &decodeConversionSupport, sizeof(decodeConversionSupport));
                
                pDownsampling->Supported = 
                       (decodeConversionSupport.SupportFlags & D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAG_SUPPORTED) != 0
                    && D3D12TranslationLayer::IsScaleSupported(decodeConversionSupport.ScaleSupport, pDownsampling->pOutputDesc->SampleWidth, pDownsampling->pOutputDesc->SampleHeight);

                pDownsampling->RealTime = pDownsampling->Supported;
                break;
            }

            case D3DWDDM2_0DDI_VIDEO_CAPABILITY_QUERY_RECOMMEND_DECODER_DOWNSAMPLING:
            {
                assert(DataSize == sizeof(D3DWDDM2_0DDI_VIDEO_CAPABILITY_RECOMMEND_DECODER_DOWNSAMPLING));
                D3DWDDM2_0DDI_VIDEO_CAPABILITY_RECOMMEND_DECODER_DOWNSAMPLING *pRecommendDownsampling = (D3DWDDM2_0DDI_VIDEO_CAPABILITY_RECOMMEND_DECODER_DOWNSAMPLING *)pData;
                pRecommendDownsampling->pOutputDesc->OutputFormat = pRecommendDownsampling->pInputDesc->OutputFormat;
                pRecommendDownsampling->pOutputDesc->SampleWidth = pRecommendDownsampling->pInputDesc->SampleWidth;
                pRecommendDownsampling->pOutputDesc->SampleHeight = pRecommendDownsampling->pInputDesc->SampleHeight;
                // TODO: work item  9070454 - need VP information about min/max resolution/min/max scale factor + modifications in output_conversion support to be able to answer this query properly.
                // For now, just filling out the values with the input ones.
                break;
            }

            case D3DWDDM2_0DDI_VIDEO_CAPABILITY_QUERY_DECODER_DOWNSAMPLE_OUTPUT_FORMAT:
            {
                assert(DataSize == sizeof(D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_DOWNSAMPLE_OUTPUT_FORMAT));
                D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_DOWNSAMPLE_OUTPUT_FORMAT *pDownsampleOutputFormat = (D3DWDDM2_0DDI_VIDEO_CAPABILITY_DECODER_DOWNSAMPLE_OUTPUT_FORMAT *)pData;
                pDownsampleOutputFormat->Supported = FALSE;
                bool done = false;
                for (DWORD i = 0; i < _countof(videoFormats)  &&  !done; i++)
                {
                    D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT decodeConversionSupport = defaultDecodeConversionSupport;
                    decodeConversionSupport.Configuration.DecodeProfile = pDownsampleOutputFormat->DecodeProfile;
                    decodeConversionSupport.DecodeSample.Format = videoFormats[i];
                    decodeConversionSupport.OutputFormat.Format = pDownsampleOutputFormat->Format;
                    decodeConversionSupport.OutputFormat.ColorSpace = CDXGIColorSpaceHelper::ConvertFromLegacyColorSpace(!CD3D11FormatHelper::YUV(pDownsampleOutputFormat->Format), CD3D11FormatHelper::GetBitsPerUnit(pDownsampleOutputFormat->Format), /* StudioRGB= */ false, /* P709= */ true, /* StudioYUV= */ true);
                    pVideoDevice->UnderlyingVideoDevice()->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_CONVERSION_SUPPORT, &decodeConversionSupport, sizeof(decodeConversionSupport));
                    if (decodeConversionSupport.SupportFlags & D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAG_SUPPORTED)
                    {
                        pDownsampleOutputFormat->Supported = TRUE;
                        done = true;
                    }
                }
            }
            break;
            
            case D3DWDDM2_4DDI_VIDEO_CAPABILITY_QUERY_DECODER_HISTOGRAM:
            {
                assert(DataSize == sizeof(D3DWDDM2_4DDI_VIDEO_CAPABILITY_DECODER_HISTOGRAM));
                D3DWDDM2_4DDI_VIDEO_CAPABILITY_DECODER_HISTOGRAM *pDDIHistogram = static_cast<D3DWDDM2_4DDI_VIDEO_CAPABILITY_DECODER_HISTOGRAM *>(pData);

                D3D12_FEATURE_DATA_VIDEO_DECODE_HISTOGRAM APIHistogram =
                {
                    pDevice->GetImmediateContextNoFlush().GetNodeIndex(),
                    pDDIHistogram->DecoderDesc.Guid,
                    pDDIHistogram->DecoderDesc.SampleWidth,
                    pDDIHistogram->DecoderDesc.SampleHeight,
                    pDDIHistogram->DecoderDesc.OutputFormat
                };

                pVideoDevice->UnderlyingVideoDevice()->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_HISTOGRAM, &APIHistogram, sizeof(APIHistogram));

                pDDIHistogram->Components = static_cast<D3DWDDM2_4DDI_VIDEO_DECODER_HISTOGRAM_COMPONENT_FLAGS>(APIHistogram.Components);
                pDDIHistogram->BinCount = APIHistogram.BinCount;
                pDDIHistogram->CounterBitDepth = APIHistogram.CounterBitDepth;

                break;
            }
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    SIZE_T APIENTRY VideoDevice::CalcPrivateVideoDecoderOutputViewSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEODECODEROUTPUTVIEW * /*pCreateVideoDecoderOutputView*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T videoDecoderOutputViewSize = sizeof(VideoDecoderOutputView);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, videoDecoderOutputViewSize);
    }

    HRESULT APIENTRY VideoDevice::CreateVideoDecoderOutputView(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEODECODEROUTPUTVIEW* pCreateVideoDecoderOutputView, _In_ D3D11_1DDI_HVIDEODECODEROUTPUTVIEW hVideoDecoderOutputView, _In_ D3D11_1DDI_HRTVIDEODECODEROUTPUTVIEW /*hRTVideoDecoderOutputView*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device &device = *Device::CastFrom(hDevice);

        new (hVideoDecoderOutputView.pDrvPrivate) VideoDecoderOutputView(device, *Resource::CastFromAndGetImmediateResource(pCreateVideoDecoderOutputView->hDrvResource), pCreateVideoDecoderOutputView);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    VOID APIENTRY VideoDevice::DestroyVideoDecoderOutputView(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEODECODEROUTPUTVIEW hVideoDecoderOutputView)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoDecoderOutputView::CastFrom(hVideoDecoderOutputView)->~View();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    SIZE_T APIENTRY VideoDevice::CalcPrivateVideoProcessorInputViewSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORINPUTVIEW * /*pCreateVideoProcessorInputView*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T videoProcessorInputViewSize = sizeof(VideoProcessorInputView);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, videoProcessorInputViewSize);
    }

    HRESULT APIENTRY VideoDevice::CreateVideoProcessorInputView(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORINPUTVIEW* pCreateVideoProcessorInputView, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW hVideoProcessorInputView, _In_ D3D11_1DDI_HRTVIDEOPROCESSORINPUTVIEW /*hRTVideoProcessorInputView*/)
    {
        HRESULT hr = S_OK;
        D3D11on12_DDI_ENTRYPOINT_START();
        Device &device = *Device::CastFrom(hDevice);

        new (hVideoProcessorInputView.pDrvPrivate) VideoProcessorInputView(device, *Resource::CastFromAndGetImmediateResource(pCreateVideoProcessorInputView->hDrvResource), pCreateVideoProcessorInputView);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    VOID APIENTRY VideoDevice::DestroyVideoProcessorInputView(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW hVideoProcessorInputView)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessorInputView::CastFrom(hVideoProcessorInputView)->~View();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    SIZE_T APIENTRY VideoDevice::CalcPrivateVideoProcessorOutputViewSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOROUTPUTVIEW * /*pCreateVideoProcessorOutputView*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T videoProcessorOutputViewSize = sizeof(VideoProcessorOutputView);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, videoProcessorOutputViewSize);
    }

    HRESULT APIENTRY VideoDevice::CreateVideoProcessorOutputView(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOROUTPUTVIEW* pCreateVideoProcessorOutputView, _In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW hVideoProcessorOutputView, _In_ D3D11_1DDI_HRTVIDEOPROCESSOROUTPUTVIEW /*hRTVideoProcessorOutputView*/)
    {
        HRESULT hr = S_OK;
        D3D11on12_DDI_ENTRYPOINT_START();
        Device &device = *Device::CastFrom(hDevice);

        new (hVideoProcessorOutputView.pDrvPrivate) VideoProcessorOutputView(device, *Resource::CastFromAndGetImmediateResource(pCreateVideoProcessorOutputView->hDrvResource), pCreateVideoProcessorOutputView);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    VOID APIENTRY VideoDevice::DestroyVideoProcessorOutputView(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW hVideoProcessorOutputView)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessorOutputView::CastFrom(hVideoProcessorOutputView)->~View();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void VideoDevice::GetContentProtectionSystems(_In_ Device *, _In_ const GUID *pCryptoType, std::vector<GUID> &contentProtectionSystems)
    {
        if (pCryptoType == nullptr || IsEqualGUID(*pCryptoType, GUID_NULL))
        {
            contentProtectionSystems.clear();
        }
        else
        {
            contentProtectionSystems.resize(1);
            contentProtectionSystems[0] = *pCryptoType;
        }
    }

    void VideoDevice::GetDecodeProfiles(_In_ Device *pDevice, _In_ const GUID *pDecodeProfile, std::vector<GUID> &decodeProfiles)
    {
        if (pDecodeProfile == nullptr  ||  IsEqualGUID(*pDecodeProfile, GUID_NULL))
        {
            VideoDevice *pVideoDevice = pDevice->GetVideoDevice();
            UINT count = 0;
            pVideoDevice->UnderlyingVideoDevice()->GetVideoDecoderProfileCount(&count);
            decodeProfiles.resize(count);
            D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES decodeProfileSupport = {};
            decodeProfileSupport.ProfileCount = count;
            decodeProfileSupport.pProfiles = decodeProfiles.data();
            pVideoDevice->UnderlyingVideoDevice()->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILES, &decodeProfileSupport, sizeof(decodeProfileSupport));
        }
        else
        {
            decodeProfiles.resize(1);
            decodeProfiles[0] = *pDecodeProfile;
        }

    }

    bool VideoDevice::CheckHardwareDRMSupport(_In_ Device *, _In_ const GUID* , _In_ const GUID* )
    {
        return false;
    }

    HRESULT APIENTRY VideoDevice::GetContentProtectionCaps(_In_ D3D10DDI_HDEVICE hDevice, _In_ const GUID* pCryptoType, _In_ const GUID* pDecodeProfile, _Out_ D3D11_1DDI_VIDEO_CONTENT_PROTECTION_CAPS* pCaps)
    {
        HRESULT hr = S_OK;
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);
        D3D12TranslationLayer::ImmediateContext& context = pDevice->GetImmediateContextNoFlush();

        pCaps->Caps = 0;
        pCaps->KeyExchangeTypeCount = 0;
        pCaps->BlockAlignmentSize = 0;              // TODO:
        pCaps->ProtectedMemorySize = 0;             // TODO:

        // get protected session support
        D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT protectedResourceSessionSupport = {};
        context.CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT, &protectedResourceSessionSupport, sizeof(protectedResourceSessionSupport));
        if (protectedResourceSessionSupport.Support &  D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAG_SUPPORTED)
        {
            pCaps->Caps |= D3DWDDM2_0DDI_CONTENT_PROTECTION_CAPS_HARDWARE_PROTECT_UNCOMPRESSED;
        }

        if (CheckHardwareDRMSupport(pDevice, pCryptoType, pDecodeProfile))
        {
            pCaps->Caps |= D3D11_1DDI_CONTENT_PROTECTION_CAPS_HARDWARE;
            pCaps->KeyExchangeTypeCount = 1;        // Just hardware DRM is supported
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY VideoDevice::GetCryptoKeyExchangeType(_In_ D3D10DDI_HDEVICE hDevice, _In_ const GUID* pCryptoType, _In_ const GUID* pDecodeProfile, _In_ UINT Index, _Out_ GUID* pKeyExchangeType)
    {
        HRESULT hr = S_OK;
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        if (Index >= 1)      // Just hardware DRM is supported
        {
            ThrowFailure(E_INVALIDARG);
        }

        if (CheckHardwareDRMSupport(pDevice, pCryptoType, pDecodeProfile))
        {
            *pKeyExchangeType = D3D11_KEY_EXCHANGE_HW_PROTECTION;
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    void APIENTRY VideoDevice::GetCryptoSessionPrivateDataSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ const GUID* pCryptoType, _In_opt_ const GUID* pDecodeProfile, _In_ const GUID* pKeyExchangeType, _Out_ UINT* pPrivateInputSize, _Out_ UINT* pPrivateOutputSize)
    {
        D3D11on12_DDI_ENTRYPOINT_START();

        UNREFERENCED_PARAMETER(pKeyExchangeType);
        UNREFERENCED_PARAMETER(pDecodeProfile);
        UNREFERENCED_PARAMETER(pCryptoType);
        UNREFERENCED_PARAMETER(hDevice);

        *pPrivateInputSize = 0;
        *pPrivateOutputSize = 0;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
}

