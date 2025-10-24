// MatrixStatic.hpp

#include "..\impl\Headers.hpp"
#include "..\impl\Helper.hpp"

_MATH_START_
_MTEMPL_ requires _STD_ is_nothrow_destructible_v<T> class _NODISC_ Matrix2x2 {
    public:
        Matrix2x2() requires _CPY_CSTR_ || _DFLT_CSTR_ {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw _STD_ logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            if constexpr (_CPY_CSTR_) {
                if (zero_exists || !_DFLT_CSTR_) {
                    const T &zero_val = zero_exists ? _GET_ZERO_ : T{};
                    if constexpr (_STD_ is_nothrow_copy_constructible_v<T>) _STD_ uninitialized_fill_n(m_ptr, 4, zero_val);
                    else {
                        unsigned char i = 0;
                        try { for (; i < 4; i++) _STD_ construct_at(m_ptr + i, zero_val); }
                        catch(...) { if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_ptr, i); throw; }
                    }
                }
                else goto DEFAULT_CONSTRUCT;
                return;
            }
            else goto DEFAULT_CONSTRUCT;
            DEFAULT_CONSTRUCT:
                if constexpr (_STD_ is_nothrow_default_constructible_v<T>) _STD_ uninitialized_value_construct_n(m_ptr, 4);
                else {
                    unsigned char i = 0;
                    try { for (; i < 4; i++) _STD_ construct_at(m_ptr + i); }
                    catch(...) { if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_ptr, i); throw; }
                }
        }

        Matrix2x2(T &&a, T &&b, T &&c, T &&d) requires _CPY_CSTR_ || _MV_CSTR_ {
            unsigned char i = 0;
            try {
                _STD_ construct_at(m_ptr + i, _STD_ forward<T>(a)); ++i;
                _STD_ construct_at(m_ptr + i, _STD_ forward<T>(b)); ++i;
                _STD_ construct_at(m_ptr + i, _STD_ forward<T>(c)); ++i;
                _STD_ construct_at(m_ptr + i, _STD_ forward<T>(d)); ++i;                
            } catch(...) { if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_ptr, i); throw; }
        }

        Matrix2x2(T *data) requires _CPY_CSTR_ {
            if constexpr (!_STD_ is_nothrow_copy_constructible_v<T>) {
                unsigned char i = 0;
                try { for (; i < 4; i++) _STD_ construct_at(m_ptr + i, data[i]); }
                catch(...) { if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_ptr, i); throw; }
            } else _STD_ uninitialized_copy_n(data, 4, m_ptr);
        }

        Matrix2x2(T data[2][2]) requires _CPY_CSTR_ {
            if constexpr (_STD_ is_nothrow_copy_constructible_v<T>) {
                _STD_ construct_at(m_ptr, data[0][0]);
                _STD_ construct_at(m_ptr + 1, data[0][1]);
                _STD_ construct_at(m_ptr + 2, data[1][0]);
                _STD_ construct_at(m_ptr + 3, data[1][1]);
            }
            else {
                unsigned char i = 0;
                try {
                    _STD_ construct_at(m_ptr, data[0][0]); ++i;
                    _STD_ construct_at(m_ptr + 1, data[0][1]); ++i;
                    _STD_ construct_at(m_ptr + 2, data[1][0]); ++i;
                    _STD_ construct_at(m_ptr + 3, data[1][1]); ++i;
                }
                catch(...) { if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_ptr, i); throw; }
            }
        }

    public:
        Matrix2x2(const Matrix2x2 &other) requires _CPY_CSTR_ : Matrix2x2(other.m_ptr) {}
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
            if (row >= 2 || col >= 2) throw _STD_ out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_ptr[(row << 1) + col];
        }
        const T &at(const size_t row, const size_t col) const {
            if (row >= 2 || col >= 2) throw _STD_ out_of_range("Provided index does not exist within the bounds of this Matrix.");
            return m_ptr[(row << 1) + col];
        }
    
    public:
        _NODISC_ Matrix2x2 operator+(const Matrix2x2 &other) const requires _MATH_ isAdditive<T> {
            return Matrix2x2(m_ptr[0] + other.m_ptr[0], m_ptr[1] + other.m_ptr[1], m_ptr[2] + other.m_ptr[2], m_ptr[3] + other.m_ptr[3]);
        }
        Matrix2x2 &operator+=(const Matrix2x2 &other) requires _MATH_ compoundAddition<T> {
            if constexpr (noexcept( _STD_ declval<T&>() += _STD_ declval<T>() ))
                for (unsigned char i = 0; i < 4; i++) m_ptr[i] += other.m_ptr[i];
            else *this = *this + other;
            return *this;
        }

        _NODISC_ Matrix2x2 operator-(const Matrix2x2 &other) const requires _MATH_ isSubtractible<T> {
            return Matrix2x2(m_ptr[0] - other.m_ptr[0], m_ptr[1] - other.m_ptr[1], m_ptr[2] - other.m_ptr[2], m_ptr[3] - other.m_ptr[3]);
        }
        Matrix2x2 &operator-=(const Matrix2x2 &other) requires _MATH_ compoundSubtraction<T> {
            if constexpr (noexcept( _STD_ declval<T&>() -= _STD_ declval<T>() ))
                for (unsigned char i = 0; i < 4; i++) m_ptr[i] -= other.m_ptr[i];
            else *this = *this - other;
            return *this;
        }

        _NODISC_ Matrix2x2 operator*(const Matrix &other) const requires _MATH_ isAdditive<T> && _MATH_ isMultiplicative<T> {
            return this == &other ? this->square() : Matrix {
                m_ptr[0] * other(0, 0) + m_ptr[1] * other(1, 0),
                m_ptr[0] * other(0, 1) + m_ptr[1] * other(1, 1),
                m_ptr[2] * other(0, 0) + m_ptr[3] * other(1, 0),
                m_ptr[2] * other(0, 1) + m_ptr[3] * other(1, 1),
            };
        }
        Matrix &operator*=(const Matrix &other) {
            return this == &other ? this->square_in_place() : (*this = *this * other);
        }

    public:
        _NODISC_ Matrix2x2 square() requires _MATH_ isAdditive<T> && _MATH_ isMultiplicative<T> { // Assuming commutative-ness.
            // Better than normal multiplication as it has 5 multiplication and 3 addition instead of the normal 8 multiplication and 4 addition.
            const T bc = m_ptr[1] * m_ptr[2];
            const T apd = m_ptr[0] + m_ptr[3];
            return Matrix {
                m_ptr[0] * m_ptr[0] + bc,
                m_ptr[1] * apd,
                m_ptr[2] * apd,
                m_ptr[3] * m_ptr[3] + bc,
            };
        }

        Matrix2x2 &square_in_place() requires _MATH_ isAdditive<T> && _MATH_ isMultiplicative<T> {
            if constexpr (
                noexcept( _STD_ declval<T>() + _STD_ declval<T>() ) &&
                noexcept( _STD_ declval<T>() * _STD_ declval<T>() )
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

        _NODISC_ T trace() requires _MATH_ isAdditive<T> {
            return m_ptr[0] + m_ptr[3];
        }

        _NODISC_ T determinant() requires _MATH_ isMultiplicative<T> && _MATH_ isSubtractible<T> {
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
            if constexpr (_STD_ is_nothrow_swappable_v<T>)
                _STD_ swap(m_ptr[1], m_ptr[2]);
            else *this = this->transpose();
            return *this;
        }

    public:
        _NODISC_ bool operator==(const Matrix2x2 &other) requires _MATH_ isEqualityOperationPossible<T> {
            return  _MATH_ is_equal(m_ptr[0], other.m_ptr[0]) &&
                    _MATH_ is_equal(m_ptr[1], other.m_ptr[1]) &&
                    _MATH_ is_equal(m_ptr[2], other.m_ptr[2]) &&
                    _MATH_ is_equal(m_ptr[3], other.m_ptr[3]);
        }
        _NODISC_ bool operator!=(const Matrix2x2 &other) requires _MATH_ isEqualityOperationPossible<T> {
            return !(*this == other);
        }

    public:
        ~Matrix2x2() noexcept {
            if constexpr (!_TRV_DSTR_) _STD_ destroy_n(m_ptr, 4);
        }

    public:
        void swap(Matrix2x2 &other) noexcept {
            _STD_ swap(m_ptr, other.m_ptr);
        }

    private:
        T m_ptr[4];
};

_MATH_END_