// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

//==================================================================================================================================
//
// D3D11On12::Fence
//
//==================================================================================================================================

namespace D3D11On12
{
    class Fence : public ID3D11On12DDIFence, DeviceChild
    {
    public:
        Fence(Device& parent, std::shared_ptr<D3D12TranslationLayer::Fence> spFence)
            : DeviceChild(parent)
            , m_pUnderlyingFence(std::move(spFence))
        {
        }

        std::shared_ptr<D3D12TranslationLayer::Fence> const& GetUnderlying() const { return m_pUnderlyingFence; }

        // ID3D11On12DDIFence
        STDMETHOD(CreateSharedHandle)(_In_opt_ const SECURITY_ATTRIBUTES *pAttributes, _In_ DWORD dwAccess, _In_opt_ LPCWSTR lpName, _Out_ HANDLE *pHandle) noexcept final
        {
            return m_parentDevice.GetImmediateContextNoFlush().m_pDevice12->CreateSharedHandle(m_pUnderlyingFence->Get(), pAttributes, dwAccess, lpName, pHandle);
        }
        STDMETHOD(SetEventOnCompletion)(UINT64 Value, _In_ HANDLE hEvent) noexcept final
        {
            return m_pUnderlyingFence->Get()->SetEventOnCompletion(Value, hEvent);
        }
        STDMETHOD_(UINT64, GetCompletedValue)() noexcept final
        {
            return m_pUnderlyingFence->Get()->GetCompletedValue();
        }
        STDMETHOD_(void, Destroy)() noexcept final
        {
            delete this;
        }

    protected:
        // Since drivers don't have visibility into this object, the runtime ensures
        // destruction ordering.
        std::shared_ptr<D3D12TranslationLayer::Fence> m_pUnderlyingFence;
    };
}