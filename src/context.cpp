// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.hpp"

namespace D3D11On12
{
    BOOL APIENTRY Device::Flush(D3D10DDI_HDEVICE hDevice, UINT ContextType, UINT FlushFlags)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        Device *pDevice = Device::CastFrom(hDevice);

        if (FlushFlags & D3DWDDM1_3DDI_TRIM_MEMORY)
        {
            // TODO: Trim
        }

        BOOL flushResult = pDevice->GetBatchedContext().Flush(CommandListTypeMask(ContextType));

#pragma warning( suppress : 4127 ) // conditional expression is constant
        D3D11on12_DDI_ENTRYPOINT_END_REPORT_HR_AND_RETURN_VALUE(hDevice, S_OK, flushResult);
    }

    void APIENTRY DeviceBase::SetViewports(D3D10DDI_HDEVICE hDevice, UINT NumViewports, UINT ClearViewports, const D3D10_DDI_VIEWPORT* pViewports)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        UNREFERENCED_PARAMETER(ClearViewports);
        pDevice->GetBatchedContext().SetViewports(NumViewports, reinterpret_cast<const D3D12_VIEWPORT*>(pViewports));
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::SetScissorRects(D3D10DDI_HDEVICE hDevice, UINT NumRects, UINT ClearRects, const D3D10_DDI_RECT* pRects)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        UNREFERENCED_PARAMETER(ClearRects);
        pDevice->GetBatchedContext().SetScissorRects(NumRects, pRects);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceBase::ClearView(D3D10DDI_HDEVICE hDevice, D3D11DDI_HANDLETYPE HandleType, VOID* hView, const FLOAT Color[4], const D3D10_DDI_RECT* pRects, UINT NumRects)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        DXGI_FORMAT ViewFormat;

        C_ASSERT(sizeof(D3D12_RECT) == sizeof(D3D10_DDI_RECT));
        const D3D12_RECT *pRects12 = reinterpret_cast<const D3D12_RECT*>(pRects);
        switch (HandleType)
        {
        case D3D10DDI_HT_RENDERTARGETVIEW:
        {
            auto pView = RenderTargetView::CastToTranslationView(MAKE_D3D10DDI_HRENDERTARGETVIEW(hView));
            ViewFormat = pView->GetDesc12().Format;
            break;
        }
        case D3D10DDI_HT_DEPTHSTENCILVIEW:
        {
            D3D12TranslationLayer::DSV *pDSV = DepthStencilView::CastToTranslationView(MAKE_D3D10DDI_HDEPTHSTENCILVIEW(hView));
            // ClearView can only be called on resources that do not have stencil, but the runtime does not validate this
            if (pDSV->m_pResource->SubresourceMultiplier() > 1)
            {
                return;
            }
            ViewFormat = pDSV->GetDesc12().Format;
            break;
        }
        case D3D11DDI_HT_UNORDEREDACCESSVIEW:
        {
            D3D12TranslationLayer::UAV *pUAV = UnorderedAccessView::CastToTranslationView(MAKE_D3D11DDI_HUNORDEREDACCESSVIEW(hView));
            ViewFormat = pUAV->GetDesc12().Format;
            break;
        }
        case D3D11_1DDI_HT_VIDEODECODEROUTPUTVIEW:
        {
            D3D12TranslationLayer::VDOV *pVDOV = VideoDecoderOutputView::CastToTranslationView(MAKE_D3D11_1DDI_HVIDEODECODEROUTPUTVIEW(hView));
            ViewFormat = pVDOV->GetDesc12().Format;
            break;
        }
        case D3D11_1DDI_HT_VIDEOPROCESSORINPUTVIEW:
        {
            D3D12TranslationLayer::VPIV *pVPIV = VideoProcessorInputView::CastToTranslationView(MAKE_D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW(hView));
            ViewFormat = pVPIV->GetDesc12().Format;
            break;
        }
        case D3D11_1DDI_HT_VIDEOPROCESSOROUTPUTVIEW:
        {
            D3D12TranslationLayer::VPOV *pVPOV = VideoProcessorOutputView::CastToTranslationView(MAKE_D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW(hView));
            ViewFormat = pVPOV->GetDesc12().Format;
            break;
        }
        default:
            ASSUME(false);
        }

        switch (HandleType)
        {
        case D3D10DDI_HT_RENDERTARGETVIEW:
        {
            auto pView = RenderTargetView::CastToTranslationView(MAKE_D3D10DDI_HRENDERTARGETVIEW(hView));
            pDevice->GetBatchedContext().ClearRenderTargetView(pView, Color, NumRects, pRects12);
            break;
        }
        case D3D10DDI_HT_DEPTHSTENCILVIEW:
        {
            D3D12TranslationLayer::DSV *pDSV = DepthStencilView::CastToTranslationView(MAKE_D3D10DDI_HDEPTHSTENCILVIEW(hView));
            pDevice->GetBatchedContext().ClearDepthStencilView(pDSV, D3D12_CLEAR_FLAG_DEPTH, Color[0], 0, NumRects, pRects12);
            break;
        }
        case D3D11DDI_HT_UNORDEREDACCESSVIEW:
        {
            D3D12TranslationLayer::UAV *pUAV = UnorderedAccessView::CastToTranslationView(MAKE_D3D11DDI_HUNORDEREDACCESSVIEW(hView));
            D3D11_FORMAT_COMPONENT_INTERPRETATION Interpretation = CD3D11FormatHelper::GetFormatComponentInterpretation(ViewFormat, 0);
            if (Interpretation == D3D11FCI_UINT || Interpretation == D3D11FCI_SINT)
            {
                // According to the D3D11 spec, ClearView needs to round down when used on integer formats
                // Code copied from Warp's ClearViewCommon and slightly modified
                UINT aUINTs[4];
                for (UINT i = 0; i < 4; ++i)
                {
                    UINT bits = CD3D11FormatHelper::GetBitsPerComponent(ViewFormat, i);
                    if (Interpretation == D3D11FCI_SINT)
                    {
                        int tmp = Float32ToInt32Floor(Color[i]);
                        if (bits < 32)
                        {
                            const INT32 Scale = (1 << (bits - 1));
                            tmp = max(-Scale, min(Scale - 1, tmp));
                        }
                        aUINTs[i] = tmp;
                    }
                    else
                    {
                        aUINTs[i] = Float32ToUInt32Floor(Color[i]);
                        if (bits < 32)
                        {
                            const UINT32 Scale = (1 << bits);
                            aUINTs[i] = min(aUINTs[i], Scale - 1);
                        }
                    }
                }
                pDevice->GetBatchedContext().ClearUnorderedAccessViewUint(pUAV, aUINTs, NumRects, reinterpret_cast<const D3D12_RECT*>(pRects));
            }
            else
            {
                pDevice->GetBatchedContext().ClearUnorderedAccessViewFloat(pUAV, Color, NumRects, reinterpret_cast<const D3D12_RECT*>(pRects));
            }
            break;
        }
        case D3D11_1DDI_HT_VIDEODECODEROUTPUTVIEW:
        {
            D3D12TranslationLayer::VDOV *pVDOV = VideoDecoderOutputView::CastToTranslationView(MAKE_D3D11_1DDI_HVIDEODECODEROUTPUTVIEW(hView));
            pDevice->GetBatchedContext().ClearVideoDecoderOutputView(pVDOV, Color, NumRects, pRects12);
            break;
        }
        case D3D11_1DDI_HT_VIDEOPROCESSORINPUTVIEW:
        {
            D3D12TranslationLayer::VPIV *pVPIV = VideoProcessorInputView::CastToTranslationView(MAKE_D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW(hView));
            pDevice->GetBatchedContext().ClearVideoProcessorInputView(pVPIV, Color, NumRects, pRects12);
            break;
        }
        case D3D11_1DDI_HT_VIDEOPROCESSOROUTPUTVIEW:
        {
            D3D12TranslationLayer::VPOV *pVPOV = VideoProcessorOutputView::CastToTranslationView(MAKE_D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW(hView));
            pDevice->GetBatchedContext().ClearVideoProcessorOutputView(pVPOV, Color, NumRects, pRects12);
            break;
        }

        default:
            ASSUME(false);
        }

        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::ClearView(D3D10DDI_HDEVICE hDevice, D3D11DDI_HANDLETYPE HandleType, VOID* hView, const FLOAT Color[4], const D3D10_DDI_RECT* pRects, UINT NumRects)
    {
        DeviceBase::ClearView(hDevice, HandleType, GetImmediate(MAKE_D3D10DDI_HRENDERTARGETVIEW(hView)).pDrvPrivate, Color, pRects, NumRects);
    }

    void APIENTRY DeviceBase::ClearRenderTargetView(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRENDERTARGETVIEW hView, FLOAT Color[4])
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        D3D12TranslationLayer::RTV *pRTV = RenderTargetView::CastToTranslationView(hView);

        pDevice->GetBatchedContext().ClearRenderTargetView(pRTV, Color, 0, nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void DeviceDeferred::ClearRenderTargetView(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRENDERTARGETVIEW hView, FLOAT Color[4])
    {
        DeviceBase::ClearRenderTargetView(hDevice, GetImmediate(hView), Color);
    }

    void APIENTRY DeviceBase::ClearDepthStencilView(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILVIEW hView, UINT ClearFlags, FLOAT DepthValue, UINT8 StencilValue)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        D3D12TranslationLayer::DSV *pDSV = DepthStencilView::CastToTranslationView(hView);

        pDevice->GetBatchedContext().ClearDepthStencilView(pDSV, ClearFlags, DepthValue, StencilValue, 0, nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::ClearDepthStencilView(D3D10DDI_HDEVICE hDevice, D3D10DDI_HDEPTHSTENCILVIEW hView, UINT ClearFlags, FLOAT DepthValue, UINT8 StencilValue)
    {
        DeviceBase::ClearDepthStencilView(hDevice, GetImmediate(hView), ClearFlags, DepthValue, StencilValue);
    }

    void APIENTRY DeviceBase::ClearUnorderedAccessViewUint(D3D10DDI_HDEVICE hDevice, D3D11DDI_HUNORDEREDACCESSVIEW hView, CONST UINT Color[4])
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        D3D12TranslationLayer::UAV *pUAV = UnorderedAccessView::CastToTranslationView(hView);

        pDevice->GetBatchedContext().ClearUnorderedAccessViewUint(pUAV, Color, 0, nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::ClearUnorderedAccessViewUint(D3D10DDI_HDEVICE hDevice, D3D11DDI_HUNORDEREDACCESSVIEW hView, CONST UINT Color[4])
    {
        DeviceBase::ClearUnorderedAccessViewUint(hDevice, GetImmediate(hView), Color);
    }

    void APIENTRY DeviceBase::ClearUnorderedAccessViewFloat(D3D10DDI_HDEVICE hDevice, D3D11DDI_HUNORDEREDACCESSVIEW hView, CONST FLOAT Color[4])
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        D3D12TranslationLayer::UAV *pUAV = UnorderedAccessView::CastToTranslationView(hView);

        pDevice->GetBatchedContext().ClearUnorderedAccessViewFloat(pUAV, Color, 0, nullptr);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::ClearUnorderedAccessViewFloat(D3D10DDI_HDEVICE hDevice, D3D11DDI_HUNORDEREDACCESSVIEW hView, CONST FLOAT Color[4])
    {
        DeviceBase::ClearUnorderedAccessViewFloat(hDevice, GetImmediate(hView), Color);
    }

    void APIENTRY DeviceBase::Discard(D3D10DDI_HDEVICE hDevice, D3D11DDI_HANDLETYPE HandleType, VOID* ResourceOrView, const D3D10_DDI_RECT* pRects, UINT NumRects)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);

        if (HandleType == D3D10DDI_HT_RESOURCE)
        {
            pDevice->GetBatchedContext().DiscardResource(Resource::CastFromAndGetImmediateResource(MAKE_D3D10DDI_HRESOURCE(ResourceOrView)), pRects, NumRects);
        }
        else
        {
            D3D12TranslationLayer::ViewBase* pView;
            switch (HandleType)
            {
            case D3D10DDI_HT_SHADERRESOURCEVIEW:
                pView = ShaderResourceView::CastToTranslationView(MAKE_D3D10DDI_HSHADERRESOURCEVIEW(ResourceOrView));
                break;
            case D3D10DDI_HT_DEPTHSTENCILVIEW:
                pView = DepthStencilView::CastToTranslationView(MAKE_D3D10DDI_HDEPTHSTENCILVIEW(ResourceOrView));
                break;
            case D3D10DDI_HT_RENDERTARGETVIEW:
                pView = RenderTargetView::CastToTranslationView(MAKE_D3D10DDI_HRENDERTARGETVIEW(ResourceOrView));
                break;
            case D3D11DDI_HT_UNORDEREDACCESSVIEW:
                pView = UnorderedAccessView::CastToTranslationView(MAKE_D3D11DDI_HUNORDEREDACCESSVIEW(ResourceOrView));
                break;
            default:
                ASSUME(false);
                break;
            }
            pDevice->GetBatchedContext().DiscardView(pView, pRects, NumRects);
        }
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::Discard(D3D10DDI_HDEVICE hDevice, D3D11DDI_HANDLETYPE HandleType, VOID* ResourceOrView, const D3D10_DDI_RECT* pRects, UINT NumRects)
    {
        if (HandleType == D3D10DDI_HT_RESOURCE)
        {
            DeviceBase::Discard(hDevice, HandleType, ResourceOrView, pRects, NumRects);
        }
        else
        {
            DeviceBase::Discard(hDevice, HandleType, GetImmediate(MAKE_D3D10DDI_HRENDERTARGETVIEW(ResourceOrView)).pDrvPrivate, pRects, NumRects);
        }
    }

    void APIENTRY DeviceBase::GenMips(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADERRESOURCEVIEW hView)
    {
        D3D11on12_DDI_ENTRYPOINT_START();
        auto pDevice = CastFrom(hDevice);
        D3D12TranslationLayer::SRV *pSRV = ShaderResourceView::CastToTranslationView(hView);

        pDevice->GetBatchedContext().GenMips(pSRV, D3D12_FILTER_TYPE_LINEAR);
        D3D11on12_DDI_ENTRYPOINT_END_AND_REPORT_HR(hDevice, S_OK);
    }

    void APIENTRY DeviceDeferred::GenMips(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADERRESOURCEVIEW hView)
    {
        DeviceBase::GenMips(hDevice, GetImmediate(hView));
    }
}