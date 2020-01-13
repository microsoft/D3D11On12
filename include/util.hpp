// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D11On12
{
    template <typename T> using unique_comptr = D3D12TranslationLayer::unique_comptr<T>;

    //----------------------------------------------------------------------------------------------------------------------------------
    unsigned __int32 const IntNearEven_MinFloat = 0xcf000000;
    unsigned __int32 const IntNearEven_MaxFloat = 0x4effffff;
    inline INT32 Float32ToInt32Floor(float const & f)
    {
        if (f <= *(float*)&IntNearEven_MinFloat)
            return INT32(0x80000000);

        if (f >= *(float*)&IntNearEven_MaxFloat)
            return INT32(0x7FFFFFFF);

        if (f != f) //NaN
            return 0;

        INT32 i = (INT32)floorf(fabs(f));

        return f >= 0 ? i : -i;
    }

    inline UINT32 Float32ToUInt32Floor(float const &f)
    {
        INT32 i = *(INT32*)&f;

        // compare with 0.5f that has binary representation 0x3f000000
        if (i < INT32(0x3f000000))
        {
            // negative number, negative infinity, negative NaN or too small number
            return 0;
        }

        if (i >= 0x4f800000)
        {
            // too big number, positive infinity or NaN
            if (i > INT32(0x7f800000))
            {
                // positive NaN
                return 0;
            }
            else
            {
                // too big number or positive infinity, clamp it
                return 0xFFFFFFFF;
            }
        }

        UINT32 n = (i & 0x007FFFFF) | 0x00800000;
        INT32 biasedExp = i >> 23;
        INT32 shift = biasedExp - 0x7F - 23;
        UINT32 result;
        if (shift >= 0)
        {
            assert(shift <= 8);
            result = n << shift;
        }
        else
        {
            assert(shift >= -24);
            n >>= (-1 - shift);
            n++;
            result = n >> 1;
        }
        return float(result) > f ? result - 1 : result;
    }

    template<typename T>
    struct inline_destructor
    {
        void operator()(T *pT) const
        {
            pT->~T();
        }
    };
    template <typename T> using AutoCleanupPtr = std::unique_ptr<T, inline_destructor<T>>;

    // Calls the destructor and sets the ptr to null
    template<typename T>
    struct inline_destructor_and_clear
    {
        void operator()(T **ppT) const
        {
            (*ppT)->~T();
            (*ppT) = nullptr;
        }
    };
    template <typename T> using AutoCleanupAndClearPtr = std::unique_ptr<T*, inline_destructor_and_clear<T>>;

    static HRESULT SynchronizedResultToHRESULT(bool bSynchronized)
    {
        return bSynchronized ? S_OK : DXGI_DDI_ERR_WASSTILLDRAWING;
    }
}
