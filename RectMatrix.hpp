#pragma once

#include <stdint.h>
#include <limits>
#include <initializer_list>
#include <algorithm>
#include <concepts>
#include <cmath>

namespace {
    template <std::integral T> // Just becuase it's better to send numbers by value instead of const&.
    inline bool is_equal(T a, T b) {
        return a == b;
    }

    template <std::floating_point T>
    inline bool is_equal(T a, T b) {
        T diff = std::fabs(a - b);
        T tol = std::numeric_limits<T>::epsilon() * std::max(std::fabs(a), std::fabs(b));
        return diff < tol;
    }

    template <typename T>
    requires ((!std::floating_point<T>) && (!std::integral<T>))
    inline bool is_equal(const T &a, const T &b) {
        return a == b;
    }
}

namespace Matrix {

    typedef enum class Direction : char {
        VERTICAL, HORIZONTAL
    } DR;

    typedef enum class Modifier : char {
        EXTEND, SHRINK, SAME_SIZE_MUST
    } MD;

    typedef enum class Initialize : char {
        DEFAULT_INITIALIZE, THROW
    } IT;

    typedef class Order {
        private:
            size_t m_rows;
            size_t m_columns;
        
        public:
            Order(size_t rows, size_t columns): m_rows(rows), m_columns(columns) {}

        public:
            inline size_t get_rows(void) const noexcept {
                return m_rows;
            }

            inline size_t get_columns(void) const noexcept {
                return m_columns;
            }

            inline bool operator==(const Order &other) const noexcept {
                return ((m_rows == other.m_rows) && (m_columns == other.m_columns));
            }

            inline bool operator!=(const Order &other) const noexcept {
                return !(*this == other);
            }
        
        public:
            inline Order opp(void) const noexcept {
                return Order(m_columns, m_rows);
            }
    } OR;

    template <typename Type>
    class Rect {
        private:
            size_t m_rows, m_columns;
            Type *m_data;
        
        // CONSTRUCTORS.
        public:
            Rect(void) {
                m_rows = 0;
                m_columns = 0;
                m_data = static_cast<Type*>(::operator new[](0));
            }

            Rect(const Matrix::Order &ord): m_rows(ord.get_rows()), m_columns(ord.get_columns()) requires std::default_initializable<Type> {
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                for (Type &t : (*this)) t = to_copy;
            }

            Rect(const Matrix::Order &ord, const Type &to_copy): m_rows(ord.get_rows()), m_columns(ord.get_columns()) {
                m_data = static_cast<Type*>(::operator new[](this->size() * sizeof(Type)));
                for (Type &t: *this) {
                    t = to_copy;
                }
            }
            
            Rect(const size_t rows, const size_t columns): m_rows(rows), m_columns(columns) requires std::default_initializable<Type> {
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                Type to_copy;
                for (Type &t : (*this)) t = to_copy;
            }

            Rect(const size_t rows, const size_t columns, const Type &to_copy): m_rows(rows), m_columns(columns) {
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                for (Type &t: *this) {
                    t = to_copy;
                }
            }

            Rect(const Type * const array, const size_t rows, const size_t columns): m_rows(rows), m_columns(columns) {
                if (this->size() == 0) {
                    m_rows = 0;
                    m_columns = 0;
                }
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                std::uninitialized_copy(array, array + this->size(), m_data);
            }
            
            Rect(const std::initializer_list<Type> &il, const size_t rows, const size_t columns, const Matrix::Initialize init = Matrix::IT::DEFAULT_INITIALIZE): m_rows(rows), m_columns(columns) {
                if (this->size() == 0) {
                    m_rows = 0;
                    m_columns = 0;
                }
                if constexpr (std::default_initializable<Type>) {
                    size_t min = std::min(this->size(), il.size());
                    if (this->size() > il.size()) {
                        switch (init) {
                            case Matrix::IT::DEFAULT_INITIALIZE:
                                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                                break;
                            case Matrix::IT::THROW:
                                throw std::invalid_argument("Size of matrix can't be greater than initializer list size(Throw enabled).");
                        }
                    }
                    else m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                    std::uninitialized_copy(il.begin(), il.begin() + min, m_data);
                    Type to_copy;
                    for (size_t i = min; i < this->size(); i++) {
                        m_data[i] = to_copy;
                    }
                }
                else {
                    if (this->size() > il.size()) throw std::invalid_argument("Size of matrix can't be greater than initializer list(only for non-default_initializable objects.)");
                    else m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                    std::uninitialized_copy(il.begin(), il.end(), m_data);
                }
            }

            Rect(const std::initializer_list<Type> &il, const Matrix::Direction dir = Matrix::DR::HORIZONTAL) {
                switch (dir) {
                    case Matrix::DR::HORIZONTAL:
                        m_rows = il.size() != 0;
                        m_columns = il.size();
                        break;
                    case Matrix::DR::VERTICAL:
                        m_rows = il.size();
                        m_columns = il.size() != 0;
                        break;
                }
                m_data = static_cast<Type*>(::operator new[](il.size() * sizeof(Type)));
                std::uninitialized_copy(il.begin(), il.end(), m_data);
            }
            
            Rect(const std::initializer_list<std::initializer_list<Type>> &ill, const  Matrix::Modifier modi = Matrix::MD::EXTEND) {
                auto FALLBACK = [&]() -> void {
                    m_rows = 0;
                    m_columns = 0;
                    if constexpr (std::default_initializable<Type>) {
                        m_data = new Type[0]();
                    }
                    else m_data = static_cast<Type*>(::operator new[](0));
                };
                switch (modi) {
                    case Matrix::MD::EXTEND:
                        size_t max_size = 0;
                        for (const std::initializer_list<Type> &il : ill) 
                            if (il.size() > max_size) max_size = il.size();
                        if (max_size == 0) {
                            FALLBACK();
                            return;
                        }
                        m_rows = ill.size();
                        m_columns = max_size;
                        m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                        size_t row_num = 0;
                        for (const std::initializer_list<Type> &il: ill) {
                            std::uninitialized_copy(il.begin(), il.end(), m_data + (i++) * max_size);
                        }
                        if (il.size() != max_size && (!std::default_initializable<Type>)) {
                            throw std::logic_error("Cannot create default value for the type inside Rect; Hence, cannot extend on creation.");
                        }
                        else {
                            const Type to_copy();
                            for (size_t i = il.size(); i < max_size; i++) {
                                m_data[i] = to_copy;
                            }
                        }
                        break;
                    case Matrix::MD::SHRINK:
                        size_t min_size = std::numeric_limits<size_t>::max();
                        for (const std::initializer_list<Type> &il : ill) 
                            if (il.size() < min_size) min_size = il.size();
                        m_rows = ill.size();
                        m_columns = min_size;
                        if (min_size == 0) {
                            FALLBACK();
                            return;
                        }
                        m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                        size_t i = 0;
                        for (const std::initializer_list<Type> &il: ill) {
                            std::uninitialized_copy(il.begin(), il.begin() + min_size, m_data + (i++) * min_size);
                        }
                        break;
                    case Matrix::MD::SAME_SIZE_MUST:
                        if (ill.size() != 0) {
                            size_t col_size = ill[0].size();
                            for (const std::initializer_list<Type> &il: ill) {
                                if (il.size() != col_size) throw std::logic_error("Every row doesn't contain the same number of columns. Err: at Rect construction.");
                            }
                            m_rows = ill.size();
                            m_columns = (*(ill.begin())).size();
                            m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                            size_t i = 0;
                            for (const std::initializer_list<Type> &il : ill)
                                for (const Type &tp : il)
                                    m_data[i++] = tp;
                        }
                        else {
                            FALLBACK();
                            return;
                        }
                }
            }

            Rect(const Rect &other): m_rows(other.m_rows), m_columns(other.m_columns) {
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                std::uninitialized_copy(other.m_data, other.m_data + m_rows * m_columns, m_data);
            }

            Rect(Rect &&other): m_rows(other.m_rows), m_columns(other.m_columns), m_data(other.m_data) {
                other.m_data = nullptr;
                other.m_rows = 0;
                other.m_columns = 0;
            }

            Rect &operator=(const Rect &other) {
                if (this == &other) return *this;
                m_rows = other.m_rows;
                m_columns = other.m_columns;
                if (m_data != nullptr) ::operator delete[](m_data);
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                std::uninitialized_copy(other.m_data, other.m_data + m_rows * other.m_columns, m_data);
                return *this;
            }

            Rect &operator=(Rect &&other) {
                if (this == &other) return *this;
                if (m_data != nullptr) ::operator delete[](m_data);
                m_rows = other.m_rows;
                m_columns = other.m_columns;
                m_data = other.m_data;
                other.m_data = nullptr;
                other.m_rows = 0;
                other.m_columns = 0;
                return *this;
            }

            ~Rect(void) {
                if (m_data != nullptr) ::operator delete[](m_data);
            }

        // ACCESSING
        public:
            // No bounds checking.
            inline Type &operator()(const size_t row, const size_t column) noexcept {
                return m_data[row * m_columns + column];
            }

            const inline Type &operator()(const size_t row, const size_t column) const noexcept {
                return m_data[row * m_columns + column];
            }

            inline Type &at(const size_t row, const size_t column) {
                if (row >= m_rows || column >= m_columns) throw std::logic_error("Cannot access the outside bounds of matrix.");
                return m_data[row * m_columns + column];
            }
            
            const inline Type &at(const size_t row, const size_t column) const {
                if (row >= m_rows || column >= m_columns) throw std::logic_error("Cannot access the outside bounds of matrix.");
                return m_data[row * m_columns + column];
            }

            inline Type *begin(void) noexcept {
                return m_data;
            }

            const inline Type *begin(void) const noexcept {
                return m_data;
            }

            inline Type *end(void) noexcept {
                return m_data + m_rows * m_columns;
            }
            
            const inline Type *end(void) const noexcept {
                return m_data + m_rows * m_columns;
            }

            inline size_t size(void) const noexcept {
                return m_rows * m_columns;
            }

            inline Matrix::OR order(void) const noexcept {
                return Matrix::OR(m_rows, m_columns);
            }

            inline size_t column_len(void) const noexcept {
                return m_columns;
            }

            inline size_t row_len(void) const noexcept {
                return m_rows;
            }
            
        // MATH OPERATIONS.
        public:
            Rect &operator+=(const Rect &other) {
                if (!is_sum_possible(other)) throw std::logic_error("Cannot add matrices of different order with each other.");
                for (size_t i = 0; i < other.size(); i++) {
                    m_data[i] += other.m_data[i];
                }
                return *this;
            }
            
            Rect &operator-=(const Rect &other) {
                if (!is_sum_possible(other)) throw std::logic_error("Cannot add matrices of different order with each other.");
                if (this == &other) {
                    *this = Rect(other.order());
                    return *this;
                }
                for (size_t i = 0; i < this->size(); i++) {
                    m_data[i] -= other.m_data[i];
                }
                return *this;
            }
            
            Rect operator+(const Rect &other) {
                Rect newer(*this);
                newer += other;
                return newer;
            }
            
            Rect operator-(const Rect &other) {
                Rect newer(*this);
                newer -= other;
                return newer;
            }

            inline Rect &negate(void) noexcept {
                for (Type &t : *this) t = -t;
                return *this;
            }

            inline Rect operator-(void) const noexcept {
                Rect newer(*this);
                return newer.negate();
            }

            Rect operator*(const Rect &other) const {
                if (m_columns != other.m_rows) throw std::logic_error("Can only multiply matrices if the number of columns of first and the number of rows of second are same.\n");
                Rect result(m_rows, other.m_columns);
                for (size_t i = 0; i < m_rows; i++) {
                    for (size_t j = 0; j < other.m_columns; j++) {
                        for (size_t k = 0; k < m_columns; k++) {
                            result(i, j) += (*this)(i, k) * other(k, j);
                        }
                    }
                }
                return result;
            }

            Rect &operator*=(const Rect &other) {
                *this = *this * other;
                return *this;
            }

        // TOOLKIT.
        public:
            inline bool is_sum_possible(const Rect &other) const noexcept {
                return this->order() == other.order();
            }

            inline bool is_prod_possible(const Rect &other) const noexcept {
                return m_columns == other.m_rows;
            }

            inline bool is_square(void) const noexcept {
                return m_rows == m_columns;
            }

            bool operator==(const Rect &other) const noexcept {
                if (this == &other) return true;
                if (this->order() != other.order()) return false;
                for (size_t i = 0; i < this->size(); i++) if (!( ::is_equal(m_data[i], other.m_data[i]))) return false;
                return true;
            }
            
            bool operator!=(const Rect &other) const noexcept {
                return !(*this == other);
            }

            bool is_zero(void) requires (std::default_initializable<Type>) const noexcept {
                const Type zero();
                for (const Type &t: (*this)) if (!( ::is_equal(t, zero))) return false;
                return true;
            }
            
            bool is_samev(const Type &val) const noexcept {
                for (const Type &t: (*this)) if (!( ::is_equal(t, val))) return false;
                return true;
            }

            bool is_rzero(const size_t row) requires (std::default_initializable<Type>) const noexcept {
                const Type zero();
                for (size_t i = 0; i < m_columns; i++) if (!( ::is_equal(m_data[row * m_columns + i], zero))) return false;
                return true;
            }
            
            bool is_czero(const size_t column) requires (std::default_initializable<Type>) const noexcept {
                const Type zero();
                for (size_t i = 0; i < m_rows; i++) if (!( ::is_equal(m_data[m_rows * i + column], zero))) return false;
                return true;
            }

            bool is_rsamev(const size_t row, const Type &val) const noexcept {
                for (size_t i = 0; i < m_columns; i++) if (!( ::is_equal(m_data[row * m_columns + i], val))) return false;
                return true;
            }
            
            bool is_csamev(const size_t column, const Type &val) const noexcept {
                for (size_t i = 0; i < m_rows; i++) if (!( ::is_equal(m_data[m_rows * i + column], val))) return false;
                return true;
            }
        public:
            
            // TO-DO: Under Construction
        };
        
}