// Static.hpp
#pragma once

#include "..\..\Helper\Helper.hpp"

_MMAT_IMPL_START_
#define _DES_N_CLEANUP_ catch(...) { if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_data, i); throw; }

_MTEMPL_ inline constexpr void zero_construct(T *mem_ptr, size_t start_pos, size_t end_pos) {
    size_t i = start_pos;
    if constexpr (_CPY_CSTR_) {
        if (zero_exists || !_DFLT_CSTR_) {
            const T &zero_val = zero_exists ? _GET_ZERO_ : T{};
            if constexpr (_STD_ is_nothrow_copy_constructible_v<T>) _STD_ uninitialized_fill_n(mem_ptr + start_pos, end_pos, zero_val);
            else try { for (; i < end_pos; i++) _STD_ construct_at(mem_ptr + i, zero_val); } _DES_N_CLEANUP_
        }
        else goto DEFAULT_CONSTRUCT;
        return;
    }
    else goto DEFAULT_CONSTRUCT;
    DEFAULT_CONSTRUCT:
        if constexpr (_STD_ is_nothrow_default_constructible_v<T>) _STD_ uninitialized_value_construct_n(mem_ptr + start_pos, end_pos);
        else try { for (; i < end_pos; i++) _STD_ construct_at(mem_ptr + i); } _DES_N_CLEANUP_
}

_MTYPE_TEMPL(T  , ...Args) inline constexpr void variadic_construct(T *mem_ptr, size_t start_pos, Args&& ...args) noexcept ( _STD_ is_nothrow_constructible_v<T> ) {
    size_t i = start_pos;
    if constexpr (_STD_ is_nothrow_constructible_v<T>)
        (_STD_ construct_at(mem_ptr + i++, _STD_ forward<T>(args)), ...);
    else
        try { ((_STD_ construct_at(mem_ptr + i, _STD_ forward<T>(args)); i++), ...); } _DES_N_CLEANUP_
}

_MTEMPL_ inline constexpr void copy_construct(T *mem_ptr, T *source, size_t start_pos, size_t end_pos) {
    size_t i = start_pos;
    if constexpr ( _STD_ is_nothrow_copy_constructible_v<T> ) _STD_ uninitialized_copy_n(source, end_pos - start_pos, mem_ptr + start_pos);
    else try { for (; i < end_pos; i++) _STD_ construct_at(mem_ptr + i, source[i]); } _DES_N_CLEANUP_
}
_MATH_END_