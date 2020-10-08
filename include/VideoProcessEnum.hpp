// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class VideoProcessEnum: public DeviceChild
    {
    public:
        VideoProcessEnum(Device& parent);

        static VideoProcessEnum* CastFrom(D3D11_1DDI_HVIDEOPROCESSORENUM hVideoProcessEnum) noexcept { return reinterpret_cast<VideoProcessEnum*>(hVideoProcessEnum.pDrvPrivate); }

        static SIZE_T APIENTRY CalcPrivateVideoProcessorEnumSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORENUM* pCreateEnum) noexcept;
        static HRESULT APIENTRY CreateVideoProcessorEnum(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORENUM*, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM, _In_ D3D11_1DDI_HRTVIDEOPROCESSORENUM) noexcept;
        static void APIENTRY DestroyVideoProcessorEnum(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM) noexcept;

        static void APIENTRY GetVideoProcessorCaps(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_CAPS* pCaps) noexcept;
        static void APIENTRY GetVideoProcessorRateConversionCaps(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _In_ UINT RateConversionIndex, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS* pCaps) noexcept;
        static void APIENTRY GetVideoProcessorCustomRate(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _In_ UINT RateConversionIndex, _In_ UINT CustomRateIndex, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_CUSTOM_RATE* pCustomRate) noexcept;
        static void APIENTRY GetVideoProcessorFilterRange(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _In_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER Filter, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER_RANGE* pFilterRange) noexcept;
        static void APIENTRY CheckVideoProcessorFormat(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcEnum, _In_ DXGI_FORMAT Format, _Out_ UINT* pFormatCaps) noexcept;
        static void APIENTRY CheckVideoProcessorFormatConversion(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSORENUM hProcessorEnum, _In_ D3DWDDM2_0DDI_CHECK_VIDEO_PROCESSOR_FORMAT_CONVERSION* pConversion) noexcept;

    public:
        // helpers
        virtual void CacheVideoProcessorInfo(_In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSORENUM *pCreateVideoProcessEnum);
        void GetCreationArgs(_Out_ D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS *pCreationArgs) { *pCreationArgs = m_creationArgs; }
        bool IsFormatConversionSupported(DXGI_FORMAT inputFormat, DXGI_COLOR_SPACE_TYPE inputColorSpace, DXGI_FORMAT outputFormat, DXGI_COLOR_SPACE_TYPE outputColorSpace);
        bool IsAutoProcessingSupported() { return UnderlyingVideoProcessEnum()->IsAutoProcessingSupported(); }
        D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS GetRateConversionCaps(UINT RateConversionIndex);

    protected:
        virtual D3D12TranslationLayer::VideoProcessEnum* UnderlyingVideoProcessEnum() { return m_UnderlyingVideoProcessEnum.get(); }

        std::unique_ptr<D3D12TranslationLayer::VideoProcessEnum> m_UnderlyingVideoProcessEnum;

        D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS m_creationArgs;
        D3D11_1DDI_VIDEO_PROCESSOR_CAPS m_videoProcessCaps = {};
        std::vector<D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS> m_rateConversionCaps;
        static const UINT MAX_VIDEO_PROCESSOR_FILTERS = D3D12_VIDEO_PROCESS_MAX_FILTERS;
        D3D11_1DDI_VIDEO_PROCESSOR_FILTER_RANGE m_filterRanges[MAX_VIDEO_PROCESSOR_FILTERS] = {};
    };
};