// MatrixStatic.hpp
#pragma once

#include "..\Helper\Helper.hpp"

namespace math {
_MTEMPL_ requires NothrDtor<T> class _NODISC_ Matrix2x2 {
    public:
        Matrix2x2() requires CpyCtor<T> || DfltCtor<T> {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            if constexpr (CpyCtor<T>) {
                if (zero_exists || !DfltCtor<T>) {
                    const T &zero_val = zero_exists ? _GET_ZERO_ : T{};
                    if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_ptr, 4, zero_val);
                    else {
                        unsigned char i = 0;
                        try { for (; i < 4; i++) std::construct_at(m_ptr + i, zero_val); }
                        catch(...) { if constexpr (!TrvDtor<T>) std::destroy_n(m_ptr, i); throw; }
                    }
                }
                else goto DEFAULT_CONSTRUCT;
                return;
            }
            else goto DEFAULT_CONSTRUCT;
            DEFAULT_CONSTRUCT:
                if constexpr (std::is_nothrow_default_constructible_v<T>) std::uninitialized_value_construct_n(m_ptr, 4);
                else {
                    unsigned char i = 0;
                    try { for (; i < 4; i++) std::construct_at(m_ptr + i); }
                    catch(...) { if constexpr (!TrvDtor<T>) std::destroy_n(m_ptr, i); throw; }
                }
        }

        Matrix2x2(T &&a, T &&b, T &&c, T &&d) requires CpyCtor<T> || MvCtor<T> {
            unsigned char i = 0;
            try {
                std::construct_at(m_ptr + i, std::forward<T>(a)); ++i;
                std::construct_at(m_ptr + i, std::forward<T>(b)); ++i;
                std::construct_at(m_ptr + i, std::forward<T>(c)); ++i;
                std::construct_at(m_ptr + i, std::forward<T>(d)); ++i;                
            } catch(...) { if constexpr (!TrvDtor<T>) std::destroy_n(m_ptr, i); throw; }
        }

        Matrix2x2(T *data) requires CpyCtor<T> {
            if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                unsigned char i = 0;
                try { for (; i < 4; i++) std::construct_at(m_ptr + i, data[i]); }
                catch(...) { if constexpr (!TrvDtor<T>) std::destroy_n(m_ptr, i); throw; }
            } else std::uninitialized_copy_n(data, 4, m_ptr);
        }

        Matrix2x2(T data[2][2]) requires CpyCtor<T> {
            if constexpr (std::is_nothrow_copy_constructible_v<T>) {
                std::construct_at(m_ptr, data[0][0]);
                std::construct_at(m_ptr + 1, data[0][1]);
                std::construct_at(m_ptr + 2, data[1][0]);
                std::construct_at(m_ptr + 3, data[1][1]);
            }
            else {
                unsigned char i = 0;
                try {
                    std::construct_at(m_ptr, data[0][0]); ++i;
                    std::construct_at(m_ptr + 1, data[0][1]); ++i;
                    std::construct_at(m_ptr + 2, data[1][0]); ++i;
                    std::construct_at(m_ptr + 3, data[1][1]); ++i;
                }
                catch(...) { if constexpr (!TrvDtor<T>) std::destroy_n(m_ptr, i); throw; }
            }
        }

    public:
        Matrix2x2(const Matrix2x2 &other) requires CpyCtor<T> : Matrix2x2(other.m_ptr) {}
        Matrix2x2(Matrix2x2 &&other) noexcept {
            this->swap(other);
        }
        Matrix2x2 &operator=(const Matrix2x2 &other) {
            if (this != &other) {
                Matrix2x2 temp(other);
                this->swap(temp);
            }
            return *this;
        } 
        Matrix2x2 &operator=(Matrix2x2 &&other) noexcept {
            if (this != &other) this->swap(other);
            return *this;
        }

    public:
        _NODISC_ T &operator()(const size_t row, const size_t col) noexcept {
            return m_ptr[(row << 1) + col];
        }
        const T &operator()(const size_t row, const size_t col) const noexcept {
            return m_ptr[(row << 1) + col];
        }

        _NODISC_ T &at(const size_t row, const size_t col) {
            if (row >= 2 || col >= 2) throw std::out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_ptr[(row << 1) + col];
        }
        const T &at(const size_t row, const size_t col) const {
            if (row >= 2 || col >= 2) throw std::out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_ptr[(row << 1) + col];
        }
    
    public:
        _NODISC_ Matrix2x2 operator+(const Matrix2x2 &other) const requires math::isAdditive<T> {
            return Matrix2x2(m_ptr[0] + other.m_ptr[0], m_ptr[1] + other.m_ptr[1], m_ptr[2] + other.m_ptr[2], m_ptr[3] + other.m_ptr[3]);
        }
        Matrix2x2 &operator+=(const Matrix2x2 &other) requires math::compoundAddition<T> {
            if constexpr (noexcept( std::declval<T&>() += std::declval<T>() ))
                for (unsigned char i = 0; i < 4; i++) m_ptr[i] += other.m_ptr[i];
            else *this = *this + other;
            return *this;
        }

        _NODISC_ Matrix2x2 operator-(const Matrix2x2 &other) const requires math::isSubtractible<T> {
            return Matrix2x2(m_ptr[0] - other.m_ptr[0], m_ptr[1] - other.m_ptr[1], m_ptr[2] - other.m_ptr[2], m_ptr[3] - other.m_ptr[3]);
        }
        Matrix2x2 &operator-=(const Matrix2x2 &other) requires math::compoundSubtraction<T> {
            if constexpr (noexcept( std::declval<T&>() -= std::declval<T>() ))
                for (unsigned char i = 0; i < 4; i++) m_ptr[i] -= other.m_ptr[i];
            else *this = *this - other;
            return *this;
        }

        _NODISC_ Matrix2x2 operator*(const Matrix2x2 &other) const requires math::isAdditive<T> && math::isMultiplicative<T> {
            return this == &other ? this->square() : Matrix2x2 {
                m_ptr[0] * other(0, 0) + m_ptr[1] * other(1, 0),
                m_ptr[0] * other(0, 1) + m_ptr[1] * other(1, 1),
                m_ptr[2] * other(0, 0) + m_ptr[3] * other(1, 0),
                m_ptr[2] * other(0, 1) + m_ptr[3] * other(1, 1),
            };
        }
        Matrix2x2 &operator*=(const Matrix2x2 &other) {
            return this == &other ? this->square_in_place() : (*this = *this * other);
        }

    public:
        _NODISC_ Matrix2x2 square() requires math::isAdditive<T> && math::isMultiplicative<T> { // Assuming commutative-ness.
            // Better than normal multiplication as it has 5 multiplication and 3 addition instead of the normal 8 multiplication and 4 addition.
            const T bc = m_ptr[1] * m_ptr[2];
            const T apd = m_ptr[0] + m_ptr[3];
            return Matrix2x2 {
                m_ptr[0] * m_ptr[0] + bc,
                m_ptr[1] * apd,
                m_ptr[2] * apd,
                m_ptr[3] * m_ptr[3] + bc,
            };
        }

        Matrix2x2 &square_in_place() requires math::isAdditive<T> && math::isMultiplicative<T> {
            if constexpr (
                noexcept( std::declval<T>() + std::declval<T>() ) &&
                noexcept( std::declval<T>() * std::declval<T>() )
            ) {
                const T bc = m_ptr[1] * m_ptr[2];
                const T apd = m_ptr[0] + m_ptr[3];
                m_ptr[0] = m_ptr[0] * m_ptr[0] + bc;
                m_ptr[1] = m_ptr[1] * apd;
                m_ptr[2] = m_ptr[2] * apd;
                m_ptr[3] = m_ptr[3] * m_ptr[3] + bc;
            }
            else *this = this->square();
            return *this;
        }

        _NODISC_ T trace() requires math::isAdditive<T> {
            return m_ptr[0] + m_ptr[3];
        }

        _NODISC_ T determinant() requires math::isMultiplicative<T> && math::isSubtractible<T> {
            return m_ptr[0] * m_ptr[3] - m_ptr[1] * m_ptr[2];
        }

    public:
        _NODISC_ Matrix2x2 transpose() const {
            return Matrix2x2 {
                m_ptr[0], m_ptr[2],
                m_ptr[1], m_ptr[3],
            };
        }

        Matrix2x2 &transpose_in_place() {
            if constexpr (std::is_nothrow_swappable_v<T>)
                std::swap(m_ptr[1], m_ptr[2]);
            else *this = this->transpose();
            return *this;
        }

    public:
        _NODISC_ bool operator==(const Matrix2x2 &other) requires math::isEqualityOperationPossible<T> {
            return  math::is_equal(m_ptr[0], other.m_ptr[0]) &&
                    math::is_equal(m_ptr[1], other.m_ptr[1]) &&
                    math::is_equal(m_ptr[2], other.m_ptr[2]) &&
                    math::is_equal(m_ptr[3], other.m_ptr[3]);
        }
        _NODISC_ bool operator!=(const Matrix2x2 &other) requires math::isEqualityOperationPossible<T> {
            return !(*this == other);
        }

    public:
        ~Matrix2x2() noexcept {
            if constexpr (!TrvDtor<T>) std::destroy_n(m_ptr, 4);
        }

    public:
        void swap(Matrix2x2 &other) noexcept {
            std::swap(m_ptr, other.m_ptr);
        }

    private:
        T m_ptr[4];
};

}