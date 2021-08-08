// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <D3D12TranslationLayerDependencyIncludes.h>
#include <D3D12TranslationLayerIncludes.h>

#include <d3dx11on12.h>

#include "SharedResourceHelpers.hpp"

#ifdef WIN32_LEAN_AND_MEAN
// This defines NTSTATUS and other types that are needed for kernel headers
#include <wincrypt.h>
#endif

#include "d3dkmthk.h"
#include "dxgiddi.h"

#ifndef D3D12_TOKENIZED_PROGRAM_FORMAT_HEADER
#define D3D12_TOKENIZED_PROGRAM_FORMAT_HEADER
#endif
#include "d3d10umddi.h"

#include "D3D11On12DDI.h"

_At_(return, _When_(FAILED(hr), __analysis_noreturn))
inline void ThrowFailure(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw _com_error(hr);
    }
}

template< typename T >
constexpr bool IsPow2(T num)
{
    static_assert(!std::is_signed_v<T>, "Signed type passed to IsPow2");
    return !(num & (num - 1));
}
template< typename T >
constexpr T Align(T uValue, T uAlign)
{
    T uResult;

    if (IsPow2(uAlign))
    {
        T uMask = uAlign - 1;
        uResult = (uValue + uMask) & ~uMask;
    }
    else
    {
        uResult = ((uValue + uAlign - 1) / uAlign) * uAlign;
    }

    assert(uResult >= uValue);
    assert(0 == (uResult % uAlign));
    return uResult;
}
extern bool GetCompatValue(PCSTR, UINT64*);

TRACELOGGING_DECLARE_PROVIDER(g_hD3D11On12TraceLoggingProvider);

#define ASSERT assert

class CDevice;
#include "Util.hpp"
#include "DeviceChild.hpp"
#include "PipelineStateCache.hpp"
#include "Shader.hpp"
#include "InputLayout.hpp"
#include "State.hpp"
#include "PipelineState.hpp"
#include "Layer.hpp"
#include "Adapter.hpp"
#include "VideoTranslate.hpp"
#include "VideoDevice.hpp"
#include "VideoDecode.hpp"
#include "VideoProcess.hpp"
#include "VideoProcessEnum.hpp"
#include "Device.hpp"
#include "Resource.hpp"
#include "View.hpp"
#include "Query.hpp"
#include "Sampler.hpp"
#include "Fence.hpp"

#include "InputLayout.inl"
#include "View.inl"
#include "Shader.inl"

