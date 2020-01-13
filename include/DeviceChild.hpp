// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
namespace D3D11On12
{
    class Device;

    class DeviceChild
    {
    public:
        DeviceChild(Device& device) : m_parentDevice(device)
        {}

    protected:
        Device& m_parentDevice;
    };

    template <typename THandle>
    struct DeviceChildDeferred
    {
        THandle m_ImmediateHandle;
    };

    template <typename TFunc>
    struct InitializeDeferredHandle;
    template <typename TArgs, typename THandle, typename TRTHandle>
    struct InitializeDeferredHandle<void (APIENTRY*)(D3D10DDI_HDEVICE, TArgs, THandle, TRTHandle)>
    {
        static void Initialize(D3D10DDI_HDEVICE, TArgs, THandle handle, TRTHandle rtHandle)
        {
            reinterpret_cast<DeviceChildDeferred<THandle>*>(handle.pDrvPrivate)->m_ImmediateHandle = THandle{ rtHandle.handle };
        }
        static void Destroy(D3D10DDI_HDEVICE, THandle) { }
    };
    template <typename THandle, typename TRTHandle>
    struct InitializeDeferredHandle<void (APIENTRY*)(D3D10DDI_HDEVICE, THandle, TRTHandle)>
    {
        static void Initialize(D3D10DDI_HDEVICE, THandle handle, TRTHandle rtHandle)
        {
            reinterpret_cast<DeviceChildDeferred<THandle>*>(handle.pDrvPrivate)->m_ImmediateHandle = THandle{ rtHandle.handle };
        }
        static void Destroy(D3D10DDI_HDEVICE, THandle) { }
    };
    template <typename TFunc, typename TFunc2> void PopulateDeferredInitMethod(TFunc& init, TFunc2& destroy)
    {
        init = InitializeDeferredHandle<TFunc>::Initialize;
        destroy = InitializeDeferredHandle<TFunc>::Destroy;
    }

    template <typename... TArgs>
    struct InitializeDeferredHandle<void (APIENTRY*)(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, TArgs...)>
    {
        static void Initialize(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER handle, D3D10DDI_HRTSHADER rtHandle, TArgs...)
        {
            reinterpret_cast<DeviceChildDeferred<D3D10DDI_HSHADER>*>(handle.pDrvPrivate)->m_ImmediateHandle = D3D10DDI_HSHADER{ rtHandle.handle };
        }
    };
    template <typename TFunc> void PopulateDeferredShaderInit(TFunc& init)
    {
        init = InitializeDeferredHandle<TFunc>::Initialize;
    }

    template <typename THandle> THandle GetImmediate(THandle deferred)
    {
        return deferred.pDrvPrivate ?
            reinterpret_cast<DeviceChildDeferred<THandle>*>(deferred.pDrvPrivate)->m_ImmediateHandle :
            THandle{ nullptr };
    }
}