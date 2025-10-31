// StackMatrix.hpp
#pragma once

#include "Helper\Static.hpp"

namespace math {
template <typename T, size_t R, size_t C> requires NothrDtor<T>
class MatrixS;

// Zero size specialization.
template <typename T> requires NothrDtor<T>
class MatrixS<T, 0, 0> {};
template <typename T, size_t R> requires NothrDtor<T>
class MatrixS<T, R, 0> {};
template <typename T, size_t C> requires NothrDtor<T>
class MatrixS<T, 0, C> {};

template <typename T, size_t R, size_t C> requires NothrDtor<T>
class _NODISC_ MatrixS {
    public:
        constexpr MatrixS() requires DfltCtor<T> || CpyCtor<T> {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            math::matrix::impl::zero_construct(m_data, 0, R * C);
        }

        template <typename ...Args>
        requires allSameType<T, ...Args> && (sizeof...(Args) == R * C)
        constexpr MatrixS(Args&& ...args) noexcept ( std::is_nothrow_constructible_v<T> ) {
            math::matrix::impl::variadic_construct(m_data, 0, std::forward<T>(args)...);
        }

        template <typename ...Args>
        requires allSameType<T, ...Args> && (sizeof...(Args) < R * C)
        constexpr MatrixS(Args&& ...args) {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            math::matrix::impl::variadic_construct(m_data, 0, std::forward<T>(args)...);
            math::matrix::impl::zero_construct(m_data, sizeof...(Args), R * C);
        }

    public:
        constexpr MatrixS(read_ptr<T> data) requires CpyCtor<T> {
            math::matrix::impl::copy_construct(m_data, data, 0, R * C);
        }

        constexpr MatrixS(read_ptr<T> data, size_t buffer_size) requires CpyCtor<T> {
            math::matrix::impl::copy_construct(m_data, data, 0, std::min(R * C, buffer_size));
            if (buffer_size < R * C) zero_construct(m_data, buffer_size, R * C);
        }

        constexpr MatrixS(read_ptr2d<T> data) requires CpyCtor<T> {
            for (size_t i = 0; i < R; i++) math::matrix::impl::copy_construct(m_data, data[i], i * C, (i + 1) * C);
        }
        
        constexpr MatrixS(read_ptr<T> data[C], size_t rows) requires CpyCtor<T> {
            if (rows < R) {
                _ZERO_EXISTS_
                _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            }
            const size_t min_size = std::min(R, rows);
            for (size_t i = 0; i < min_size; i++) math::matrix::impl::copy_construct(m_data, data[i], i * C, (i + 1) * C);
            if (R > rows) zero_construct(m_data, rows * C, R * C);
        }

    public:
        constexpr MatrixS(const MatrixS &other) : MatrixS(other.m_data) {}
        constexpr MatrixS(MatrixS &&other) noexcept {
            std::swap(m_data, other.m_data);
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
        constexpr void swap(MatrixS &other) noexcept requires std::is_nothrow_swappable_v<T> {
            std::swap(m_data, other.m_data);
        }

    public:
        constexpr ~MatrixS() noexcept {
            if constexpr (!TrvDtor<T>) std::destroy_n(m_data, R * C);
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
            if (row >= R || col >= C) throw std::out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_data[row * C + col];
        }
        _NODISC_ constexpr const T &at()(const size_t row, const size_t col) const {
            if (row >= R || col >= C) throw std::out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_data[row * C + col];
        }

    public:
        constexpr MatrixS &operator+=(const MatrixS &other) {
            if constexpr ( std::declval<T&> += _DECL_ ) {
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
}