// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    const UINT DECODER_EXTENSION_STATUS_REPORT_FUNCTION = 7;

    SIZE_T APIENTRY VideoDecode::CalcPrivateVideoDecoderSize(_In_ D3D10DDI_HDEVICE hDevice, _In_ CONST D3D11_1DDIARG_CREATEVIDEODECODER * /*pCreateVideoDecoder*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        const SIZE_T videoDecodeSize = sizeof(VideoDecode);
#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, videoDecodeSize);
    }

    _Use_decl_annotations_
    HRESULT APIENTRY VideoDecode::CreateVideoDecoder(D3D10DDI_HDEVICE hDevice, CONST D3D11_1DDIARG_CREATEVIDEODECODER *pCreateVideoDecoder, D3D11_1DDI_HDECODE hVideoDecode, D3D11_1DDI_HRTDECODE /*hRTVideoDecode*/) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        HRESULT hr = S_OK;
        Device *pDevice = Device::CastFrom(hDevice);

        if (pCreateVideoDecoder->Config.guidConfigMBcontrolEncryption != DXVA_NoEncrypt ||
            pCreateVideoDecoder->Config.guidConfigResidDiffEncryption != DXVA_NoEncrypt ||
            pCreateVideoDecoder->Config.ConfigBitstreamRaw == 0                        ||
            pCreateVideoDecoder->Config.ConfigResidDiffHost != 0                       ||
            pCreateVideoDecoder->Config.ConfigSpatialResid8 != 0                       ||
            pCreateVideoDecoder->Config.ConfigResid8Subtraction != 0                   ||
            pCreateVideoDecoder->Config.ConfigSpatialHost8or9Clipping != 0             ||
            pCreateVideoDecoder->Config.ConfigSpatialResidInterleaved != 0             ||
            pCreateVideoDecoder->Config.ConfigIntraResidUnsigned != 0                  ||
            pCreateVideoDecoder->Config.Config4GroupedCoefs != 0)
        {
            ThrowFailure(E_INVALIDARG);
        }

        D3D12TranslationLayer::VideoDecodeCreationArgs createArgs = {};
        createArgs.Desc.DecodeProfile = pCreateVideoDecoder->Desc.Guid;
        createArgs.Desc.Width = pCreateVideoDecoder->Desc.SampleWidth;
        createArgs.Desc.Height = pCreateVideoDecoder->Desc.SampleHeight;
        createArgs.Desc.DecodeFormat = pCreateVideoDecoder->Desc.OutputFormat;
        createArgs.Config.ConfigDecoderSpecific = pCreateVideoDecoder->Config.ConfigDecoderSpecific;
        createArgs.Config.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;

        new (hVideoDecode.pDrvPrivate) VideoDecode(*pDevice, createArgs, pCreateVideoDecoder->Config.guidConfigBitstreamEncryption);

        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    void APIENTRY VideoDecode::DestroyVideoDecoder(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HDECODE hVideoDecode) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
        pVideoDecode->~VideoDecode();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);

    }

    VideoDecode::VideoDecode(Device& parent,
                             D3D12TranslationLayer::VideoDecodeCreationArgs const& args,
                             GUID const& bitstreamEncryption) :
        DeviceChild(parent),
        m_UnderlyingVideoDecode(parent.GetBatchedContext(), args),
        m_decodeProfile(args.Desc.DecodeProfile),
        m_bitstreamEncryption(bitstreamEncryption),
        m_frameNestCount(0)
    {
        ZeroMemory(&m_inputArguments, sizeof(m_inputArguments));
        ZeroMemory(&m_outputArguments, sizeof(m_outputArguments));
    }

    HRESULT APIENTRY VideoDecode::VideoDecoderExtension(_In_ D3D10DDI_HDEVICE /*hDevice*/, _In_ D3D11_1DDI_HDECODE hVideoDecode, _In_ CONST D3D11_1DDIARG_VIDEODECODEREXTENSION *pVideoDecoderExtension) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        HRESULT hr = S_OK;
        if (pVideoDecoderExtension->Function == DECODER_EXTENSION_STATUS_REPORT_FUNCTION)
        {
            VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
            hr = pVideoDecode->UnderlyingVideoDecode()->GetDecodingStatus(pVideoDecoderExtension->pPrivateOutputData, pVideoDecoderExtension->PrivateOutputDataSize);
        }
        else
        {
            hr = E_NOTIMPL;           // no other video decoder extensions in DX11 will be dealt with
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY VideoDecode::VideoDecoderBeginFrame(_In_ D3D10DDI_HDEVICE /*hDevice*/, _In_ D3D11_1DDI_HDECODE hVideoDecode, _In_ CONST D3DWDDM2_4DDIARG_VIDEODECODERBEGINFRAME *pVideoDecoderBeginFrame) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
        pVideoDecode->BeginFrame(pVideoDecoderBeginFrame);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void APIENTRY VideoDecode::VideoDecoderEndFrame(_In_ D3D10DDI_HDEVICE hDevice, _In_ D3D11_1DDI_HDECODE hVideoDecode) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
        pVideoDecode->EndFrame();
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    HRESULT APIENTRY VideoDecode::VideoDecoderSubmitBuffers(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HDECODE hVideoDecode, _In_ UINT BufferCount, _In_reads_(BufferCount) CONST D3D11_1DDI_VIDEO_DECODER_BUFFER_DESC *pVideoDecoderBufferDesc) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        HRESULT hr = S_OK;
        VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
        for (UINT i = 0; i < BufferCount; i++)
        {
            D3DWDDM2_4DDI_VIDEO_DECODER_BUFFER_DESC desc1 = {};

            // Map to D3DWDDM2_0DDI_VIDEO_DECODER_BUFFER_DESC1:
            //  1. From D3D11_VIDEO_DECODER_BUFFER_DESC  structure, dwWidth, dwHeight, dwStride, ReservedBits and BufferIndex are ignored (reserved);
            //  2. FirstMBaddress and NumMBsInBuffer not used as per codec team.

            desc1.hResource = pVideoDecoderBufferDesc[i].hResource;
            desc1.BufferType = pVideoDecoderBufferDesc[i].BufferType;
            desc1.DataOffset = pVideoDecoderBufferDesc[i].DataOffset;
            desc1.DataSize = pVideoDecoderBufferDesc[i].DataSize;
            desc1.pIV = pVideoDecoderBufferDesc[i].pIV;
            desc1.IVSize = pVideoDecoderBufferDesc[i].IVSize;
            pVideoDecode->SubmitBuffer(&desc1);
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    HRESULT APIENTRY VideoDecode::VideoDecoderSubmitBuffers2(_In_ D3D10DDI_HDEVICE, _In_ D3D11_1DDI_HDECODE hVideoDecode, _In_ UINT BufferCount, _In_reads_(BufferCount) CONST D3DWDDM2_4DDI_VIDEO_DECODER_BUFFER_DESC* pVideoDecoderBufferDesc) noexcept
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        HRESULT hr = S_OK;
        VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
        for (UINT i = 0; i < BufferCount; i++)
        {
            pVideoDecode->SubmitBuffer(&pVideoDecoderBufferDesc[i]);
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(hr);
    }

    void VideoDecode::BeginFrame(_In_ const D3DWDDM2_4DDIARG_VIDEODECODERBEGINFRAME *pVideoDecoderBeginFrame)
    {
        // we do not allow nested BeginFrames in 11on12. Ignoring nested calls.
        if (m_frameNestCount == 0)
        {
            ZeroMemory(&m_inputArguments, sizeof(m_inputArguments));
            ZeroMemory(&m_outputArguments, sizeof(m_outputArguments));

            if (m_bitstreamEncryption == D3D11_DECODER_ENCRYPTION_HW_CENC  &&  pVideoDecoderBeginFrame->ContentKeySize != sizeof(D3D11_VIDEO_DECODER_BEGIN_FRAME_CRYPTO_SESSION))
            {
                const D3DWDDM2_0DDI_VIDEO_DECODER_BEGIN_FRAME_CRYPTO_SESSION *pBeginFrameCryptoSession = static_cast<const D3DWDDM2_0DDI_VIDEO_DECODER_BEGIN_FRAME_CRYPTO_SESSION *>(pVideoDecoderBeginFrame->pContentKey);
                m_inputArguments.DecryptionArguments.pKeyInfo = pBeginFrameCryptoSession->pBlob;
                m_inputArguments.DecryptionArguments.KeyInfoSize = pBeginFrameCryptoSession->BlobSize;
                // 11on12: KeyInfoID not used.

                // TODO: work item 7147060 - pPrivateData and PrivateDataSize are used for transcryption, use it after implementing NegotiateCryptoSessionKeyExchange
            }
            
            D3D12TranslationLayer::VDOV *pVDOV = VideoDecoderOutputView::CastToTranslationView(pVideoDecoderBeginFrame->hOutputView);
            m_outputArguments.pOutputTexture2D = pVDOV->m_pResource;
            m_outputArguments.SubresourceSubset = pVDOV->m_subresources;
            m_inUseResources.output = pVDOV->m_pResource;

            static_assert(D3D12TranslationLayer::VIDEO_DECODE_MAX_HISTOGRAM_COMPONENTS == D3DWDDM2_4DDI_VIDEO_DECODER_MAX_HISTOGRAM_COMPONENTS, "Must keep histogram component count in sync");

            for (UINT i = 0; i < _countof(pVideoDecoderBeginFrame->Histograms); i++)
            {
                const D3DWDDM2_4DDI_VIDEO_DECODER_HISTOGRAM& DDIHistogram = pVideoDecoderBeginFrame->Histograms[i];
                D3D12TranslationLayer::VIDEO_DECODE_COMPONENT_HISTOGRAM& TLHistogram = m_outputArguments.Histograms[i];
                
                if (DDIHistogram.hBuffer.pDrvPrivate)
                {
                    Resource* pBuffer = Resource::CastFrom(DDIHistogram.hBuffer);
                    TLHistogram.pBuffer = pBuffer->ImmediateResource();
                    TLHistogram.Offset = DDIHistogram.Offset;
                    m_inUseResources.histogramBuffers[i] = pBuffer->ImmediateResource();
                }
            }
        }
        ++m_frameNestCount;
    }

    void VideoDecode::SubmitBuffer(_In_ const D3DWDDM2_4DDI_VIDEO_DECODER_BUFFER_DESC *pVideoDecoderBufferDesc)
    {
        // TODO: How is this going to work for VP9? What are IHVs using for the probability buffer type currently? Gotta make a standard on this, or we won't be able to do 11on12 for VP9.

        if (m_inputArguments.FrameArgumentsCount >= D3D12_VIDEO_DECODE_MAX_ARGUMENTS)
        {
            ThrowFailure(E_INVALIDARG);
        }

        switch (pVideoDecoderBufferDesc->BufferType)
        {
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS:
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_SLICE_CONTROL:
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX:
            {
                Resource* pResource = Resource::CastFrom(pVideoDecoderBufferDesc->hResource);
                D3D12TranslationLayer::MappedSubresource mappedSubresource;

                m_parentDevice.GetBatchedContext().MapUnderlyingSynchronize(&pResource->BatchedResource(), 0, D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READ, false, nullptr, &mappedSubresource);
                m_inputArguments.FrameArguments[m_inputArguments.FrameArgumentsCount].Type = VideoTranslate::VideoDecodeArgumentType(pVideoDecoderBufferDesc->BufferType);
                m_inputArguments.FrameArguments[m_inputArguments.FrameArgumentsCount].pData = mappedSubresource.pData;
                m_inputArguments.FrameArguments[m_inputArguments.FrameArgumentsCount].Size = pVideoDecoderBufferDesc->DataSize;
                m_inUseResources.frameArguments[m_inputArguments.FrameArgumentsCount] = pResource->ImmediateResource();
                ++m_inputArguments.FrameArgumentsCount;
            }
            break;

        case D3D11_1DDI_VIDEO_DECODER_BUFFER_BITSTREAM:
            {
                // TODO: assume it all fits for now, no bad slice chopping (see work items 5743248/5743254)
                Resource* pResource = Resource::CastFrom(pVideoDecoderBufferDesc->hResource);
                D3D12TranslationLayer::Resource *pTranslationResource = pResource->ImmediateResource();
                m_inputArguments.CompressedBitstream.pBuffer = pTranslationResource;
                m_inputArguments.CompressedBitstream.Offset = pVideoDecoderBufferDesc->DataOffset;
                m_inputArguments.CompressedBitstream.Size = pVideoDecoderBufferDesc->DataSize;
                m_inputArguments.DecryptionArguments.pIV = pVideoDecoderBufferDesc->pIV;
                m_inputArguments.DecryptionArguments.IVSize = pVideoDecoderBufferDesc->IVSize;
                m_inputArguments.DecryptionArguments.pSubSampleMappingBlock = pVideoDecoderBufferDesc->pSubSampleMappingBlock;
                m_inputArguments.DecryptionArguments.SubSampleMappingCount = pVideoDecoderBufferDesc->SubSampleMappingCount;
                m_inputArguments.DecryptionArguments.cBlocksStripeEncrypted = pVideoDecoderBufferDesc->cBlocksStripeEncrypted;
                m_inputArguments.DecryptionArguments.cBlocksStripeClear = pVideoDecoderBufferDesc->cBlocksStripeClear;
                m_inUseResources.compressedBitstream = pResource->ImmediateResource();
            }
            break;

        case D3D11_1DDI_VIDEO_DECODER_BUFFER_UNKNOWN:
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_MACROBLOCK_CONTROL:
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_RESIDUAL_DIFFERENCE:
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_DEBLOCKING_CONTROL:
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_MOTION_VECTOR:
        case D3D11_1DDI_VIDEO_DECODER_BUFFER_FILM_GRAIN:
            ThrowFailure(E_INVALIDARG);
        }
    }

    void VideoDecode::EndFrame()
    {
        // we do not allow nested BeginFrames in 11on12. Ignoring nested calls.
        if (m_frameNestCount == 1)
        {
            UnderlyingVideoDecode()->DecodeFrame(&m_inputArguments, &m_outputArguments);

            // now unmap frame arguments
            for (UINT i = 0; i < m_inputArguments.FrameArgumentsCount; i++)
            {
                assert(m_inUseResources.frameArguments[i]);
                m_parentDevice.FlushBatchAndGetImmediateContext().Unmap(m_inUseResources.frameArguments[i].get(), 0, D3D12TranslationLayer::MAP_TYPE::MAP_TYPE_READ, nullptr);
            }
            m_inUseResources = decltype(m_inUseResources){};
        }
        --m_frameNestCount;
    }

    HRESULT APIENTRY VideoDecode::VideoDecoderEnableDownsampling(_In_ D3D10DDI_HDEVICE /* hDevice */, _In_ D3D11_1DDI_HDECODE hVideoDecode, _In_ D3DDDI_COLOR_SPACE_TYPE InputColorSpace, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace, _In_ UINT ReferenceFrameCount)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
        pVideoDecode->EnableDownsampling(InputColorSpace, pOutputDesc, OutputColorSpace, ReferenceFrameCount);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void VideoDecode::EnableDownsampling(_In_ D3DDDI_COLOR_SPACE_TYPE InputColorSpace, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace, _In_ UINT ReferenceFrameCount)
    {
        m_outputArguments.ConversionArguments.OutputColorSpace = VideoTranslate::ColorSpaceType(OutputColorSpace);
        m_outputArguments.ConversionArguments.ReferenceFrameCount = ReferenceFrameCount;
        m_outputArguments.ConversionArguments.ReferenceInfo.Format.ColorSpace = VideoTranslate::ColorSpaceType(InputColorSpace);
        m_outputArguments.ConversionArguments.ReferenceInfo.Format.Format = pOutputDesc->OutputFormat;
        m_outputArguments.ConversionArguments.ReferenceInfo.Width = pOutputDesc->SampleWidth;
        m_outputArguments.ConversionArguments.ReferenceInfo.Height = pOutputDesc->SampleHeight;
        m_outputArguments.ConversionArguments.Enable = TRUE;
    }

    HRESULT APIENTRY VideoDecode::VideoDecoderUpdateDownsampling(_In_ D3D10DDI_HDEVICE /* hDevice */, _In_ D3D11_1DDI_HDECODE hVideoDecode, _In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        VideoDecode *pVideoDecode = VideoDecode::CastFrom(hVideoDecode);
        pVideoDecode->UpdateDownsampling(pOutputDesc, OutputColorSpace);
        D3D11on12_DDI_ENTRYPOINT_END_AND_RETURN_HR(S_OK);
    }

    void VideoDecode::UpdateDownsampling(_In_ CONST D3D11_1DDI_VIDEO_DECODER_DESC* pOutputDesc, _In_ D3DDDI_COLOR_SPACE_TYPE OutputColorSpace)
    {
        m_outputArguments.ConversionArguments.OutputColorSpace = VideoTranslate::ColorSpaceType(OutputColorSpace);
        m_outputArguments.ConversionArguments.ReferenceInfo.Format.Format = pOutputDesc->OutputFormat;
        m_outputArguments.ConversionArguments.ReferenceInfo.Width = pOutputDesc->SampleWidth;
        m_outputArguments.ConversionArguments.ReferenceInfo.Height = pOutputDesc->SampleHeight;
        m_outputArguments.ConversionArguments.Enable = TRUE;
    }
};