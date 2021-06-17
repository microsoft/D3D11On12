//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       d3dx11on12.h
//  Content:    D3DX12 utility library
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __D3DX11ON12_H__
#define __D3DX11ON12_H__

#include "d3d11.h"
#include "d3dx12.h"

#if defined( __cplusplus )


struct CD3DX11ON12_HEAP_PROPERTIES_FROM11 : public CD3DX12_HEAP_PROPERTIES
{
    CD3DX11ON12_HEAP_PROPERTIES_FROM11() : CD3DX12_HEAP_PROPERTIES() { }
    static CD3DX12_HEAP_PROPERTIES ConvertD3D11Usage( D3D11_USAGE Usage, UINT CPUAccessFlags )
    {
        switch (Usage)
        {
        case D3D11_USAGE_DEFAULT:
            if (CPUAccessFlags)
            {
                return CD3DX12_HEAP_PROPERTIES(
                    CPUAccessFlags == D3D11_CPU_ACCESS_WRITE ? D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE : D3D12_CPU_PAGE_PROPERTY_WRITE_BACK
                    , D3D12_MEMORY_POOL_L0
                    );
            }
            else
            {
                return CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
            }
        case D3D11_USAGE_IMMUTABLE:
            return CD3DX12_HEAP_PROPERTIES( CPUAccessFlags ? D3D12_HEAP_TYPE( 0 ) : D3D12_HEAP_TYPE_DEFAULT );
        case D3D11_USAGE_DYNAMIC:
            {
                if((CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) != 0)
                    return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

                // DJB FIXME: The following is inaccurate but DWM seems to call
                // to create a dynamic buffer for reading in one of the first
                // few buffers it creates
                if ((CPUAccessFlags & D3D11_CPU_ACCESS_READ) != 0)
                    return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

                return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE(0));
            }
        case D3D11_USAGE_STAGING:
            return CD3DX12_HEAP_PROPERTIES( CPUAccessFlags == D3D11_CPU_ACCESS_WRITE
                ? D3D12_HEAP_TYPE_UPLOAD
                : (CPUAccessFlags ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE( 0 ))
                );
        default: return CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE( 0 ) );
        }
    }
    explicit CD3DX11ON12_HEAP_PROPERTIES_FROM11( const D3D11_BUFFER_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_PROPERTIES*>(this) = ConvertD3D11Usage( o.Usage, o.CPUAccessFlags );
    }
    explicit CD3DX11ON12_HEAP_PROPERTIES_FROM11( const D3D11_TEXTURE1D_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_PROPERTIES*>(this) = ConvertD3D11Usage( o.Usage, o.CPUAccessFlags );
    }
    explicit CD3DX11ON12_HEAP_PROPERTIES_FROM11( const D3D11_TEXTURE2D_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_PROPERTIES*>(this) = ConvertD3D11Usage( o.Usage, o.CPUAccessFlags );
    }
    explicit CD3DX11ON12_HEAP_PROPERTIES_FROM11( const D3D11_TEXTURE3D_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_PROPERTIES*>(this) = ConvertD3D11Usage( o.Usage, o.CPUAccessFlags );
    }
};
struct CD3DX11ON12_HEAP_DESC_FROM11 : public CD3DX12_HEAP_DESC
{
    CD3DX11ON12_HEAP_DESC_FROM11() : CD3DX12_HEAP_DESC() { }
    static D3D12_HEAP_FLAGS ConvertD3D11MiscFlags( UINT miscFlags )
    {
        D3D12_HEAP_FLAGS Flags = D3D12_HEAP_FLAG_NONE;
        if ((miscFlags & (D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED)) != 0)
        {
            Flags |= D3D12_HEAP_FLAG_SHARED;
        }
        return Flags;
    }
    explicit CD3DX11ON12_HEAP_DESC_FROM11( const D3D11_BUFFER_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_DESC*>(this) = CD3DX12_HEAP_DESC( 0, CD3DX11ON12_HEAP_PROPERTIES_FROM11( o ), 0, ConvertD3D11MiscFlags( o.MiscFlags ) );
        if (o.MiscFlags & D3D11_RESOURCE_MISC_TILE_POOL) { SizeInBytes = o.ByteWidth; }
    }
    explicit CD3DX11ON12_HEAP_DESC_FROM11( const D3D11_TEXTURE1D_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_DESC*>(this) = CD3DX12_HEAP_DESC( 0, CD3DX11ON12_HEAP_PROPERTIES_FROM11( o ), 0, ConvertD3D11MiscFlags( o.MiscFlags ) );
    }
    explicit CD3DX11ON12_HEAP_DESC_FROM11( const D3D11_TEXTURE2D_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_DESC*>(this) = CD3DX12_HEAP_DESC( 0, CD3DX11ON12_HEAP_PROPERTIES_FROM11( o ), 0, ConvertD3D11MiscFlags( o.MiscFlags ) );
    }
    explicit CD3DX11ON12_HEAP_DESC_FROM11( const D3D11_TEXTURE3D_DESC& o )
    {
        *static_cast<CD3DX12_HEAP_DESC*>(this) = CD3DX12_HEAP_DESC( 0, CD3DX11ON12_HEAP_PROPERTIES_FROM11( o ), 0, ConvertD3D11MiscFlags( o.MiscFlags ) );
    }
};
struct CD3DX11ON12_RESOURCE_DESC_FROM11 : public CD3DX12_RESOURCE_DESC
{
    CD3DX11ON12_RESOURCE_DESC_FROM11() : CD3DX12_RESOURCE_DESC() { }
    static D3D12_RESOURCE_FLAGS ConvertD3D11Flags( UINT bindFlags )
    {
        D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
        if (!(bindFlags & D3D11_BIND_SHADER_RESOURCE) && (bindFlags & D3D11_BIND_DEPTH_STENCIL)) Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        if (bindFlags & D3D11_BIND_RENDER_TARGET) Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        if (bindFlags & D3D11_BIND_DEPTH_STENCIL) Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (bindFlags & D3D11_BIND_UNORDERED_ACCESS) Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        return Flags;
    }
    explicit CD3DX11ON12_RESOURCE_DESC_FROM11( const D3D11_BUFFER_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC*>(this) = Buffer( o.ByteWidth, D3D12_RESOURCE_FLAG_NONE );
        Flags |= ConvertD3D11Flags( o.BindFlags );
    }
    explicit CD3DX11ON12_RESOURCE_DESC_FROM11( const D3D11_TEXTURE1D_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC*>(this) = Tex1D( o.Format, o.Width, UINT16( o.ArraySize ), UINT16( o.MipLevels ) );
        Flags |= ConvertD3D11Flags( o.BindFlags );
        if (o.MiscFlags & D3D11_RESOURCE_MISC_TILED) Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    }
    explicit CD3DX11ON12_RESOURCE_DESC_FROM11( const D3D11_TEXTURE2D_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC*>(this) = Tex2D( o.Format, o.Width, o.Height, UINT16( o.ArraySize ), UINT16( o.MipLevels ), 
            o.SampleDesc.Count, o.SampleDesc.Quality );
        Flags |= ConvertD3D11Flags( o.BindFlags );
        if (o.MiscFlags & D3D11_RESOURCE_MISC_TILED) Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    }
    explicit CD3DX11ON12_RESOURCE_DESC_FROM11( const D3D11_TEXTURE3D_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC*>(this) = Tex3D( o.Format, o.Width, o.Height, UINT16( o.Depth ), UINT16( o.MipLevels ) );
        Flags |= ConvertD3D11Flags( o.BindFlags );
        if (o.MiscFlags & D3D11_RESOURCE_MISC_TILED) Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    }
};

struct CD3DX11ON12_RESOURCE_DESC1_FROM11 : public CD3DX12_RESOURCE_DESC1
{
    CD3DX11ON12_RESOURCE_DESC1_FROM11() : CD3DX12_RESOURCE_DESC1() { }
    static D3D12_RESOURCE_FLAGS ConvertD3D11Flags( UINT bindFlags )
    {
        D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
        if (!(bindFlags & D3D11_BIND_SHADER_RESOURCE) && (bindFlags & D3D11_BIND_DEPTH_STENCIL)) Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        if (bindFlags & D3D11_BIND_RENDER_TARGET) Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        if (bindFlags & D3D11_BIND_DEPTH_STENCIL) Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (bindFlags & D3D11_BIND_UNORDERED_ACCESS) Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        return Flags;
    }
    explicit CD3DX11ON12_RESOURCE_DESC1_FROM11( const D3D11_BUFFER_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC1*>(this) = Buffer( o.ByteWidth, D3D12_RESOURCE_FLAG_NONE );
        Flags |= ConvertD3D11Flags( o.BindFlags );
    }
    explicit CD3DX11ON12_RESOURCE_DESC1_FROM11( const D3D11_TEXTURE1D_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC1*>(this) = Tex1D( o.Format, o.Width, UINT16( o.ArraySize ), UINT16( o.MipLevels ) );
        Flags |= ConvertD3D11Flags( o.BindFlags );
        if (o.MiscFlags & D3D11_RESOURCE_MISC_TILED) Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    }
    explicit CD3DX11ON12_RESOURCE_DESC1_FROM11( const D3D11_TEXTURE2D_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC1*>(this) = Tex2D( o.Format, o.Width, o.Height, UINT16( o.ArraySize ), UINT16( o.MipLevels ), 
            o.SampleDesc.Count, o.SampleDesc.Quality );
        Flags |= ConvertD3D11Flags( o.BindFlags );
        if (o.MiscFlags & D3D11_RESOURCE_MISC_TILED) Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    }
    explicit CD3DX11ON12_RESOURCE_DESC1_FROM11( const D3D11_TEXTURE3D_DESC& o )
    {
        *static_cast<CD3DX12_RESOURCE_DESC1*>(this) = Tex3D( o.Format, o.Width, o.Height, UINT16( o.Depth ), UINT16( o.MipLevels ) );
        Flags |= ConvertD3D11Flags( o.BindFlags );
        if (o.MiscFlags & D3D11_RESOURCE_MISC_TILED) Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    }
};
#endif // defined( __cplusplus )

#endif //__D3DX11ON12_H__



