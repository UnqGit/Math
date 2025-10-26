// StackMatrix.hpp
#pragma once

#include "Helper\Static.hpp"

_MATH_START_
template <typename T, size_t R, size_t C> requires _NOTHR_DSTR_
class MatrixS;

// Zero size specialization.
template <typename T> requires _NOTHR_DSTR_
class MatrixS<T, 0, 0> {};
template <typename T, size_t R> requires _NOTHR_DSTR_
class MatrixS<T, R, 0> {};
template <typename T, size_t C> requires _NOTHR_DSTR_
class MatrixS<T, 0, C> {};

template <typename T, size_t R, size_t C> requires _NOTHR_DSTR_
class _NODISC_ MatrixS {
    public:
        constexpr MatrixS() requires _DFLT_CSTR_ || _CPY_CSTR_ {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw _STD_ logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            _MAT_IMPL_ zero_construct(m_data, 0, R * C);
        }

        template <typename ...Args>
        requires allSameType<T, ...Args> && (sizeof...(Args) == R * C)
        constexpr MatrixS(Args&& ...args) noexcept ( _STD_ is_nothrow_constructible_v<T> ) {
            _MAT_IMPL_ variadic_construct(m_data, 0, _STD_ forward<T>(args)...);
        }

        template <typename ...Args>
        requires allSameType<T, ...Args> && (sizeof...(Args) < R * C)
        constexpr MatrixS(Args&& ...args) {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw _STD_ logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            _MAT_IMPL_ variadic_construct(m_data, 0, _STD_ forward<T>(args)...);
            _MAT_IMPL_ zero_construct(m_data, sizeof...(Args), R * C);
        }

    public:
        constexpr MatrixS(read_ptr<T> data) requires _CPY_CSTR_ {
            _MAT_IMPL_ copy_construct(m_data, data, 0, R * C);
        }

        constexpr MatrixS(read_ptr<T> data, size_t buffer_size) requires _CPY_CSTR_ {
            _MAT_IMPL_ copy_construct(m_data, data, 0, _STD_ min(R * C, buffer_size));
            if (buffer_size < R * C) zero_construct(m_data, buffer_size, R * C);
        }

        constexpr MatrixS(read_ptr2d<T> data) requires _CPY_CSTR_ {
            for (size_t i = 0; i < R; i++) _MAT_IMPL_ copy_construct(m_data, data[i], i * C, (i + 1) * C);
        }
        
        constexpr MatrixS(read_ptr<T> data[C], size_t rows) requires _CPY_CSTR_ {
            if (rows < R) {
                _ZERO_EXISTS_
                _NO_ZERO_COND_ throw _STD_ logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            }
            const size_t min_size = _STD_ min(R, rows);
            for (size_t i = 0; i < min_size; i++) _MAT_IMPL_ copy_construct(m_data, data[i], i * C, (i + 1) * C);
            if (R > rows) zero_construct(m_data, rows * C, R * C);
        }

    public:
        constexpr MatrixS(const MatrixS &other) : MatrixS(other.m_data) {}
        constexpr MatrixS(MatrixS &&other) noexcept {
            _STD_ swap(m_data, other.m_data);
        }
        constexpr MatrixS &operator=(const MatrixS &other) {
            if (this != &other) {
                MatrixS temp(other);
                this->swap(temp);
            }
            return *this;
        }
        constexpr MatrixS &operator=(Matrix &&other) {
            if (this != &other) this->swap(other);
            return *this;
        }
        constexpr void swap(MatrixS &other) noexcept requires _STD_ is_nothrow_swappable_v<T> {
            _STD_ swap(m_data, other.m_data);
        }

    public:
        constexpr ~MatrixS() noexcept {
            if constexpr (!_TRV_DSTR_) std::destroy_n(m_data, R * C);
        }

    public:
        _NODISC_ constexpr size_t num_rows() const noexcept { return R; }
        _NODISC_ constexpr size_t num_columns() const noexcept { return C; }
        _NODISC_ constexpr size_t column_len() const noexcept { return R; }
        _NODISC_ constexpr size_t row_len() const noexcept { return C; }
        _NODISC_ constexpr size_t size() const noexcept { return R * C; }

    public:
        _NODISC_ constexpr T &operator()(const size_t row, const size_t col) noexcept {
            return m_data[row * C + col];
        }
        _NODISC_ constexpr const T &operator()(const size_t row, const size_t col) const noexcept {
            return m_data[row * C + col];
        }
        
        _NODISC_ constexpr T &at()(const size_t row, const size_t col) {
            if (row >= R || col >= C) throw _STD_ out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_data[row * C + col];
        }
        _NODISC_ constexpr const T &at()(const size_t row, const size_t col) const {
            if (row >= R || col >= C) throw _STD_ out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_data[row * C + col];
        }

    public:
        constexpr MatrixS &operator+=(const MatrixS &other) {
            if constexpr ( _STD_ declval<T&> += _DECL_ ) {
                #pragma omp parallel for schedule(static)
                for (size_t i = 0; i < R * C; i++) m_data[i] += other.m_data[i];
            }
            else {

            }
        }
        _NODISC_ constexpr MatrixS operator+(const MatrixS &other) const {
            MatrixS result(*this);
            return (result += other);
        }

    private:
        T m_data[R * C];
};
_MATH_END_