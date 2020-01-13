// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    class Resource : public ID3D11On12DDIResource, public DeviceChild
    {
    public:

        Resource(Device& parent, D3D12TranslationLayer::Resource* resource, bool ownsResource)
            : DeviceChild(parent),
            m_UnderlyingResource(parent.GetBatchedContext(), resource, ownsResource)
        {
        }

        static Resource* CastFrom(D3D10DDI_HRESOURCE hResource) noexcept { return reinterpret_cast<Resource*>(hResource.pDrvPrivate); }
        static Resource* CastFrom(DXGI_DDI_HRESOURCE hResource) noexcept { return CastFrom(MAKE_D3D10DDI_HRESOURCE(reinterpret_cast<void*>(hResource))); };

        static SIZE_T APIENTRY CalcPrivateResourceSize(D3D10DDI_HDEVICE, _In_ CONST D3D11DDIARG_CREATERESOURCE*);
        static SIZE_T APIENTRY CalcPrivateOpenedResourceSize(D3D10DDI_HDEVICE, _In_ CONST D3D10DDIARG_OPENRESOURCE*);
        static VOID APIENTRY CreateResource(D3D10DDI_HDEVICE, _In_ CONST D3D11DDIARG_CREATERESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE);
        static VOID APIENTRY CreateDeferredResource(D3D10DDI_HDEVICE, _In_ CONST D3D11DDIARG_CREATERESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE);
        static VOID APIENTRY OpenResource(D3D10DDI_HDEVICE, _In_ CONST D3D10DDIARG_OPENRESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE);
        static VOID APIENTRY DestroyResource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE);

        // ID3D11On12DDIResource
        STDMETHOD_(ID3D12Resource*, GetUnderlyingResource)() noexcept final;
        STDMETHOD_(void, SetGraphicsCurrentState)(D3D12_RESOURCE_STATES State, SetStateReason Reason) noexcept final;
        STDMETHOD(CreateSharedHandle)(_In_opt_ const SECURITY_ATTRIBUTES *pAttributes, _In_ DWORD dwAccess, _In_opt_ LPCWSTR lpName, _Out_ HANDLE *pHandle) noexcept final;

        static void ConvertUnderlyingTextureCreationDesc(D3D12TranslationLayer::ResourceCreationArgs& createArgs, Device &device,
            const D3D11DDIARG_CREATERESOURCE& RDescDDI11) noexcept
        {
            if (createArgs.m_heapDesc.Properties.Type == D3D12_HEAP_TYPE_CUSTOM)
            {
                switch (RDescDDI11.TextureLayout)
                {
                    case D3DWDDM2_0DDI_TL_STANDARD_SWIZZLE_64K_TILE:
                        createArgs.m_desc12.Layout = D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE;
                        break;
                    case D3DWDDM2_0DDI_TL_ROW_MAJOR:
                    {
                        if (createArgs.m_desc12.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
                        {
                            UINT64 totalSize = 0;
                            device.GetAdapter()->m_pUnderlyingDevice->GetCopyableFootprints(&createArgs.m_desc12, 0, createArgs.m_appDesc.Subresources(), 0, nullptr, nullptr, nullptr, &totalSize);
                            ASSUME(totalSize);

                            createArgs.m_desc12 = CD3DX12_RESOURCE_DESC::Buffer(totalSize, createArgs.m_desc12.Flags);
                            createArgs.m_isPlacedTexture = true;
                        }
                    } break;
                    case D3DWDDM2_0DDI_TL_UNDEFINED: break;
                    default: assert(false); break;
                }

                // This is normally injected by GetCustomHeapProperties,
                // but that isn't used if the heap type is already CUSTOM.
                createArgs.m_heapDesc.Properties.CreationNodeMask =
                    createArgs.m_heapDesc.Properties.VisibleNodeMask =
                    device.GetImmediateContextNoFlush().GetNodeMask();
            }
        }

        D3D12TranslationLayer::BatchedResource& BatchedResource() { return m_UnderlyingResource; }
        D3D12TranslationLayer::Resource* ImmediateResource() { return m_UnderlyingResource.m_pResource; }
        static D3D12TranslationLayer::Resource* CastFromAndGetImmediateResource(D3D10DDI_HRESOURCE hResource)
        {
            return hResource.pDrvPrivate ? CastFrom(hResource)->ImmediateResource() : nullptr;
        }
        static D3D12TranslationLayer::Resource* CastFromAndGetImmediateResource(DXGI_DDI_HRESOURCE hResource) { return CastFromAndGetImmediateResource(MAKE_D3D10DDI_HRESOURCE(reinterpret_cast<void*>(hResource))); }

        static void GatherImmediateResourcesFromHandles(const D3D10DDI_HRESOURCE* pHResource, D3D12TranslationLayer::Resource** pUnderlying, UINT count)
        {
            for (size_t i = 0; i < count; i++)
            {
                pUnderlying[i] = CastFromAndGetImmediateResource(pHResource[i]);
            }
        }

    protected:
        static UINT ModifyFlagsForVideoSupport(
            D3D12TranslationLayer::ImmediateContext& context,
            UINT BindFlags,
            DXGI_FORMAT Format
            );

    private:
        D3D12TranslationLayer::BatchedResource m_UnderlyingResource;

    public:
        UINT m_VidPnSourceId = D3DDDI_ID_UNINITIALIZED;
    };
};