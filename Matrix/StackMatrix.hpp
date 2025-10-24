// StackMatrix.hpp
#pragma once

#include "..\Helper\Helper.hpp"

_MATH_START_
#define _DES_N_CLEANUP_ catch(...) { if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_data, i); throw; }

template <typename T, size_t R, size_t C> requires _NOTHR_DSTR_
class MatrixS;

// Zero size specialization.
template <typename T> requires _NOTHR_DSTR_
class MatrixS<T, 0, 0> {};
template <typename T, size_t R> requires _NOTHR_DSTR_
class MatrixS<T, R, 0> {};
template <typename T, size_t C> requires _NOTHR_DSTR_
class MatrixS<T, 0, C> {};

namespace {

_MTEMPL_ inline constexpr void zero_construct(T *mem_ptr, size_t start_pos, size_t end_pos) {
    if constexpr (_CPY_CSTR_) {
        if (zero_exists || !_DFLT_CSTR_) {
            const T &zero_val = zero_exists ? _GET_ZERO_ : T{};
            if constexpr (_STD_ is_nothrow_copy_constructible_v<T>) _STD_ uninitialized_fill_n(mem_ptr + start_pos, end_pos, zero_val);
            else {
                size_t i = start_pos;
                try { for (; i < end_pos; i++) _STD_ construct_at(mem_ptr + i, zero_val); } _DES_N_CLEANUP_
            }
        }
        else goto DEFAULT_CONSTRUCT;
        return;
    }
    else goto DEFAULT_CONSTRUCT;
    DEFAULT_CONSTRUCT:
        if constexpr (_STD_ is_nothrow_default_constructible_v<T>) _STD_ uninitialized_value_construct_n(mem_ptr + start_pos, end_pos);
        else {
            size_t i = start_pos;
            try { for (; i < end_pos; i++) _STD_ construct_at(mem_ptr + i); } _DES_N_CLEANUP_
        }
}

_MTYPE_TEMPL(T, ...Args) inline constexpr void variadic_construct(T *mem_ptr, size_t start_pos, T &&first, Args&& ...args) noexcept ( _STD_ is_nothrow_constructible_v<T> ) {
    _STD_ construct_at(mem_ptr, _STD_ forward<T>(first));
    size_t i = start_pos;
    if constexpr (_STD_ is_nothrow_constructible_v<T>)
        (_STD_ construct_at(mem_ptr + ++i, _STD_ forward<T>(args)), ...);
    else
        try { (_STD_ construct_at(mem_ptr + 1 + (i++), _STD_ forward<T>(args)), ...); } _DES_N_CLEANUP_
}

}

template <typename T, size_t R, size_t C> requires _NOTHR_DSTR_
class _NODISC_ MatrixS {
    public:
        constexpr MatrixS() requires _DFLT_CSTR_ || _CPY_CSTR_ {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw _STD_ logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            zero_construct<T>(m_data, 0, R * C);
        }

        template <typename ...Args>
        requires _MATH_ allSameType<T, ...Args> && (sizeof...(Args) == (R * C - 1))
        constexpr MatrixS(T &&first, Args&& ...args) noexcept ( _STD_ is_nothrow_constructible_v<T> ) {
            variadic_construct(m_data, 0, _STD_ move(first), _STD_ forward<T>(args)...);
        }
        template <typename ...Args>
        requires _MATH_ allSameType<T, ...Args> && (sizeof...(Args) == (R * C - 1))
        constexpr MatrixS(const T &first, Args&& ...args) noexcept ( _STD_ is_nothrow_constructible_v<T> ) {
            variadic_construct(m_data, 0, _STD_ forward(first), _STD_ forward<T>(args)...);
        }

        template <typename ...Args>
        requires _MATH_ allSameType<T, ...Args> && (sizeof...(Args) < (R * C - 1))
        constexpr MatrixS(T &&first, Args&& ...args) {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw _STD_ logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            variadic_construct(m_data, 0, _STD_ move(first), _STD_ forward(args)...);
            zero_construct<T>(m_data, (sizeof...(Args)) + 1, R * C);
        }
        template <typename ...Args>
        requires _MATH_ allSameType<T, ...Args> && (sizeof...(Args) < (R * C - 1))
        constexpr MatrixS(const T &first, Args&& ...args) {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw _STD_ logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            variadic_construct(m_data, 0, _STD_ forward(first), _STD_ forward(args)...);
            zero_construct<T>(m_data, (sizeof...(Args)) + 1, R * C);
        }

    public:
        _NODISC_ constexpr size_t num_rows() const noexcept { return R; }
        _NODISC_ constexpr size_t num_columns() const noexcept { return C; }
        _NODISC_ constexpr size_t column_len() const noexcept { return R; }
        _NODISC_ constexpr size_t row_len() const noexcept { return C; }

    private:
        T m_data[R * C];
};
_MATH_END_