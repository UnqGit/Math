// Static.hpp
#pragma once

#include "..\..\Helper\Helper.hpp"

namespace math::matrix::impl {
#define _DES_N_CLEANUP_ catch(...) { if constexpr (!TrvDtor<T>) std::destroy_n(m_data, i); throw; }

_MTEMPL_ inline constexpr void zero_construct(T *mem_ptr, size_t start_pos, size_t end_pos) {
    size_t i = start_pos;
    if constexpr (CpyCtor<T>) {
        if (zero_exists || !DfltCtor<T>) {
            const T &zero_val = zero_exists ? _GET_ZERO_ : T{};
            if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(mem_ptr + start_pos, end_pos, zero_val);
            else try { for (; i < end_pos; i++) std::construct_at(mem_ptr + i, zero_val); } _DES_N_CLEANUP_
        }
        else goto DEFAULT_CONSTRUCT;
        return;
    }
    else goto DEFAULT_CONSTRUCT;
    DEFAULT_CONSTRUCT:
        if constexpr (std::is_nothrow_default_constructible_v<T>) std::uninitialized_value_construct_n(mem_ptr + start_pos, end_pos);
        else try { for (; i < end_pos; i++) std::construct_at(mem_ptr + i); } _DES_N_CLEANUP_
}

_MTYPE_TEMPL(T  , ...Args) inline constexpr void variadic_construct(T *mem_ptr, size_t start_pos, Args&& ...args) noexcept ( std::is_nothrow_constructible_v<T> ) {
    size_t i = start_pos;
    if constexpr (std::is_nothrow_constructible_v<T>)
        (std::construct_at(mem_ptr + i++, std::forward<T>(args)), ...);
    else
        try { ((std::construct_at(mem_ptr + i, std::forward<T>(args)); i++), ...); } _DES_N_CLEANUP_
}

_MTEMPL_ inline constexpr void copy_construct(T *mem_ptr, T *source, size_t start_pos, size_t end_pos) {
    size_t i = start_pos;
    if constexpr ( std::is_nothrow_copy_constructible_v<T> ) std::uninitialized_copy_n(source, end_pos - start_pos, mem_ptr + start_pos);
    else try { for (; i < end_pos; i++) std::construct_at(mem_ptr + i, source[i]); } _DES_N_CLEANUP_
}
}