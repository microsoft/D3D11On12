// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
enum ResourceInfoType
{
    TiledPoolType,
    ResourceType
};

struct ResourceInfo
{
    union
    {
        struct
        {
            D3D12_HEAP_DESC m_HeapDesc;
        } TiledPool;

        struct {
            D3D12_RESOURCE_DESC m_ResourceDesc;
            D3D11_RESOURCE_FLAGS m_Flags11;
            D3D12_HEAP_FLAGS m_HeapFlags;
            D3D12_HEAP_PROPERTIES m_HeapProps;
        } Resource;
    };
    ResourceInfoType m_Type;
    bool m_bShared;
    bool m_bNTHandle;
    bool m_bSynchronized;
    HANDLE m_GDIHandle;
};

struct PrivateCallbacks
{
    D3D11_RESOURCE_FLAGS (CALLBACK *GetResourceFlags)(_In_ D3D10DDI_HRESOURCE, _Out_ bool *pbAcquireableOnWrite);
    bool (CALLBACK *NotifySharedResourceCreation)(_In_ HANDLE, _In_ IUnknown*);
};

constexpr UINT c_CurrentD3D11On12InterfaceVersion = 5;

struct SOpenAdapterArgs
{
    ID3D12Device1* pDevice;
    ID3D12CommandQueue* p3DCommandQueue;
    IUnknown* pAdapter;
    UINT NodeIndex;
    PrivateCallbacks Callbacks;
    bool bDisableGPUTimeout;

    // Velocity features
    bool bSupportDisplayableTextures;
    bool bSupportDeferredContexts;

    UINT D3D11On12InterfaceVersion = c_CurrentD3D11On12InterfaceVersion;
};

#define D3D11DDI_CREATEDEVICE_FLAG_IS_XBOX 0x80000000

struct SHADER_DESC
{
    const BYTE* pFunction;
    UINT SizeInBytes;
    ID3D11ClassLinkage* pLinkage;
};

struct GEOMETRY_SHADER_DESC
{
    const BYTE *pFunction;
    UINT SizeInBytes;
    const D3D11_SO_DECLARATION_ENTRY *pDeclaration;
    UINT NumElements;
    const UINT *pBufferStrides;
    UINT NumStrides;
    UINT RasterizedStream;
    ID3D11ClassLinkage* pLinkage;
};

enum class WrapReason
{
    CreateWrappedResource = 1
};
}

enum class SetStateReason
{
    Create,
    Acquire
};

interface ID3D11On12DDIResource
{
    static ID3D11On12DDIResource* CastFrom(D3D10DDI_HRESOURCE hResource) { return reinterpret_cast<ID3D11On12DDIResource*>(hResource.pDrvPrivate); }
    STDMETHOD_(ID3D12Resource*, GetUnderlyingResource)() = 0;
    STDMETHOD_(void, SetGraphicsCurrentState)(D3D12_RESOURCE_STATES State, SetStateReason Reason) = 0;
    STDMETHOD(CreateSharedHandle)(_In_opt_ const SECURITY_ATTRIBUTES *pAttributes, _In_ DWORD dwAccess, _In_opt_ LPCWSTR lpName, _Out_ HANDLE *pHandle) = 0;
};

interface ID3D11On12DDIFence
{
    static ID3D11On12DDIFence* CastFrom(HANDLE hFence) { return reinterpret_cast<ID3D11On12DDIFence*>(hFence); }
    STDMETHOD(CreateSharedHandle)(_In_opt_ const SECURITY_ATTRIBUTES *pAttributes, _In_ DWORD dwAccess, _In_opt_ LPCWSTR lpName, _Out_ HANDLE *pHandle) = 0;
    STDMETHOD(SetEventOnCompletion)(UINT64 Value, _In_ HANDLE hEvent) = 0;
    STDMETHOD_(UINT64, GetCompletedValue)() = 0;
    STDMETHOD_(void, Destroy)() = 0;
};

interface ID3D11On12DDIDevice
{
    static ID3D11On12DDIDevice* CastFrom(D3D10DDI_HDEVICE hDevice) { return reinterpret_cast<ID3D11On12DDIDevice*>(hDevice.pDrvPrivate); }
    STDMETHOD(GetD3D12Device)(REFIID riid, void** ppv) = 0;
    STDMETHOD(GetGraphicsQueue)(REFIID riid, void** ppv) = 0;
    STDMETHOD(EnqueueSetEvent)(_In_ HANDLE hEvent) = 0;
    STDMETHOD_(UINT, GetNodeMask)() = 0;
    STDMETHOD(Present)(_In_ D3DKMT_PRESENT* pArgs) = 0;

    // Resource wrapping helpers
    STDMETHOD_(UINT, GetResourcePrivateDataSize)() = 0;
    STDMETHOD(OpenSharedHandle)(_In_ HANDLE hSharedHandle, _Out_writes_bytes_(PrivateDriverDataSize) void* pPrivateDriverData, UINT PrivateDriverDataSize, _Out_ D3DKMT_HANDLE* hKMTHandle) = 0;
    STDMETHOD(CreateWrappingHandle)(_In_ IUnknown* pResource, D3D11On12::WrapReason reason, _Out_writes_bytes_(PrivateDriverDataSize) void* pPrivateDriverData, UINT PrivateDriverDataSize, _Out_ D3DKMT_HANDLE* hKMTHandle) = 0;
    STDMETHOD(FillResourceInfo)(D3DKMT_HANDLE hKMTHandle, _In_ D3D11_RESOURCE_FLAGS const* pFlagOverrides, _Out_ D3D11On12::ResourceInfo* pResourceInfo) = 0;
    STDMETHOD_(void, DestroyKMTHandle)(D3DKMT_HANDLE) = 0;

    // Interop helpers
    STDMETHOD_(void, TransitionResourceForRelease)(_In_ ID3D11On12DDIResource* pResource, D3D12_RESOURCE_STATES State) = 0;
    STDMETHOD_(void, ApplyAllResourceTransitions)() = 0;

    // Fence helpers
    STDMETHOD(CreateFence)(UINT64 InitialValue, UINT DXGIInternalFenceFlags, _COM_Outptr_ ID3D11On12DDIFence** ppFence) = 0;
    STDMETHOD(OpenFence)(HANDLE hSharedFence, _Out_ bool* pbMonitored, _COM_Outptr_ ID3D11On12DDIFence** ppFence) = 0;
    STDMETHOD(Wait)(_In_ ID3D11On12DDIFence* pFence, UINT64 Value) = 0;
    STDMETHOD(Signal)(_In_ ID3D11On12DDIFence* pFence, UINT64 Value) = 0;

    // Shader creates which take the full containers instead of driver bytecode
    STDMETHOD(CreateVertexShader)(D3D10DDI_HSHADER hShader, _In_ D3D11On12::SHADER_DESC const* pDesc) = 0;
    STDMETHOD(CreatePixelShader)(D3D10DDI_HSHADER hShader, _In_ D3D11On12::SHADER_DESC const* pDesc) = 0;
    STDMETHOD(CreateGeometryShader)(D3D10DDI_HSHADER hShader, _In_ D3D11On12::GEOMETRY_SHADER_DESC const* pDesc, _In_opt_ D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT const* pCGSWSOArgs) = 0;
    STDMETHOD(CreateHullShader)(D3D10DDI_HSHADER hShader, _In_ D3D11On12::SHADER_DESC const* pDesc) = 0;
    STDMETHOD(CreateDomainShader)(D3D10DDI_HSHADER hShader, _In_ D3D11On12::SHADER_DESC const* pDesc) = 0;
    STDMETHOD(CreateComputeShader)(D3D10DDI_HSHADER hShader, _In_ D3D11On12::SHADER_DESC const* pDesc) = 0;

    STDMETHOD_(void, WriteToSubresource)(D3D10DDI_HRESOURCE hDstResource, UINT DstSubresource, _In_opt_ const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) = 0;
    STDMETHOD_(void, ReadFromSubresource)(void* pDstData, UINT DstRowPitch, UINT DstDepthPitch, D3D10DDI_HRESOURCE hSrcResource, UINT SrcSubresource, _In_opt_ const D3D11_BOX* pSrcBox) = 0;

    // Entries beyond this point only valid when D3D11On12InterfaceVersion >= 1
    // Called in response to SetPrivateData(WKPDID_DXAnalysisPresentBoundary) on a resource or fence
    STDMETHOD_(void, SharingContractPresent)(_In_ ID3D11On12DDIResource* pResource) = 0;

    // Entries beyond this point only valid when D3D11On12InterfaceVersion >= 2
    STDMETHOD_(void, SetMarker)(_In_opt_z_ const wchar_t* name) = 0;
    STDMETHOD_(void, BeginEvent)(_In_opt_z_ const wchar_t* name) = 0;
    STDMETHOD_(void, EndEvent)() = 0;

    // Entries beyond this point only valid when D3D11On12InterfaceVersion >= 3
    STDMETHOD(AddResourceWaitsToQueue)(_In_ ID3D11On12DDIResource* pResource, _In_ ID3D12CommandQueue* pCommmandQueue) = 0;
    STDMETHOD(AddDeferredWaitsToResource)(_In_ ID3D11On12DDIResource* pResource, UINT NumSync, _In_reads_(NumSync) UINT64* pSignalValues, _In_reads_(NumSync) ID3D12Fence** ppFences) = 0;

    // Entries beyond this point only valid when D3D11On12InterfaceVersion >= 4
    STDMETHOD_(void, SetMaximumFrameLatency)(UINT MaxFrameLatency) = 0;
    STDMETHOD_(bool, IsMaximumFrameLatencyReached)() = 0;
};

extern "C" HRESULT WINAPI OpenAdapter_D3D11On12(_Inout_ D3D10DDIARG_OPENADAPTER* pArgs, _Inout_ D3D11On12::SOpenAdapterArgs* pArgs2);
typedef HRESULT(APIENTRY* PFND3D11ON12_OPENADAPTER)(_Inout_ D3D10DDIARG_OPENADAPTER* pArgs, _Inout_ D3D11On12::SOpenAdapterArgs* pArgs2);

