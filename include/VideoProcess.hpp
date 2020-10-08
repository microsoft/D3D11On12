// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class VideoProcess: public DeviceChild
    {
    public:
        static void CreateVideoProcess(D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, Device& parent, UINT MaxInputStreams, const D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS& rateConversionCaps, bool AutoProcessingSupported);

        static VideoProcess* CastFrom(D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess) noexcept { return reinterpret_cast<VideoProcess*>(hVideoProcess.pDrvPrivate); }

        static SIZE_T APIENTRY CalcPrivateVideoProcessorSize(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOR*);
        static HRESULT APIENTRY CreateVideoProcessor(_In_ D3D10DDI_HDEVICE, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOR*, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ D3D11_1DDI_HRTVIDEOPROCESSOR) noexcept;
        static void APIENTRY DestroyVideoProcessor(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR) noexcept;

        static void APIENTRY VideoProcessorSetOutputTargetRect(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ BOOL, _In_ CONST RECT*) noexcept;
        static void APIENTRY VideoProcessorSetOutputBackgroundColor(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ BOOL, _In_ CONST D3D11_1DDI_VIDEO_COLOR*) noexcept;
        static void APIENTRY VideoProcessorSetOutputAlphaFillMode(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE, _In_ UINT) noexcept;
        static void APIENTRY VideoProcessorSetOutputConstriction(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ BOOL, _In_ SIZE) noexcept;
        static void APIENTRY VideoProcessorSetOutputStereoMode(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ BOOL) noexcept;
        static void APIENTRY VideoProcessorSetStreamFrameFormat(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ D3D11_1DDI_VIDEO_FRAME_FORMAT) noexcept;
        static void APIENTRY VideoProcessorSetStreamOutputRate(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ D3D11_1DDI_VIDEO_PROCESSOR_OUTPUT_RATE, _In_ BOOL, _In_ CONST DXGI_RATIONAL*) noexcept;
        static void APIENTRY VideoProcessorSetStreamSourceRect(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ BOOL, _In_ CONST RECT*) noexcept;
        static void APIENTRY VideoProcessorSetStreamDestRect(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ BOOL, _In_ CONST RECT*) noexcept;
        static void APIENTRY VideoProcessorSetStreamAlpha(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ BOOL, _In_ FLOAT) noexcept;
        static void APIENTRY VideoProcessorSetStreamPixelAspectRatio(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ BOOL, _In_ CONST DXGI_RATIONAL*, _In_ CONST DXGI_RATIONAL*) noexcept;
        static void APIENTRY VideoProcessorSetStreamLumaKey(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ BOOL, _In_ FLOAT, _In_ FLOAT) noexcept;
        static void APIENTRY VideoProcessorSetStreamStereoFormat(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ BOOL, _In_ D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT, _In_ BOOL, _In_ BOOL, _In_ D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FLIP_MODE, _In_ int) noexcept;

        static void APIENTRY VideoProcessorSetStreamAutoProcessingMode(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ BOOL) noexcept;
        static void APIENTRY VideoProcessorSetStreamFilter(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER, _In_ BOOL, _In_ int) noexcept;
        static void APIENTRY VideoProcessorSetStreamRotation(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT StreamIndex, _In_ BOOL Enable, _In_ D3D11_1DDI_VIDEO_PROCESSOR_ROTATION Rotation) noexcept;
        static void APIENTRY VideoProcessorSetOutputColorSpace1(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ D3DDDI_COLOR_SPACE_TYPE ColorSpace) noexcept;
        static void APIENTRY VideoProcessorSetOutputShaderUsage(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ BOOL ShaderUsage) noexcept;
        static void APIENTRY VideoProcessorSetStreamColorSpace1(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ D3DDDI_COLOR_SPACE_TYPE ColorSpace) noexcept;
        static void APIENTRY VideoProcessorSetStreamMirror(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ BOOL Enable, _In_ BOOL FlipHorizontal, _In_ BOOL FlipVertical) noexcept;

        static HRESULT APIENTRY VideoProcessorSetOutputExtension(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ CONST GUID*, _In_ UINT, _In_ PVOID) noexcept;
        static HRESULT APIENTRY VideoProcessorGetOutputExtension(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ CONST GUID*, _In_ UINT, _Out_ PVOID) noexcept;
        static HRESULT APIENTRY VideoProcessorSetStreamExtension(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ CONST GUID *, _In_ UINT, _In_ PVOID) noexcept;
        static HRESULT APIENTRY VideoProcessorGetStreamExtension(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ UINT, _In_ CONST GUID*, _In_ UINT, _Out_ PVOID) noexcept;

        static HRESULT APIENTRY VideoProcessorBlt(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR, _In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW, _In_ UINT, _In_ UINT StreamCount, _In_reads_(StreamCount) CONST D3D11_1DDI_VIDEO_PROCESSOR_STREAM*) noexcept;
        D3D12TranslationLayer::BatchedVideoProcess* UnderlyingVideoProcess()
        {
            return m_UnderlyingVideoProcess.get();
        }

        // helpers
        void Blt(_In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW hVideoProcessorOutputView, _In_ UINT OutputFrame, _In_ UINT StreamCount, _In_reads_(StreamCount) CONST D3D11_1DDI_VIDEO_PROCESSOR_STREAM *pStreams);
        void FillReferenceSet(_In_ D3D12TranslationLayer::VIDEO_PROCESS_STREAM_INFO *pStreamInfo, _In_ UINT view, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW hInputSurface, _In_ UINT numPastFrames, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW* pPastSurfaces, _In_ UINT numFutureFrames, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW* pFutureSurfaces);
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS GetDeinterlaceMode(const D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS & rateConversionCaps);

    protected:
        VideoProcess(Device& parent, UINT MaxInputStreams, bool AutoProcessingSupported);
        virtual void ProcessFrames(UINT streamCount) { UnderlyingVideoProcess()->ProcessFrames(&m_inputArguments, streamCount, &m_outputArguments); }

        std::unique_ptr<D3D12TranslationLayer::BatchedVideoProcess> m_UnderlyingVideoProcess;

        const UINT m_MaxInputStreams;
        const bool m_fAutoProcessingSupported;
        D3D12TranslationLayer::VIDEO_PROCESS_INPUT_ARGUMENTS m_inputArguments;
        D3D12TranslationLayer::VIDEO_PROCESS_OUTPUT_ARGUMENTS m_outputArguments;
    };
};