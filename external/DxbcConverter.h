///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxbcConverter.h                                                           //
// Copyright (C) Microsoft. All rights reserved.                             //
//                                                                           //
// Provides declarations for the DirectX DXBC to DXIL converter component.   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __DXBC_CONVERTER__H__
#define __DXBC_CONVERTER__H__

struct __declspec(uuid("5F956ED5-78D1-4B15-8247-F7187614A041"))
IDxbcConverter : public IUnknown {
  /// Create DXIL container out of DXBC shader blob.
  virtual HRESULT STDMETHODCALLTYPE Convert(
    _In_reads_bytes_(DxbcSize) LPCVOID pDxbc,
    _In_ UINT32 DxbcSize,
    _In_opt_z_ LPCWSTR pExtraOptions,
    _Outptr_result_bytebuffer_maybenull_(*pDxilSize) LPVOID *ppDxil,
    _Out_ UINT32 *pDxilSize,
    _Outptr_result_maybenull_z_ LPWSTR *ppDiag) = 0;

  /// Create DXIL LLVM module out of DXBC bytecode and DDI I/O signatures.
  /// This is for driver consumption only.
  virtual HRESULT STDMETHODCALLTYPE ConvertInDriver(
    _In_reads_bytes_(pBytecode[1]) const UINT32 *pBytecode,
    _In_opt_z_ LPCVOID pInputSignature,
    _In_ UINT32 NumInputSignatureElements,
    _In_opt_z_ LPCVOID pOutputSignature,
    _In_ UINT32 NumOutputSignatureElements,
    _In_opt_z_ LPCVOID pPatchConstantSignature,
    _In_ UINT32 NumPatchConstantSignatureElements,
    _In_opt_z_ LPCWSTR pExtraOptions,
    _Out_ IDxcBlob **ppDxilModule,
    _Outptr_result_maybenull_z_ LPWSTR *ppDiag) = 0;
};

__declspec(selectany)
extern const CLSID CLSID_DxbcConverter = { /* 4900391E-B752-4EDD-A885-6FB76E25ADDB */
  0x4900391e,
  0xb752,
  0x4edd,
  { 0xa8, 0x85, 0x6f, 0xb7, 0x6e, 0x25, 0xad, 0xdb }
};

#endif
