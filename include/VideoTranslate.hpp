// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    DEFINE_GUID(DXVA_NoEncrypt, 0x1b81beD0, 0xa0c7, 0x11d3, 0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5);

    class VideoTranslate
    {
        static inline UINT GetBitstreamRawType(GUID Profile)
        {
            struct BitstreamTypePair {
                const GUID* DecodeProfile;
                UINT BitstreamRawType;
            };

            static constexpr BitstreamTypePair map[] =
            {
                // H264 VLD SHORT slice = 2. (H264 DXVA Spec: 2.2 Semantics)
                &D3D11_1DDI_DECODER_PROFILE_H264_VLD_FGT,                      2,
                &D3D11_1DDI_DECODER_PROFILE_H264_VLD_NOFGT,                    2,
                &D3D11_1DDI_DECODER_PROFILE_H264_VLD_WITHFMOASO_NOFGT,         2,
                &D3D11_1DDI_DECODER_PROFILE_H264_VLD_STEREO_PROGRESSIVE_NOFGT, 2,
                &D3D11_1DDI_DECODER_PROFILE_H264_VLD_STEREO_NOFGT,             2,
                &D3D11_1DDI_DECODER_PROFILE_H264_VLD_MULTIVIEW_NOFGT,          2,
            };

            for (UINT Index = 0; Index < ARRAYSIZE(map); Index++)
            {
                if (IsEqualGUID(*map[Index].DecodeProfile, Profile))
                {
                    return map[Index].BitstreamRawType;
                }
            }

            return 1; // All other decode profiles should use bitstream type 1.
        }

    public:
        static inline void VideoDecodeDesc(_In_ const D3D11_1DDI_VIDEO_DECODER_DESC *pDesc11, UINT APINodeMask, _Out_ D3D12_VIDEO_DECODER_DESC *pDesc12)
        {
            pDesc12->NodeMask = APINodeMask;
            pDesc12->Configuration.DecodeProfile = pDesc11->Guid;
            pDesc12->Configuration.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
            pDesc12->Configuration.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
        }

        static inline void VideoDecodeDesc(_In_ const D3D11_1DDI_VIDEO_DECODER_DESC *pDesc11, _Out_ D3D12TranslationLayer::VIDEO_DECODE_DESC *pDesc)
        {
            pDesc->DecodeProfile = pDesc11->Guid;
            pDesc->Width = pDesc11->SampleWidth;
            pDesc->Height = pDesc11->SampleHeight;
            pDesc->DecodeFormat = pDesc11->OutputFormat;
        }

        static inline void VideoDecodeBufferInfo(D3D12TranslationLayer::VIDEO_DECODE_BUFFER_TYPE Type, UINT Size, _Out_ D3D11_1DDI_VIDEO_DECODER_BUFFER_INFO *pBufferInfo11)
        {
            pBufferInfo11->Type = VideoDecodeBufferType(Type);
            pBufferInfo11->Size = Size;
            pBufferInfo11->Usage = D3D11_USAGE_DYNAMIC;
        }

        static inline D3D11_DDI_VIDEO_DECODER_BUFFER_TYPE VideoDecodeBufferType(D3D12TranslationLayer::VIDEO_DECODE_BUFFER_TYPE Type)
        {
            static constexpr D3D11_DDI_VIDEO_DECODER_BUFFER_TYPE map[] =
            {
                D3D11_1DDI_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS,
                D3D11_1DDI_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX,
                D3D11_1DDI_VIDEO_DECODER_BUFFER_SLICE_CONTROL,
                D3D11_1DDI_VIDEO_DECODER_BUFFER_BITSTREAM,
                D3D11_1DDI_VIDEO_DECODER_BUFFER_UNKNOWN                     // TODO: what to do for VP9 probability buffer???
            };
            if (Type < D3D12TranslationLayer::VIDEO_DECODE_BUFFER_TYPE_SIZEOF)
            {
                return map[Type];
            }
            else
            {
                return D3D11_1DDI_VIDEO_DECODER_BUFFER_UNKNOWN;
            }
        }

        static inline D3D12_VIDEO_DECODE_ARGUMENT_TYPE VideoDecodeArgumentType(D3D11_DDI_VIDEO_DECODER_BUFFER_TYPE Type11)
        {
            static constexpr D3D12_VIDEO_DECODE_ARGUMENT_TYPE map[] =
            {
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_INVERSE_QUANTIZATION_MATRIX,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_SLICE_CONTROL,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID,
                D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID,
            };
            if (Type11 <= D3D11_1DDI_VIDEO_DECODER_BUFFER_FILM_GRAIN)
            {
                return map[Type11];
            }
            else
            {
                return D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID;
            }
        }

        static inline void VideoDecodeConfig(_In_ const D3D12TranslationLayer::VIDEO_DECODE_DESC *pDesc, _In_ const D3D12TranslationLayer::VIDEO_DECODE_CONFIG *pConfig, _Out_ D3D11_1DDI_VIDEO_DECODER_CONFIG *pConfig11)
        {
            ZeroMemory(pConfig11, sizeof(*pConfig11));
            pConfig11->guidConfigMBcontrolEncryption = DXVA_NoEncrypt;
            pConfig11->guidConfigResidDiffEncryption = DXVA_NoEncrypt;
            pConfig11->guidConfigBitstreamEncryption = DXVA_NoEncrypt;
            pConfig11->ConfigBitstreamRaw = GetBitstreamRawType(pDesc->DecodeProfile);                      // always VLD, always SHORT slice
            pConfig11->ConfigDecoderSpecific = pConfig->ConfigDecoderSpecific;
            pConfig11->ConfigMinRenderTargetBuffCount = 0;                                                  // TODO: what to do here? Talk to Shyam
        }

        static inline void VideoDecodeConfig(_In_ const D3D11_1DDI_VIDEO_DECODER_CONFIG *pConfig11, _Out_ D3D12TranslationLayer::VIDEO_DECODE_CONFIG *pConfig)
        {
            pConfig->ConfigDecoderSpecific = pConfig11->ConfigDecoderSpecific;
        }

        static inline D3D12_VIDEO_FRAME_STEREO_FORMAT VideoProcessStereoFormat(_In_ D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT Format)
        {
            static constexpr D3D12_VIDEO_FRAME_STEREO_FORMAT map[] =
            {
                D3D12_VIDEO_FRAME_STEREO_FORMAT_MONO,       // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_MONO = 0,
                D3D12_VIDEO_FRAME_STEREO_FORMAT_HORIZONTAL, // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_HORIZONTAL = 1,
                D3D12_VIDEO_FRAME_STEREO_FORMAT_VERTICAL,   // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_VERTICAL = 2,
                D3D12_VIDEO_FRAME_STEREO_FORMAT_SEPARATE,   // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_SEPARATE = 3,
                D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,       // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_MONO_OFFSET = 4,
                D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,       // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_ROW_INTERLEAVED = 5,
                D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,       // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_COLUMN_INTERLEAVED = 6,
                D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,       // D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_CHECKERBOARD
            };
            return map[Format];
        }

        static inline D3D12_VIDEO_PROCESS_FILTER_FLAGS VideoProcessFilterFlag(_In_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER Filter)
        {
            static constexpr D3D12_VIDEO_PROCESS_FILTER_FLAGS map[] =
            {
                D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS,              // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_BRIGHTNESS = 0,
                D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST,                // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CONTRAST = 1,
                D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE,                     // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_HUE = 2,
                D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION,              // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_SATURATION = 3,
                D3D12_VIDEO_PROCESS_FILTER_FLAG_NOISE_REDUCTION,         // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_NOISE_REDUCTION = 4,
                D3D12_VIDEO_PROCESS_FILTER_FLAG_EDGE_ENHANCEMENT,        // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_EDGE_ENHANCEMENT = 5,
                D3D12_VIDEO_PROCESS_FILTER_FLAG_ANAMORPHIC_SCALING,      // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_ANAMORPHIC_SCALING = 6,
                D3D12_VIDEO_PROCESS_FILTER_FLAG_STEREO_ADJUSTMENT,       // D3D11_1DDI_VIDEO_PROCESSOR_FILTER_STEREO_ADJUSTMENT = 7
            };
            return map[Filter];
        }

        static inline D3D12_VIDEO_PROCESS_FILTER VideoProcessFilter(_In_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER Filter)
        {
            // we assume identical filters for DX11/DX12
            static_assert(
                   D3D11_1DDI_VIDEO_PROCESSOR_FILTER_BRIGHTNESS         == D3D12_VIDEO_PROCESS_FILTER_BRIGHTNESS
                && D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CONTRAST           == D3D12_VIDEO_PROCESS_FILTER_CONTRAST
                && D3D11_1DDI_VIDEO_PROCESSOR_FILTER_HUE                == D3D12_VIDEO_PROCESS_FILTER_HUE
                && D3D11_1DDI_VIDEO_PROCESSOR_FILTER_SATURATION         == D3D12_VIDEO_PROCESS_FILTER_SATURATION
                && D3D11_1DDI_VIDEO_PROCESSOR_FILTER_NOISE_REDUCTION    == D3D12_VIDEO_PROCESS_FILTER_NOISE_REDUCTION
                && D3D11_1DDI_VIDEO_PROCESSOR_FILTER_EDGE_ENHANCEMENT   == D3D12_VIDEO_PROCESS_FILTER_EDGE_ENHANCEMENT
                && D3D11_1DDI_VIDEO_PROCESSOR_FILTER_ANAMORPHIC_SCALING == D3D12_VIDEO_PROCESS_FILTER_ANAMORPHIC_SCALING
                && D3D11_1DDI_VIDEO_PROCESSOR_FILTER_STEREO_ADJUSTMENT  == D3D12_VIDEO_PROCESS_FILTER_STEREO_ADJUSTMENT,
                "Filter enum values are assumed to be identical for DX11/DX12");

            return static_cast<D3D12_VIDEO_PROCESS_FILTER>(Filter);
        }

        static inline D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE AlphaFillMode(_In_ D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE AlphaFillMode)
        {
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE_OPAQUE        == D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_OPAQUE, "11 and 12 fill modes should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE_BACKGROUND    == D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_BACKGROUND, "11 and 12 fill modes should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE_DESTINATION   == D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_DESTINATION, "11 and 12 fill modes should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE_SOURCE_STREAM == D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_SOURCE_STREAM, "11 and 12 fill modes should match");

            return (D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE)AlphaFillMode;
        }

        static inline void VideoColor(_In_ const D3D11_1DDI_VIDEO_COLOR *pInput, _Out_writes_all_(4) float Output[4])
        {
            Output[0] = pInput->RGBA.R;
            Output[1] = pInput->RGBA.G;
            Output[2] = pInput->RGBA.B;
            Output[3] = pInput->RGBA.A;
        }

        static inline D3D12_VIDEO_PROCESS_ORIENTATION VideoProcessOrientation(_In_ D3D11_1DDI_VIDEO_PROCESSOR_ROTATION Rotation)
        {
            static constexpr D3D12_VIDEO_PROCESS_ORIENTATION map[] =
            {
                D3D12_VIDEO_PROCESS_ORIENTATION_DEFAULT,       // D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_IDENTITY = 0,
                D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_90,  // D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_90 = 1,
                D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_180, // D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_180 = 2,
                D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_270, // D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_270 = 3
            };
            return map[Rotation];
        }

        static inline DXGI_COLOR_SPACE_TYPE ColorSpaceType(_In_ D3DDDI_COLOR_SPACE_TYPE ColorSpace)
        {
            static constexpr DXGI_COLOR_SPACE_TYPE map[] =
            {
                DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,           // D3DDDI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 = 0,
                DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 ,          // D3DDDI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 = 1,
                DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709,         // D3DDDI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709 = 2,
                DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020,        // D3DDDI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020 = 3,
                DXGI_COLOR_SPACE_RESERVED,                         // D3DDDI_COLOR_SPACE_RESERVED = 4,
                DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601 ,   // D3DDDI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601 = 5,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601 ,      // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601 = 6,
                DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601 ,        // D3DDDI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601 = 7,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 ,      // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 = 8,
                DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709 ,        // D3DDDI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709 = 9,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020 ,     // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020 = 10,
                DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020 ,       // D3DDDI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020 = 11,
                DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020,        // D3DDDI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 = 12,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020,    // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020 = 13,
                DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020 ,     // D3DDDI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020 = 14,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020 ,  // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020 = 15,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020, // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020 = 16,
                DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020,          // D3DDDI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020 = 17,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_GHLG_TOPLEFT_P2020,  // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_GHLG_TOPLEFT_P2020 = 18,
                DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020,    // D3DDDI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020 = 19,
                DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P709,         // D3DDDI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P709 = 20,
                DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P2020,        // D3DDDI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P2020 = 21,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P709,       // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P709 = 22,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P2020,      // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P2020 = 23,
                DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_TOPLEFT_P2020,   // D3DDDI_COLOR_SPACE_YCBCR_STUDIO_G24_TOPLEFT_P2020 = 24,
                DXGI_COLOR_SPACE_CUSTOM,                           // D3DDDI_COLOR_SPACE_CUSTOM = 0xFFFFFFFF
            };
            return map[ColorSpace];
        }

        static inline D3D12_VIDEO_FIELD_TYPE VideoFieldType(D3D11_1DDI_VIDEO_FRAME_FORMAT dx11FrameFormat)
        {
            static constexpr D3D12_VIDEO_FIELD_TYPE map[] =
            {
                D3D12_VIDEO_FIELD_TYPE_NONE,                           // D3D11_1DDI_VIDEO_FRAME_FORMAT_PROGRESSIVE
                D3D12_VIDEO_FIELD_TYPE_INTERLACED_TOP_FIELD_FIRST,     // D3D11_1DDI_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST
                D3D12_VIDEO_FIELD_TYPE_INTERLACED_BOTTOM_FIELD_FIRST,  // D3D11_1DDI_VIDEO_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST
            };
            return map[dx11FrameFormat];
        }

        static inline UINT MapFlags(const UINT *pMap, UINT inputFlags)
        {
            UINT index = 0;
            UINT outputFlags = 0;
            while (inputFlags)
            {
                if (inputFlags & 0x1)
                {
                    outputFlags |= pMap[index];
                }
                inputFlags >>= 1;
                ++index;
            }
            return outputFlags;
        }

        static bool IsSupported(_In_ D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT &dx12Support)
        {
            return (dx12Support.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) == D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED;
        }

        static inline bool HasDeinterlaceSupport( const _In_ D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT &dx12Support )
        {
            return dx12Support.DeinterlaceSupport != 0;
        }

        static inline UINT ToDX11VideoProcessorDeviceCaps(UINT translationLayerConversionCaps)
        {
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_DEVICE_CAPS_LINEAR_SPACE            == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_LINEAR_SPACE &&
                          D3D11_1DDI_VIDEO_PROCESSOR_DEVICE_CAPS_xvYCC                   == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_xvYCC &&
                          D3D11_1DDI_VIDEO_PROCESSOR_DEVICE_CAPS_RGB_RANGE_CONVERSION    == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_RGB_RANGE_CONVERSION &&
                          D3D11_1DDI_VIDEO_PROCESSOR_DEVICE_CAPS_YCbCr_MATRIX_CONVERSION == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_YCbCr_MATRIX_CONVERSION &&
                          D3D11_1DDI_VIDEO_PROCESSOR_DEVICE_CAPS_NOMINAL_RANGE           == D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS_NOMINAL_RANGE,
                "DeviceCaps must match");

            return translationLayerConversionCaps;
        }

        static inline void AddVideoProcessCaps(_In_ D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT &dx12OrigSupport, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_CAPS &dx11Caps, _Out_ std::vector<D3D11_1DDI_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS> &dx11RateConversionCaps, _Out_ D3D11_1DDI_VIDEO_PROCESSOR_FILTER_RANGE *pFilterRanges, _In_ UINT cFilterRanges, _In_ D3D12TranslationLayer::VIDEO_PROCESS_CONVERSION_CAPS colorConversionCap)
        {
            UNREFERENCED_PARAMETER(cFilterRanges);
            D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT dx12Support;
            memcpy(&dx12Support, &dx12OrigSupport, sizeof(dx12Support));

            static constexpr UINT mapFeature[] =
            {
                D3D11_1DDI_VIDEO_PROCESSOR_FEATURE_CAPS_ALPHA_FILL,          // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_FILL
                D3D11_1DDI_VIDEO_PROCESSOR_FEATURE_CAPS_LUMA_KEY,            // D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY
                D3D11_1DDI_VIDEO_PROCESSOR_FEATURE_CAPS_STEREO,              // D3D12_VIDEO_PROCESS_FEATURE_FLAG_STEREO
                D3D11_1DDI_VIDEO_PROCESSOR_FEATURE_CAPS_ROTATION,            // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ROTATION
                D3D11_1DDI_VIDEO_PROCESSOR_FEATURE_CAPS_MIRROR,              // D3D12_VIDEO_PROCESS_FEATURE_FLAG_FLIP
                D3D11_1DDI_VIDEO_PROCESSOR_FEATURE_CAPS_ALPHA_STREAM,        // D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING
                D3D11_1DDI_VIDEO_PROCESSOR_FEATURE_CAPS_PIXEL_ASPECT_RATIO,  // D3D12_VIDEO_PROCESS_FEATURE_FLAG_PIXEL_ASPECT_RATIO
                0,                                                           // D3D12_VIDEO_PROCESS_FEATURE_FLAG_AUTO_PROCESSING
            };

            // we assume identical filters for DX11/DX12
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_BRIGHTNESS         == D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS, "11 and 12 filter flags should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_CONTRAST           == D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST, "11 and 12 filter flags should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_HUE                == D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE, "11 and 12 filter flags should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_SATURATION         == D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION, "11 and 12 filter flags should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_NOISE_REDUCTION    == D3D12_VIDEO_PROCESS_FILTER_FLAG_NOISE_REDUCTION, "11 and 12 filter flags should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_EDGE_ENHANCEMENT   == D3D12_VIDEO_PROCESS_FILTER_FLAG_EDGE_ENHANCEMENT, "11 and 12 filter flags should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_ANAMORPHIC_SCALING == D3D12_VIDEO_PROCESS_FILTER_FLAG_ANAMORPHIC_SCALING, "11 and 12 filter flags should match");
            static_assert(D3D11_1DDI_VIDEO_PROCESSOR_FILTER_CAPS_STEREO_ADJUSTMENT  == D3D12_VIDEO_PROCESS_FILTER_FLAG_STEREO_ADJUSTMENT, "11 and 12 filter flags should match");

            static constexpr UINT mapAutoProcessing[] =
            {
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_DENOISE,              // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_DENOISE
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_DERINGING,            // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_DERINGING
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_EDGE_ENHANCEMENT,     // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_EDGE_ENHANCEMENT
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_COLOR_CORRECTION,     // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_COLOR_CORRECTION
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_FLESH_TONE_MAPPING,   // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_FLESH_TONE_MAPPING
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_IMAGE_STABILIZATION,  // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_IMAGE_STABILIZATION
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_SUPER_RESOLUTION,     // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_SUPER_RESOLUTION
                D3D11_1DDI_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_ANAMORPHIC_SCALING,   // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_ANAMORPHIC_SCALING
                0,                                                                // D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_CUSTOM
            };

            dx11Caps.FeatureCaps |= MapFlags(mapFeature, (UINT)dx12Support.FeatureSupport);

            if (!CD3D11FormatHelper::YUV(dx12Support.InputSample.Format.Format))
            {
                if (dx12Support.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY)
                {
                    dx11Caps.InputFormatCaps |= D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_CAPS_RGB_LUMA_KEY;
                }
                if (dx12Support.FilterSupport & (D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS | D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST | D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE | D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION))
                {
                    // Note: the below should be true, but we have several outputs indicating no RGB_PROCAMP even if the condition above is true. So, ignoring RGB_PROCAMP.
                    // dx11Caps.InputFormatCaps |= D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_CAPS_RGB_PROCAMP;
                }
                if (dx12Support.InputFieldType != D3D12_VIDEO_FIELD_TYPE_NONE)
                {
                    dx11Caps.InputFormatCaps |= D3D11_1DDI_VIDEO_PROCESSOR_FORMAT_CAPS_RGB_INTERLACED;
                }
            }
            dx11Caps.AutoStreamCaps |= MapFlags(mapAutoProcessing, (UINT)dx12Support.AutoProcessingSupport);

            // filter support
            UINT origFilterCaps = dx11Caps.FilterCaps;
            dx11Caps.FilterCaps |= (UINT)dx12Support.FilterSupport;
            UINT newMask = dx11Caps.FilterCaps & (~origFilterCaps);
            UINT filter = 0;
            while (newMask)
            {
                assert(filter < cFilterRanges);
                if (newMask & 0x1)
                {
                    memcpy(&pFilterRanges[filter], &dx12Support.FilterRangeSupport[filter], sizeof(dx12Support.FilterRangeSupport[filter]));
                }
                newMask >>= 1;
                ++filter;
            }

            // rate conversion caps
            assert((dx12Support.DeinterlaceSupport & ~(D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM | D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB)) == 0);

            if (dx12Support.DeinterlaceSupport & D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB)
            {
                if (dx12Support.DeinterlaceSupport & D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM)
                {
                    if (dx11RateConversionCaps.size() < 2)
                    {
                        dx11RateConversionCaps.resize(2);
                        // Custom isn't required to be the same as adaptive, but often is in practice. 
                        // Reporting this value indicates that a "better than BOB" method is available, which apps tend to select.
                        dx11RateConversionCaps[1].ConversionCaps |= D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_DEINTERLACE_ADAPTIVE;
                    }
                }

                for (auto &caps : dx11RateConversionCaps)
                {
                    caps.ConversionCaps |= D3D11_1DDI_VIDEO_PROCESSOR_CONVERSION_CAPS_DEINTERLACE_BOB;
                }
            }

            // color conversion caps
            dx11Caps.DeviceCaps |= ToDX11VideoProcessorDeviceCaps(colorConversionCap);
        }

        static inline void ProtectedSessionStatus(_In_ D3D12_PROTECTED_SESSION_STATUS d3d12Status, _Out_ D3DWDDM2_0DDI_CRYPTO_SESSION_STATUS *pD3D11Status)
        {
            switch (d3d12Status)
            {
            case D3D12_PROTECTED_SESSION_STATUS_OK:
                *pD3D11Status = D3DWDDM2_0DDI_CRYPTO_SESSION_STATUS_OK;
                break;

            case D3D12_PROTECTED_SESSION_STATUS_INVALID:
                *pD3D11Status = D3DWDDM2_0DDI_CRYPTO_SESSION_STATUS_KEY_AND_CONTENT_LOST;
                break;

            default:
                *pD3D11Status = D3DWDDM2_0DDI_CRYPTO_SESSION_STATUS_KEY_AND_CONTENT_LOST;
                break;
            }
        }
   };
}