// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    SIZE_T APIENTRY VideoProcess::CalcPrivateVideoProcessorSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOR * /*pCreateVideoProcessor*/)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T videoProcessSize = sizeof(VideoProcess);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, videoProcessSize);
    }

    _Use_decl_annotations_
    HRESULT APIENTRY VideoProcess::CreateVideoProcessor(D3D10DDI_HDEVICE hDevice, CONST D3D11_1DDIARG_CREATEVIDEOPROCESSOR *pCreateVideoProcessor, D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, D3D11_1DDI_HRTVIDEOPROCESSOR /*hRTVideoProcess*/)  noexcept
    {
        HRESULT hr = S_OK;
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        D3D12TranslationLayer::VIDEO_PROCESS_ENUM_ARGS createArgs = {};
        VideoProcessEnum* pVideoProcessEnum = VideoProcessEnum::CastFrom(pCreateVideoProcessor->hVideoProcessorEnum);
        pVideoProcessEnum->GetCreationArgs(&createArgs);
        D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS rateConversionCaps = pVideoProcessEnum->GetRateConversionCaps(pCreateVideoProcessor->RateConversionCapsIndex);

        new (hVideoProcess.pDrvPrivate) VideoProcess(*pDevice, createArgs.MaxInputStreams, rateConversionCaps, pVideoProcessEnum->IsAutoProcessingSupported());

        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    void APIENTRY VideoProcess::DestroyVideoProcessor(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        pVideoProcess->~VideoProcess();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
    
    D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS VideoProcess::GetDeinterlaceMode(const D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS& rateConversionCaps)
    {
        // This code depends on the enumerator only reporting adaptive and bob.
        assert ((rateConversionCaps.ConversionCaps & (D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_DEINTERLACE_BLEND | D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_DEINTERLACE_MOTION_COMPENSATION | D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_INVERSE_TELECINE)) == 0);

        // The rate conversion index may have multiple deinterlace methods associated with it.  Pick the most capable one.
        // D3D12 expects a single bit when using a video processor.
        if (rateConversionCaps.ConversionCaps & D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_DEINTERLACE_ADAPTIVE)
        {
            return D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM;
        }
        
        // BOB is the default mode selection.  Later checks determine if BOB is supported or not.
        return D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB;
    }

    VideoProcess::VideoProcess(Device& parent, UINT MaxInputStreams, const D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS& rateConversionCaps, bool AutoProcessingSupported) :
        DeviceChild(parent),
        m_UnderlyingVideoProcess(parent.GetBatchedContext(), GetDeinterlaceMode(rateConversionCaps)),
        m_MaxInputStreams(MaxInputStreams),
        m_fAutoProcessingSupported(AutoProcessingSupported)
    {
        m_inputArguments.ResetStreams(m_MaxInputStreams); // throw( bad_alloc )
    }

    static inline void GetViewInfo(D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW hView, D3D12TranslationLayer::VideoProcessView *pView)
    {
        D3D12TranslationLayer::VPIV *pVPIV = VideoProcessorInputView::CastToTranslationView(hView);

        pView->SubresourceSubset = pVPIV->m_subresources;
        pView->pResource = pVPIV->m_pResource;
    }

    static inline void GetViewInfo(D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW hView, D3D12TranslationLayer::VideoProcessView *pView, UINT view)
    {
        D3D12TranslationLayer::VPOV *pVPOV = VideoProcessorOutputView::CastToTranslationView(hView);

        // In the DX11 VPOV, a range of array slices can be specified, but they can only be adjacent. This is only used for mono and stereo
        // output, so the vpDesc.ArraySize can only be 1 for mono, 2 for stereo.
        D3D12TranslationLayer::CViewSubresourceSubset SubresourceSubset(pVPOV->m_subresources);
        SubresourceSubset.m_BeginArray += static_cast<UINT16>(view);
        SubresourceSubset.m_EndArray = SubresourceSubset.m_BeginArray + 1;
        pView->SubresourceSubset = SubresourceSubset;
        pView->pResource = pVPOV->m_pResource;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY VideoProcess::VideoProcessorBlt(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW hVideoProcessorOutputView, _In_ UINT OutputFrame, _In_ UINT StreamCount, _In_reads_(StreamCount) CONST D3D11_1DDI_VIDEO_PROCESSOR_STREAM *pStreams) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);

        pVideoProcess->Blt(hVideoProcessorOutputView, OutputFrame, StreamCount, pStreams);

        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void VideoProcess::FillReferenceSet(_In_ D3D12TranslationLayer::VIDEO_PROCESS_STREAM_INFO *pStreamInfo, _In_ UINT view, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW hInputSurface, _In_ UINT numPastFrames, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW* pPastSurfaces, _In_ UINT numFutureFrames, _In_ D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW* pFutureSurfaces)
    {
        GetViewInfo(hInputSurface, &pStreamInfo->ResourceSet[view].CurrentFrame);
        pStreamInfo->ResourceSet[view].PastFrames.resize(numPastFrames);
        for (DWORD i = 0; i < numPastFrames; i++)
        {
            GetViewInfo(pPastSurfaces[i], &pStreamInfo->ResourceSet[view].PastFrames[i]);
        }

        pStreamInfo->ResourceSet[view].FutureFrames.resize(numFutureFrames);
        for (DWORD i = 0; i < numFutureFrames; i++)
        {
            GetViewInfo(pFutureSurfaces[i], &pStreamInfo->ResourceSet[view].FutureFrames[i]);
        }
    }

    void VideoProcess::Blt(_In_ D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW hVideoProcessorOutputView, _In_ UINT OutputFrame, _In_ UINT StreamCount, _In_reads_(StreamCount) CONST D3D11_1DDI_VIDEO_PROCESSOR_STREAM *pStreams)
    {
        // OutputFrame is not used in DX12 layer, it is passed as an optimization to the driver for unchanged output or for tagging. From MSDN:
        // If the frame that the OutputFrame member specifies remains unchanged at the next process time, the driver determines that the frame is unchanged(for example, paused) in the entire video processing and composition.
        // Therefore, the driver can use cached data to optimize the frame. The driver also uses the frame that the OutputFrame member specifies for tagging the command, which the driver 
        // submits to the graphics processing unit(GPU).
        UNREFERENCED_PARAMETER(OutputFrame);

        // fill input resources
        for (DWORD streamIndex = 0; streamIndex < StreamCount; streamIndex++)
        {
            auto& streamInfo = m_inputArguments.StreamInfo[streamIndex];
            const D3D11_1DDI_VIDEO_PROCESSOR_STREAM& stream = pStreams[streamIndex];
            D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC& streamDesc = m_inputArguments.D3D12InputStreamDesc[streamIndex];

            UINT left = 0;
            UINT right = 1;

            if (streamInfo.StereoFormatSwapViews)
            {
                if (streamDesc.StereoFormat != D3D12_VIDEO_FRAME_STEREO_FORMAT_SEPARATE)
                {
                    ThrowFailure(E_INVALIDARG);
                }
                left = 1;
                right = 0;
            }
            FillReferenceSet(&streamInfo, left, stream.hInputSurface, stream.PastFrames, stream.pPastSurfaces, stream.FutureFrames, stream.pFutureSurfaces);
            if (streamDesc.StereoFormat == D3D12_VIDEO_FRAME_STEREO_FORMAT_SEPARATE)
            {
                FillReferenceSet(&streamInfo, right, stream.hInputSurfaceRight, stream.PastFrames, stream.pPastSurfacesRight, stream.FutureFrames, stream.pFutureSurfacesRight);
            }

            streamInfo.OutputIndex = stream.OutputIndex;
            streamInfo.InputFrameOrField = stream.InputFrameOrField;
        }

        // fill output resources
         GetViewInfo(hVideoProcessorOutputView, &m_outputArguments.CurrentFrame[0], 0);
         if (m_outputArguments.D3D12OutputStreamDesc.EnableStereo)
         {
             GetViewInfo(hVideoProcessorOutputView, &m_outputArguments.CurrentFrame[1], 1);
         }

        m_UnderlyingVideoProcess.ProcessFrames(&m_inputArguments, StreamCount, &m_outputArguments);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Output Arguments
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetOutputTargetRect(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ BOOL Enable, _In_ const RECT* pRect) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START(); 
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);

        static_assert(sizeof(D3D12_RECT) == sizeof(RECT), "Rects should match");
        pVideoProcess->m_outputArguments.EnableTargetRect = Enable;
        if (Enable)
        {
            pVideoProcess->m_outputArguments.D3D12OutputStreamArguments.TargetRectangle = *reinterpret_cast<const D3D12_RECT*>(pRect);
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetOutputBackgroundColor(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ BOOL YCbCr, _In_ const D3D11_1DDI_VIDEO_COLOR* pColor) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);

        static_assert(sizeof(pVideoProcess->m_outputArguments.D3D12OutputStreamDesc.BackgroundColor) == sizeof(D3D11_VIDEO_COLOR), "Colors should match");
        VideoTranslate::VideoColor(pColor,pVideoProcess->m_outputArguments.D3D12OutputStreamDesc.BackgroundColor);
        pVideoProcess->m_outputArguments.BackgroundColorYCbCr = YCbCr;
        pVideoProcess->m_outputArguments.BackgroundColorSet = true;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetOutputAlphaFillMode(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE AlphaFillMode, _In_ UINT StreamIndex) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        pVideoProcess->m_outputArguments.D3D12OutputStreamDesc.AlphaFillMode = VideoTranslate::AlphaFillMode(AlphaFillMode);
        pVideoProcess->m_outputArguments.D3D12OutputStreamDesc.AlphaFillModeSourceStreamIndex = StreamIndex;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetOutputConstriction(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR /*hVideoProcess*/, _In_ BOOL Enable, _In_ SIZE /*Size*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, Enable ? E_NOTIMPL : S_OK);   // Constriction not implemented in DX12
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetOutputStereoMode(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ BOOL Enable) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        pVideoProcess->m_outputArguments.D3D12OutputStreamDesc.EnableStereo = Enable;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY VideoProcess::VideoProcessorSetOutputExtension(_In_ D3D10DDI_HDEVICE /*hDevice*/, _In_ D3D11_1DDI_HVIDEOPROCESSOR /*hVideoProcess*/, _In_ const GUID* /*pExtensionGuid*/, _In_ UINT /*DataSize*/, _In_ void* /*pData*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY VideoProcess::VideoProcessorGetOutputExtension(_In_ D3D10DDI_HDEVICE /*hDevice*/, _In_ D3D11_1DDI_HVIDEOPROCESSOR /*hVideoProcess*/, _In_ const GUID* /*pExtensionGuid*/, _In_ UINT /*DataSize*/, _Out_ void* /*pData*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetOutputColorSpace1(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ D3DDDI_COLOR_SPACE_TYPE ColorSpace) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        pVideoProcess->m_outputArguments.D3D12OutputStreamDesc.ColorSpace = VideoTranslate::ColorSpaceType(ColorSpace);
        pVideoProcess->m_outputArguments.ColorSpaceSet = TRUE;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetOutputShaderUsage(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR /*hVideoProcess*/, _In_ BOOL /*ShaderUsage*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, E_NOTIMPL);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Input Arguments
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamFrameFormat(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ D3D11_1DDI_VIDEO_FRAME_FORMAT FrameFormat) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);

        pVideoProcess->m_inputArguments.D3D12InputStreamArguments[StreamIndex].FieldType = (D3D12_VIDEO_FIELD_TYPE)FrameFormat;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void APIENTRY VideoProcess::VideoProcessorSetStreamOutputRate(D3D10DDI_HDEVICE hDevice, D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, UINT StreamIndex, D3D11_1DDI_VIDEO_PROCESSOR_OUTPUT_RATE /*OutputRate*/, BOOL /*RepeatFrame*/, const DXGI_RATIONAL* /*pCustomRate*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        UNREFERENCED_PARAMETER(pVideoProcess);
        UNREFERENCED_PARAMETER(StreamIndex);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        // TODO: WORK item 7494569
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void APIENTRY VideoProcess::VideoProcessorSetStreamSourceRect(D3D10DDI_HDEVICE hDevice, D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, UINT StreamIndex, BOOL Enable, const RECT* pRect) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        static_assert(sizeof(D3D12_RECT) == sizeof(RECT), "Rects should match");
        pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].EnableSourceRect = Enable;
        if (Enable)
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamArguments[StreamIndex].Transform.SourceRectangle = *reinterpret_cast<const D3D12_RECT*>(pRect);

            D3D12_VIDEO_SIZE_RANGE& SourceSizeRange = pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].SourceSizeRange;
            SourceSizeRange.MaxWidth = static_cast<UINT>(pRect->right - pRect->left);
            SourceSizeRange.MinWidth = SourceSizeRange.MaxWidth;
            SourceSizeRange.MaxHeight = static_cast<UINT>(pRect->bottom - pRect->top);
            SourceSizeRange.MinHeight = SourceSizeRange.MaxHeight;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void APIENTRY VideoProcess::VideoProcessorSetStreamDestRect(D3D10DDI_HDEVICE hDevice, D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, UINT StreamIndex, BOOL Enable, const RECT* pRect) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        static_assert(sizeof(D3D12_RECT) == sizeof(RECT), "Rects should match");
        pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].EnableDestinationRect = Enable;
        if (Enable)
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamArguments[StreamIndex].Transform.DestinationRectangle = *reinterpret_cast<const D3D12_RECT*>(pRect);

            D3D12_VIDEO_SIZE_RANGE& DestinationSizeRange = pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].DestinationSizeRange;
            DestinationSizeRange.MaxWidth = static_cast<UINT>(pRect->right - pRect->left);
            DestinationSizeRange.MinWidth = DestinationSizeRange.MaxWidth;
            DestinationSizeRange.MaxHeight = static_cast<UINT>(pRect->bottom - pRect->top);
            DestinationSizeRange.MinHeight = DestinationSizeRange.MaxHeight;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamAlpha(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ BOOL Enable, _In_ FLOAT Alpha) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].EnableAlphaBlending = Enable;
        pVideoProcess->m_inputArguments.D3D12InputStreamArguments[StreamIndex].AlphaBlending.Enable = Enable;
        pVideoProcess->m_inputArguments.D3D12InputStreamArguments[StreamIndex].AlphaBlending.Alpha = Alpha;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void APIENTRY VideoProcess::VideoProcessorSetStreamPixelAspectRatio(D3D10DDI_HDEVICE hDevice, D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, UINT StreamIndex, BOOL Enable, const DXGI_RATIONAL* pSourceAspectRatio, const DXGI_RATIONAL* pDestinationAspectRatio) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        if (Enable)
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].SourceAspectRatio = *pSourceAspectRatio;
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].DestinationAspectRatio = *pDestinationAspectRatio;
        }
        else
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].SourceAspectRatio.Numerator = 1;
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].SourceAspectRatio.Denominator = 1;
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].DestinationAspectRatio.Numerator = 1;
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].DestinationAspectRatio.Denominator = 1;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamLumaKey(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ BOOL Enable, _In_ FLOAT Lower, _In_ FLOAT Upper) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].LumaKey.Enable = Enable;
        if (Enable)
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].LumaKey.Lower = Lower;
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].LumaKey.Upper = Upper;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamStereoFormat(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ BOOL Enable, _In_ D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT Format, _In_ BOOL LeftViewFrame0, _In_ BOOL BaseViewFrame0, _In_ D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FLIP_MODE FlipMode, _In_ int /*MonoOffset*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        if (Enable)
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].StereoFormat = VideoTranslate::VideoProcessStereoFormat(Format);
            if (pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].StereoFormat == D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE)
            {
                ThrowFailure(E_NOTIMPL);
            }
            if (FlipMode != D3D11_VIDEO_PROCESSOR_STEREO_FLIP_NONE)
            {
                ThrowFailure(E_NOTIMPL);
            }
            if (LeftViewFrame0 || BaseViewFrame0)
            {
                if (pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].StereoFormat != D3D12_VIDEO_FRAME_STEREO_FORMAT_SEPARATE)
                {
                    // FORMAT_VERTICAL & FORMAT_VERTICAL have the two views in one single texture. There's no way in DX12
                    // video to indicate swapped views within the texture.
                    ThrowFailure(E_NOTIMPL);
                }
                pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].StereoFormatSwapViews = true;
            }
        }
        else
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].StereoFormat = D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamAutoProcessingMode(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ BOOL Enable) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].EnableAutoProcessing = pVideoProcess->m_fAutoProcessingSupported ? Enable : false;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamFilter(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER Filter, _In_ BOOL Enable, _In_ int Level) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        D3D12_VIDEO_PROCESS_FILTER_FLAGS Filter12Flag = VideoTranslate::VideoProcessFilterFlag(Filter);
        if (Enable)
        {
            D3D12_VIDEO_PROCESS_FILTER Filter12 = VideoTranslate::VideoProcessFilter(Filter);
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].FilterFlags |= Filter12Flag;
            pVideoProcess->m_inputArguments.D3D12InputStreamArguments[StreamIndex].FilterLevels[Filter12] = Level;
        }
        else
        {
            pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].FilterFlags &= ~Filter12Flag;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamRotation(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ BOOL Enable, _In_ D3D11_1DDI_VIDEO_PROCESSOR_ROTATION Rotation) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        ASSERT(StreamIndex < pVideoProcess->m_MaxInputStreams);
        if (Enable)
        {
            D3D12_VIDEO_PROCESS_ORIENTATION orientation = VideoTranslate::VideoProcessOrientation(Rotation);            
            pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].OrientationInfo.Rotation = orientation;
        }
        else
        {
            pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].OrientationInfo.Rotation = D3D12_VIDEO_PROCESS_ORIENTATION_DEFAULT;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY VideoProcess::VideoProcessorSetStreamExtension(_In_ D3D10DDI_HDEVICE /*hDevice*/, _In_ D3D11_1DDI_HVIDEOPROCESSOR /*hVideoProcess*/, _In_ UINT /*StreamIndex*/, _In_ const GUID* /*pExtensionGuid*/, _In_ UINT /*DataSize*/, _In_ void* /*pData*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    HRESULT APIENTRY VideoProcess::VideoProcessorGetStreamExtension(_In_ D3D10DDI_HDEVICE /*hDevice*/, _In_ D3D11_1DDI_HVIDEOPROCESSOR /*hVideoProcess*/,  _In_ UINT /*StreamIndex*/, _In_ const GUID* /*pExtensionGuid*/, _In_ UINT /*DataSize*/, _Out_ void* /*pData*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(E_NOTIMPL);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamColorSpace1(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ D3DDDI_COLOR_SPACE_TYPE ColorSpace) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        pVideoProcess->m_inputArguments.D3D12InputStreamDesc[StreamIndex].ColorSpace = VideoTranslate::ColorSpaceType(ColorSpace);
        pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].ColorSpaceSet = TRUE;
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void APIENTRY VideoProcess::VideoProcessorSetStreamMirror(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HVIDEOPROCESSOR hVideoProcess, _In_ UINT StreamIndex, _In_ BOOL Enable, _In_ BOOL FlipHorizontal, _In_ BOOL FlipVertical) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoProcess *pVideoProcess = VideoProcess::CastFrom(hVideoProcess);
        if (Enable)
        {
            pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].OrientationInfo.FlipHorizontal = FlipHorizontal;
            pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].OrientationInfo.FlipVertical = FlipVertical;
        }
        else
        {
            pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].OrientationInfo.FlipHorizontal = FALSE;
            pVideoProcess->m_inputArguments.StreamInfo[StreamIndex].OrientationInfo.FlipVertical = FALSE;
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }
};