#pragma once

#include <stdint.h>
#include <limits>
#include <initializer_list>
#include <algorithm>
#include <concepts>
#include <memory>
#include <type_traits>
#include <cmath>

namespace {
    template <std::integral T> // Just because it's better to send numbers by value instead of const T&.
    inline bool is_equal(const T a, const T b) {
        return a == b;
    }

    template <std::floating_point T>
    inline bool is_equal(const T a, const T b) {
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

    typedef enum class Restrict : char {
        ALLOW_GARBAGEV, NO_GARBAGEV, NO_CONSTRUCT
    } RT;

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
            Order(const size_t rows, const size_t columns): m_rows(rows), m_columns(columns) {}

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
                m_data = nullptr;
            }

            Rect(const Matrix::Order &ord, Matrix::Restrict res = Matrix::RT::NO_GARBAGEV): m_rows(ord.get_rows()), m_columns(ord.get_columns()) {
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                Type *end = m_data;
                switch (res) {
                    case Matrix::RT::NO_GARBAGEV:
                        if (std::default_initializable<Type>) {
                            try {
                                end = std::uninitialized_value_construct_n(m_data, this->size());
                            }
                            catch(...) {
                                if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                                ::operator delete[](m_data);
                                throw;
                            }
                        }
                        else throw std::invalid_argument("Cannot do 'No garbage' if default constructor doesn't exist.");
                        break;
                    case Matrix::RT::ALLOW_GARBAGEV:
                        if (std::is_fundamental_v<Type>) return;
                        if (std::default_initializable<Type>){
                            try {
                                end = std::uninitialized_value_construct_n(m_data, this->size());
                            }
                            catch(...) {
                                if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                                ::operator delete[](m_data);
                                throw;
                            }
                        }
                        else throw std::logic_error("Cannot allow garbage value for non-trivially constructible types.");
                        break;
                    case Matrix::RT::NO_CONSTRUCT:
                        return; 
                }
            }

            Rect(const Matrix::Order &ord, const Type &to_copy): m_rows(ord.get_rows()), m_columns(ord.get_columns()) {
                m_data = static_cast<Type*>(::operator new[](this->size() * sizeof(Type)));
                Type *end = m_data;
                try {
                    end = std::uninitialized_fill_n(m_data, this->size(), to_copy);
                }
                catch(...) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                    ::operator delete[](m_data);
                    throw;
                }
            }
            
            Rect(const size_t rows, const size_t columns, Matrix::Restrict res = Matrix::RT::NO_GARBAGEV): Rect(Matrix::Order(rows, columns), res) {}

            Rect(const size_t rows, const size_t columns, const Type &to_copy): Rect(Matrix::Order(rows, columns), to_copy) {}

            Rect(const Type * const array, const size_t rows, const size_t columns): m_rows(rows), m_columns(columns) {
                if (this->size() == 0) {
                    m_rows = 0;
                    m_columns = 0;
                    m_data = nullptr;
                    return;
                }
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                Type *end = m_data;
                try {
                    end = std::uninitialized_copy(array, array + this->size(), m_data);
                }
                catch(...) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                    ::operator delete[](m_data);
                    throw;
                }
            }
            
            Rect(const std::initializer_list<Type> &il, const size_t rows, const size_t columns, const Matrix::Initialize init = Matrix::IT::DEFAULT_INITIALIZE): m_rows(rows), m_columns(columns) {
                if (this->size() == 0) {
                    m_rows = 0;
                    m_columns = 0;
                    m_data = nullptr;
                    return;
                }
                if constexpr (std::default_initializable_v<Type>) {
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
                    Type *end = m_data;
                    try {
                        end = std::uninitialized_copy(il.begin(), il.begin() + min, m_data);
                    }
                    catch (...) {
                        if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                        ::operator delete[](m_data);
                        throw;
                    }
                    end = m_data + min;
                    try {
                        end = std::uninitialized_value_construct_n(m_data + min, this->size() - min);
                    }
                    catch(...) {
                        if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data + min, end);
                        ::operator delete[](m_data);
                        throw;
                    }
                }
                else {
                    if (this->size() > il.size()) throw std::invalid_argument("Size of matrix can't be greater than initializer list(only for non-default_initializable objects.)");
                    else m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                    std::uninitialized_copy(il.begin(), il.end(), m_data);
                }
            }

            Rect(const std::initializer_list<Type> &il, const Matrix::Direction dir = Matrix::DR::HORIZONTAL) {
                if (il.size() == 0) {
                    m_rows = 0;
                    m_columns = 0;
                    m_data = nullptr;
                    return;
                }
                switch (dir) {
                    case Matrix::DR::HORIZONTAL:
                        m_rows = 1;
                        m_columns = il.size();
                        break;
                    case Matrix::DR::VERTICAL:
                        m_rows = il.size();
                        m_columns = 1;
                        break;
                }
                m_data = static_cast<Type*>(::operator new[](il.size() * sizeof(Type)));
                Type *end = m_data;
                try {
                    end = std::uninitialized_copy(il.begin(), il.end(), m_data);
                }
                catch (...) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                    ::operator delete[](m_data);
                    throw;
                }
            }
            
            Rect(const std::initializer_list<std::initializer_list<Type>> &ill, const  Matrix::Modifier modi = Matrix::MD::SAME_SIZE_MUST) {
                auto FALLBACK = [&]() -> void {
                    m_rows = 0;
                    m_columns = 0;
                    m_data = nullptr;
                };
                size_t row_num = 0;
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
                        Type *end = m_data;
                        for (const std::initializer_list<Type> &il: ill) {
                            try {
                                std::uninitialized_copy(il.begin(), il.end(), m_data + row_num * max_size);
                            }
                            catch(...) {
                                if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                                ::operator delete[](m_data);
                                throw;
                            }
                            if (il.size() != max_size && (!std::default_initializable<Type>))
                                throw std::logic_error("Cannot create default value for the type inside Rect; Hence, cannot extend on creation.");
                            else {
                                try {
                                    end = std::uninitialized_value_construct_n(m_data + row_num * max_size + il.size(), max_size - il.size());
                                }
                                catch(...) {
                                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                                    ::operator delete[](m_data);
                                    throw;
                                }

                            }
                            row_num++;
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
                        Type *end = m_data;
                        for (const std::initializer_list<Type> &il: ill) {
                            try {
                                end = std::uninitialized_copy(il.begin(), il.begin() + min_size, m_data + (row_num++) * min_size);
                            }
                            catch(...) {
                                if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                                ::operator delete[](m_data);
                                throw;
                            }
                        }
                        break;

                    case Matrix::MD::SAME_SIZE_MUST:
                        if (ill.size() != 0) {
                            size_t col_size = (*(ill.begin())).size();
                            for (const std::initializer_list<Type> &il: ill)
                                if (il.size() != col_size) throw std::logic_error("Every row doesn't contain the same number of columns. Err: at Rect construction.");
                            m_rows = ill.size();
                            m_columns = col_size;
                            if (col_size == 0) {
                                FALLBACK();
                                return;
                            }
                            m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                            Type *end = m_data;
                            size_t i = 0;
                            for (const std::initializer_list<Type> &il : ill) {
                                try {
                                    std::uninitialized_copy(il.begin(), il.begin() + col_size, m_data + i);
                                }
                                catch(...) {
                                    if constexpr (!std::is_trivially_constructible_v<Type>) std::destroy(m_data, end);
                                    ::operator delete[](m_data);
                                    throw;
                                }
                                i += col_size;
                            }
                        }
                        else {
                            FALLBACK();
                            return;
                        }
                }
            }

            Rect(const Rect &other): Rect(other.m_data, other.m_rows, other.m_columns) {}

            Rect(Rect &&other): m_rows(other.m_rows), m_columns(other.m_columns), m_data(other.m_data) {
                other.m_data = nullptr;
                other.m_rows = 0;
                other.m_columns = 0;
            }

            Rect &operator=(const Rect &other) {
                if (this == &other) return *this;
                m_rows = other.m_rows;
                m_columns = other.m_columns;
                if (m_data != nullptr) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy((*this).begin(), (*this).end());
                    ::operator delete[](m_data);
                }
                m_data = static_cast<Type*>(::operator new[](sizeof(Type) * this->size()));
                Type *end = m_data;
                try {
                    end = std::uninitialized_copy(other.m_data, other.m_data + m_rows * other.m_columns, m_data);
                }
                catch(...) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy(m_data, end);
                    operator:: delete[](m_data);
                    throw;
                }
                return *this;
            }

            Rect &operator=(Rect &&other) {
                if (this == &other) return *this;
                m_rows = other.m_rows;
                m_columns = other.m_columns;
                if (m_data != nullptr) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy((*this).begin(), (*this).end());
                    ::operator delete[](m_data);
                }
                m_data = other.m_data;
                other.m_data = nullptr;
                other.m_rows = 0;
                other.m_columns = 0;
                return *this;
            }

            ~Rect(void) {
                if (m_data != nullptr) {
                    if constexpr (!std::is_trivially_destructible_v<Type>) std::destroy((*this).begin(), (*this).end());
                    ::operator delete[](m_data);
                }
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
                if (row >= m_rows || column >= m_columns) throw std::out_of_range("Cannot access the outside bounds of matrix.");
                return m_data[row * m_columns + column];
            }
            
            const inline Type &at(const size_t row, const size_t column) const {
                if (row >= m_rows || column >= m_columns) throw std::out_of_range("Cannot access the outside bounds of matrix.");
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
                for (size_t i = 0; i < this->size(); i++) {
                    m_data[i] -= other.m_data[i];
                }
                return *this;
            }
            
            Rect operator+(const Rect &other) const {
                Rect newer(*this);
                newer += other;
                return std::move(newer);
            }
            
            Rect operator-(const Rect &other) const {
                Rect newer(*this);
                newer -= other;
                return std::move(newer);
            }

            inline Rect &negate(void) noexcept {
                for (Type &t : *this) t = -t;
                return *this;
            }

            inline Rect operator-(void) const noexcept {
                Rect newer(*this);
                return std::move(newer.negate());
            }

            Rect operator*(const Rect &other) const {
                if (m_columns != other.m_rows) throw std::logic_error("Can only multiply matrices if the number of columns of first and the number of rows of second are same.\n");
                // TO-DO: change this function to something more generic.
                Rect result(m_rows, other.m_columns);
                for (size_t i = 0; i < m_rows; i++) {
                    for (size_t j = 0; j < other.m_columns; j++) {
                        for (size_t k = 0; k < m_columns; k++) {
                            result(i, j) += (*this)(i, k) * other(k, j);
                        }
                    }
                }
                return std::move(result);
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

            bool is_zero(void) const noexcept requires std::default_initializable<Type> {
                const Type zero{};
                for (const Type &t: (*this)) if (!( ::is_equal(t, zero))) return false;
                return true;
            }
            
            bool is_samev(const Type &val) const noexcept {
                for (const Type &t: (*this)) if (!( ::is_equal(t, val))) return false;
                return true;
            }

            bool is_rzero(const size_t row) const requires std::default_initializable<Type> {
                if (row >= m_rows) throw std::out_of_range("Cannot access row outside the range of matrix rows.");
                const Type zero{};
                for (size_t i = 0; i < m_columns; i++) if (!( ::is_equal((*this)(row, i), zero))) return false;
                return true;
            }
            
            bool is_czero(const size_t column) const requires std::default_initializable<Type> {
                if (column >= m_columns) throw std::out_of_range("Cannot access column outside the range of matrix columns.");
                const Type zero{};
                for (size_t i = 0; i < m_rows; i++) if (!( ::is_equal((*this)(i, column), zero))) return false;
                return true;
            }

            bool is_rsamev(const size_t row, const Type &val) const {
                if (row >= m_rows) throw std::out_of_range("Cannot access row outside the range of matrix rows.");
                for (size_t i = 0; i < m_columns; i++) if (!( ::is_equal((*this)(row, i), val))) return false;
                return true;
            }
            
            bool is_csamev(const size_t column, const Type &val) const {
                if (column >= m_columns) throw std::out_of_range("Cannot access column outside the range of matrix columns.");
                for (size_t i = 0; i < m_rows; i++) if (!( ::is_equal((*this)(i, column), val))) return false;
                return true;
            }
        
            void swap(Rect &other) noexcept {
                std::swap(m_data, other.m_data);
                std::swap(m_rows, other.m_rows);
                std::swap(m_columns, other.m_columns);
            }

        public:
            
            // TO-DO: Under Construction
        };
        
}