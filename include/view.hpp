// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{

    template< class TIface >
    struct ViewTraits;

#define DECLARE_VIEW_TRAITS(View, AbbreviatedView, DDIDescType, HandleType) \
    template<> struct ViewTraits<ID3D11##View> \
    { \
    typedef D3D12TranslationLayer::##AbbreviatedView TranslationLayerView; \
    typedef HandleType ViewHandle; \
    typedef DDIDescType DDIDesc; \
    }

    DECLARE_VIEW_TRAITS(ShaderResourceView, SRV, D3DWDDM2_0DDIARG_CREATESHADERRESOURCEVIEW, D3D10DDI_HSHADERRESOURCEVIEW);
    DECLARE_VIEW_TRAITS(RenderTargetView, RTV, D3DWDDM2_0DDIARG_CREATERENDERTARGETVIEW, D3D10DDI_HRENDERTARGETVIEW);
    DECLARE_VIEW_TRAITS(DepthStencilView, DSV, D3D11DDIARG_CREATEDEPTHSTENCILVIEW, D3D10DDI_HDEPTHSTENCILVIEW);
    DECLARE_VIEW_TRAITS(UnorderedAccessView, UAV, D3DWDDM2_0DDIARG_CREATEUNORDEREDACCESSVIEW, D3D11DDI_HUNORDEREDACCESSVIEW);
    DECLARE_VIEW_TRAITS(VideoDecoderOutputView, VDOV, D3D11_1DDIARG_CREATEVIDEODECODEROUTPUTVIEW, D3D11_1DDI_HVIDEODECODEROUTPUTVIEW);
    DECLARE_VIEW_TRAITS(VideoProcessorInputView, VPIV, D3D11_1DDIARG_CREATEVIDEOPROCESSORINPUTVIEW, D3D11_1DDI_HVIDEOPROCESSORINPUTVIEW);
    DECLARE_VIEW_TRAITS(VideoProcessorOutputView, VPOV, D3D11_1DDIARG_CREATEVIDEOPROCESSOROUTPUTVIEW, D3D11_1DDI_HVIDEOPROCESSOROUTPUTVIEW);

    template<typename Traits>
    class View : public DeviceChild
    {
    public:
        View(Device &device, D3D12TranslationLayer::Resource &resource, typename Traits::DDIDesc const* pDesc);

        typename Traits::TranslationLayerView *GetTranslationLayerView() { return m_spView.get(); }
        static View *CastFrom(typename Traits::ViewHandle hView) { return static_cast< View* >(hView.pDrvPrivate); }
        static typename Traits::TranslationLayerView *CastToTranslationView(typename Traits::ViewHandle hView) {
            View *pView = CastFrom(hView);
            return (pView) ? pView->GetTranslationLayerView() : nullptr;
        }

        static void GatherViewsFromHandles(typename const Traits::ViewHandle* pHViews, typename Traits::TranslationLayerView ** pUnderlying, UINT count)
        {
            for (size_t i = 0; i < count; i++)
            {
                pUnderlying[i] = CastToTranslationView(pHViews[i]);
            }
        }
    private:
        D3D12TranslationLayer::unique_batched_ptr<typename Traits::TranslationLayerView> m_spView;
    };

    typedef View <ViewTraits <ID3D11RenderTargetView> > RenderTargetView;
    typedef View <ViewTraits <ID3D11DepthStencilView> > DepthStencilView;
    typedef View <ViewTraits <ID3D11UnorderedAccessView> > UnorderedAccessView;
    typedef View <ViewTraits <ID3D11ShaderResourceView> > ShaderResourceView;
    typedef View <ViewTraits <ID3D11VideoDecoderOutputView> > VideoDecoderOutputView;
    typedef View <ViewTraits <ID3D11VideoProcessorInputView> > VideoProcessorInputView;
    typedef View <ViewTraits <ID3D11VideoProcessorOutputView> > VideoProcessorOutputView;

};