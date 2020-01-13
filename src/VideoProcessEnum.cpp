// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    SIZE_T APIENTRY VideoProcessEnum::CalcPrivateVideoProcessorEnumSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORENUM * /*pCreateVideoProcessEnum*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T videoProcessEnumSize = sizeof(VideoProcessEnum);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, videoProcessEnumSize);
    }

    _Use_decl_annotations_
    HRESULT APIENTRY VideoProcessEnum::CreateVideoProcessorEnum(D3D10DDI_HDEVICE hDevice, CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORENUM *pCreateVideoProcessEnum, D3D11_1DDI_HVIDEOPROCESSORENUM hVideoProcessEnum,  D3D11_1DDI_HRTVIDEOPROCESSORENUM /*hRTVideoProcessEnum*/) noexcept
    {
        HRESULT hr = S_OK;
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        AutoCleanupPtr<VideoProcessEnum> spVideoProcessEnum(new (hVideoProcessEnum.pDrvPrivate) VideoProcessEnum(*pDevice));
        spVideoProcessEnum->CacheVideoProcessorInfo(pCreateVideoProcessEnum);

        // No more exceptions
        spVideoProcessEnum.release();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    void APIENTRY VideoProcessEnum::DestroyVideoProcessorEnum(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hVideoProcessEnum) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessEnum *pVideoProcessEnum = VideoProcessEnum::CastFrom(hVideoProcessEnum);
        pVideoProcessEnum->~VideoProcessEnum();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    VideoProcessEnum::VideoProcessEnum(Device& parent) :
        DeviceChild(parent),
        m_UnderlyingVideoProcessEnum(&parent.GetImmediateContextNoFlush())
    {
        m_UnderlyingVideoProcessEnum.Initialize();
    }

    _Use_decl_annotations_
    void VideoProcessEnum::CacheVideoProcessorInfo(_In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORENUM *pCreateVideoProcessEnum)
    {
        m_creationArgs.InputFieldType = VideoTranslate::VideoFieldType(pCreateVideoProcessEnum->Desc.InputFrameFormat);
        m_creationArgs.InputWidth = pCreateVideoProcessEnum->Desc.InputWidth;
        m_creationArgs.InputHeight = pCreateVideoProcessEnum->Desc.InputHeight;
        m_creationArgs.InputFrameRate = pCreateVideoProcessEnum->Desc.InputFrameRate;
        m_creationArgs.OutputWidth = pCreateVideoProcessEnum->Desc.OutputWidth;
        m_creationArgs.OutputHeight = pCreateVideoProcessEnum->Desc.OutputHeight;
        m_creationArgs.OutputFrameRate = pCreateVideoProcessEnum->Desc.OutputFrameRate;

        m_UnderlyingVideoProcessEnum.CacheVideoProcessInfo(m_creationArgs);

        std::vector<D3D12TranslationLayer::VIDEO_PROCESS_SUPPORT> vpSupportTuples = m_UnderlyingVideoProcessEnum.GetVPCapsSupportTuples();

        ZeroMemory(&m_videoProcessCaps, sizeof(m_videoProcessCaps));
        m_rateConversionCaps.clear();
        m_rateConversionCaps.resize(1);

        // NOTE: Unsupported DX11 features:
        //      NO optional DX11 stereo formats
        //      NO D3D11_VIDEO_PROCESSOR_DEVICE_CAPS_LINEAR_SPACE, so assuming no LINEAR_SPACE conversions. This flag was a hint for the apps to know if the conversions were done in linear or gamma space.
        //      NO D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_CONSTRICTION
        //      NO Palettes
        //      NO Inverse telecine caps
        //      NO custom rates

        for (auto& tuple : vpSupportTuples)
        {
            VideoTranslate::AddVideoProcessCaps(tuple.dx12Support, m_videoProcessCaps, m_rateConversionCaps, m_filterRanges, _countof(m_filterRanges), tuple.colorConversionCaps);
        }

        for (auto& rateConversionCap : m_rateConversionCaps)
        {
            D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS DeinterlaceSupport = (rateConversionCap.ConversionCaps & D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_DEINTERLACE_ADAPTIVE) != 0 
                                                                     ? D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM : D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB;

            auto referenceInfo = m_UnderlyingVideoProcessEnum.UpdateReferenceInfo(DeinterlaceSupport);

            if (referenceInfo.frameRateConversionSupported)
            {
                rateConversionCap.ConversionCaps |= D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_FRAME_RATE_CONVERSION;
            }

            rateConversionCap.PastFrames = referenceInfo.pastFrames;
            rateConversionCap.FutureFrames = referenceInfo.futureFrames;
        }

        // Max input streams
        {
            D3D12_FEATURE_DATA_VIDEO_PROCESS_MAX_INPUT_STREAMS vpMaxInputStreams = {};
            m_UnderlyingVideoProcessEnum.CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS, &vpMaxInputStreams, sizeof(vpMaxInputStreams));
            m_videoProcessCaps.MaxInputStreams = vpMaxInputStreams.MaxInputStreams;
            m_videoProcessCaps.MaxStreamStates = m_videoProcessCaps.MaxInputStreams;       // Assuming identical values.
            m_creationArgs.MaxInputStreams = vpMaxInputStreams.MaxInputStreams;
        }

        m_videoProcessCaps.RateConversionCapsCount = static_cast<UINT>(m_rateConversionCaps.size());
    }

    _Use_decl_annotations_
    void APIENTRY VideoProcessEnum::CheckVideoProcessorFormat( D3D10DDI_HDEVICE hDevice, D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, DXGI_FORMAT format, UINT* pFormatCaps) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessEnum *pVideoProcessEnum = VideoProcessEnum::CastFrom(hProcEnum);
        struct {
            DXGI_FORMAT inputFormat;
            DXGI_FORMAT outputFormat;
            UINT vpFormatSupportType;
        } formatPairs[] = {
            { format, format, D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT | D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT },
            { format, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT },
            { format, DXGI_FORMAT_NV12, D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT },
            { DXGI_FORMAT_R8G8B8A8_UNORM, format, D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT },
            { DXGI_FORMAT_NV12, format, D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT },
        };
        *pFormatCaps = 0;
        if (CD3D11FormatHelper::GetTypeLevel(format) == D3D11FTL_FULL_TYPE)
        {
            for (auto& pair : formatPairs)
            {
                DXGI_COLOR_SPACE_TYPE inputColorSpace = CDXGIColorSpaceHelper::ConvertFromLegacyColorSpace(!CD3D11FormatHelper::YUV(pair.inputFormat), CD3D11FormatHelper::GetBitsPerUnit(pair.inputFormat), /* StudioRGB= */ false, /* P709= */ true, /* StudioYUV= */ true);
                DXGI_COLOR_SPACE_TYPE outputColorSpace = CDXGIColorSpaceHelper::ConvertFromLegacyColorSpace(!CD3D11FormatHelper::YUV(pair.outputFormat), CD3D11FormatHelper::GetBitsPerUnit(pair.outputFormat), /* StudioRGB= */ false, /* P709= */ true, /* StudioYUV= */ true);
                if (!(*pFormatCaps & pair.vpFormatSupportType)  &&
                    pVideoProcessEnum->IsFormatConversionSupported(pair.inputFormat, inputColorSpace, pair.outputFormat, outputColorSpace))
                {
                    *pFormatCaps |= pair.vpFormatSupportType;
                }
            }
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoProcessEnum::CheckVideoProcessorFormatConversion(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _In_ D3DWDDM2_0DDI_CHECK_VIDEO_PROCESSOR_FORMAT_CONVERSION* pConversion) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessEnum *pVideoProcessEnum = VideoProcessEnum::CastFrom(hProcEnum);
        pConversion->Supported = pVideoProcessEnum->IsFormatConversionSupported(pConversion->InputFormat, VideoTranslate::ColorSpaceType(pConversion->InputColorSpace), pConversion->OutputFormat, VideoTranslate::ColorSpaceType(pConversion->OutputColorSpace));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }


    bool VideoProcessEnum::IsFormatConversionSupported(DXGI_FORMAT inputFormat, DXGI_COLOR_SPACE_TYPE inputColorSpace, DXGI_FORMAT outputFormat, DXGI_COLOR_SPACE_TYPE outputColorSpace)
    {
        D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT dx12Support = 
        {
            0,
            { m_creationArgs.InputWidth, m_creationArgs.InputHeight, { inputFormat, inputColorSpace} }, 
            m_creationArgs.InputFieldType, 
            D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE, 
            m_creationArgs.InputFrameRate,
            { outputFormat, outputColorSpace}, 
            D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE, 
            m_creationArgs.OutputFrameRate
        };
        m_UnderlyingVideoProcessEnum.CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, &dx12Support, sizeof(dx12Support));

        D3D12_FEATURE_DATA_FORMAT_SUPPORT FmtSupport = { outputFormat };
        m_UnderlyingVideoProcessEnum.m_pParent->CheckFormatSupport(FmtSupport);
        return VideoTranslate::IsSupported(dx12Support);
    }

    D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS VideoProcessEnum::GetRateConversionCaps(UINT RateConversionIndex)
    {
        return m_rateConversionCaps[RateConversionIndex];
    }

    void APIENTRY VideoProcessEnum::GetVideoProcessorCaps(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_CAPS* pCaps) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessEnum *pVideoProcessEnum = VideoProcessEnum::CastFrom(hProcEnum);
        *pCaps = pVideoProcessEnum->m_videoProcessCaps;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoProcessEnum::GetVideoProcessorRateConversionCaps(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _In_ UINT rateConversionIndex, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS* pCaps) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessEnum *pVideoProcessEnum = VideoProcessEnum::CastFrom(hProcEnum);
        *pCaps = pVideoProcessEnum->m_rateConversionCaps[rateConversionIndex];
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY VideoProcessEnum::GetVideoProcessorCustomRate(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM /*hProcEnum*/, _In_ UINT /*rateConversionIndex*/, _In_ UINT /*customRateIndex*/, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_CUSTOM_RATE* /*pCustomRate*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, E_NOTIMPL);
    }

    void APIENTRY VideoProcessEnum::GetVideoProcessorFilterRange(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _In_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER filter, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER_RANGE* pFilterRange) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcessEnum *pVideoProcessEnum = VideoProcessEnum::CastFrom(hProcEnum);
        *pFilterRange = pVideoProcessEnum->m_filterRanges[filter];
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
};