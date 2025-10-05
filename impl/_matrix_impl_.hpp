// _matrix_impl_.hpp
#pragma once

#include "Headers.hpp"

namespace math::matrix
{
    // A lightweight wrapper on two size_t's.
    class Order {
        private:
            size_t m_rows, m_columns;
        
        public:
            Order() noexcept : m_rows(0), m_columns(0) {}
            Order(const size_t row, const size_t column) noexcept : m_rows(row), m_columns(column) {
                if ((m_rows == 0) || (m_columns == 0)) {
                    m_rows = 0;
                    m_columns = 0;
                }
            }

        public:
            bool operator==(const Order &other) const noexcept {
                return ((m_rows == other.m_rows) && (m_columns == other.m_columns));
            }
            bool operator!=(const Order &other) const noexcept {
                return ((m_rows != other.m_rows) || (m_columns != other.m_columns));
            }

        public:
            Order transpose() const noexcept {
                return Order(m_columns, m_rows);
            }
            bool is_row() const noexcept {
                return (m_columns == 1);
            }
            bool is_column() const noexcept {
                return (m_rows == 1);
            }
            bool is_square() const noexcept {
                return (m_rows == m_columns);
            }        
            bool is_zero() const noexcept {
                return (m_rows == 0);
            }
            bool is_tall() const noexcept {
                return (m_rows > m_columns);
            }
            bool is_wide() const noexcept {
                return (m_rows < m_columns);
            }

        public:
            size_t size() const noexcept {
                return m_rows * m_columns;
            }
            size_t row() const noexcept {
                return m_rows;
            }
            size_t column() const noexcept {
                return m_columns;
            }
    
        public:
            void flip() noexcept {
                std::swap(m_rows, m_columns);
            }
            void swap(Order &other) noexcept {
                std::swap(m_rows, other.m_rows);
                std::swap(m_columns, other.m_columns);
            }
            void set_row(const size_t row) noexcept {
                m_rows = row;
                if (m_rows == 0) m_columns = 0;
            }  
            void set_column(const size_t column) noexcept {
                m_columns = column;
                if (m_columns == 0) m_rows = 0;
            }

        };

    // So we can use the matrix in STL functions like std::sort using begin_one_d and end_one_d.
    template <typename T>
    class MatrixOneDIterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = T;
            using pointer           = T*;
            using reference         = T&;

        private:
            T** m_data;
            size_t m_row_size;
            size_t m_index;

        public:
            MatrixOneDIterator(T **data, const size_t row_len, const size_t index = 0) noexcept : m_data(data), m_row_size(row_len), m_index(index) {}
            MatrixOneDIterator(const MatrixOneDIterator &other) noexcept = default;
        
        public:
            bool operator==(const MatrixOneDIterator &other) const noexcept {
                return ((m_data == other.m_data) && (m_index == other.m_index));
            }
            bool operator!=(const MatrixOneDIterator &other) const noexcept {
                return ((m_data != other.m_data) || (m_index != other.m_index));
            }
            bool operator>(const MatrixOneDIterator &other) const noexcept {
                return ((m_data == other.m_data) && (m_index > other.m_index));
            }
            bool operator<(const MatrixOneDIterator &other) const noexcept {
                return ((m_data == other.m_data) && (m_index < other.m_index));
            }
            bool operator<=(const MatrixOneDIterator &other) const noexcept {
                return ((m_data == other.m_data) && (m_index <= other.m_index));
            }
            bool operator>=(const MatrixOneDIterator &other) const noexcept {
                return ((m_data == other.m_data) && (m_index >= other.m_index));
            }

        public:
            reference operator*() noexcept {
                return m_data[m_index / m_row_size][m_index % m_row_size];
            }
            pointer operator->() noexcept {
                return (m_data[m_index / m_row_size] + (m_index % m_row_size));
            }
            const reference operator*() const noexcept {
                return m_data[m_index / m_row_size][m_index % m_row_size];
            }
            const pointer operator->() const noexcept {
                return (m_data[m_index / m_row_size] + (m_index % m_row_size));
            }

        public:
            reference operator[](const size_t index) const noexcept {
                return m_data[(m_index + index) / m_row_size][(m_index + index) % m_row_size];
            }

        public:
            MatrixOneDIterator operator++(int) noexcept {
                MatrixOneDIterator prev(m_data, m_row_size, m_index++);
                return prev;
            }
            MatrixOneDIterator &operator++() noexcept {
                ++m_index;
                return *this;
            }
            MatrixOneDIterator operator--(int) noexcept {
                MatrixOneDIterator prev(m_data, m_row_size, m_index--);
                return prev;
            }
            MatrixOneDIterator &operator--() noexcept {
                --m_index;
                return *this;
            }
        
        public:
            MatrixOneDIterator operator+(const difference_type add) const noexcept {
                return MatrixOneDIterator(m_data, m_row_size, m_index + add);
            }
            MatrixOneDIterator operator-(const difference_type sub) const noexcept {
                return MatrixOneDIterator(m_data, m_row_size, m_index - sub);
            }
            MatrixOneDIterator &operator+=(const difference_type add) noexcept {
                m_index += add;
                return *this;
            }
            MatrixOneDIterator &operator-=(const difference_type sub) noexcept {
                m_index -= sub;
                return *this;
            }
            difference_type operator-(const MatrixOneDIterator &other) const noexcept {
                return (static_cast<long long>(m_index) - static_cast<long long>(other.m_index));
            }
    };

    // A view type container.
    template <typename T>
    class Row {
        private:
            T *m_data;
            size_t m_row_len;

        public:
            Row(T *data, const size_t row_len) noexcept : m_data(data), m_row_len(row_len) {}
            Row(const Row &other) noexcept = default;

        public:
            T &at(const size_t index) {
                if (index >= m_row_len) throw std::out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
                return m_data[index];
            }
            const T &at(const size_t index) const {
                if (index >= m_row_len) throw std::out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
                return m_data[index];
            }
            T &operator[](const size_t index) noexcept {
                return m_data[index];
            }
            const T &operator[](const size_t index) const noexcept {
                return m_data[index];
            }

        public:
            T *begin() noexcept {
                return m_data;
            }
            T *end() noexcept {
                return m_data + m_row_len;
            }
            const T *begin() const noexcept {
                return m_data;
            }
            const T *end() const noexcept {
                return m_data + m_row_len;
            }

        public:
            size_t size() const noexcept {
                return m_row_len;
            }

            bool is_same_as(const Row &other) const noexcept {
                return ((this == &other) || ((m_data == other.m_data) && (m_row_len == other.m_row_len)));
            }

            bool operator==(const Row &other) const {
                if (this == &other) return true;
                if (this->is_same_as(other)) return true;
                for (size_t i = 0; i < m_row_len; i++) {
                    if (!math::is_equal(m_data[i], other.m_data[i])) return false;
                }
                return true;
            }

            bool operator!=(const Row &other) const {
                return !((*this)==other);
            }
    };

    // Iterator for the matrix class.
    template <typename T>
    class MatrixIterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = Row<T>;
            using pointer           = Row<T>*;
            using reference         = Row<T>; // Proxy iterator.
        
        private:
            T** m_data;
            size_t m_row_len;
        
        public:
            MatrixIterator(T **data, const size_t row_len) noexcept : m_data(data), m_row_len(row_len) {}
            MatrixIterator(const MatrixIterator &other) noexcept = default;

        public:
            bool operator==(const MatrixIterator &other) noexcept {
                return (m_data == other.m_data);
            }
            bool operator!=(const MatrixIterator &other) noexcept {
                return (m_data != other.m_data);
            }
            bool operator>(const MatrixIterator &other) noexcept {
                return (m_data > other.m_data);
            }
            bool operator<(const MatrixIterator &other) noexcept {
                return (m_data < other.m_data);
            }
            bool operator>=(const MatrixIterator &other) noexcept {
                return (m_data >= other.m_data);
            }
            bool operator<=(const MatrixIterator &other) noexcept {
                return (m_data <= other.m_data);
            }

        public:
            reference operator*() noexcept {
                return Row<T>(*m_data, m_row_len);
            }
            const reference operator*() const noexcept {
                return Row<T>(*m_data, m_row_len);
            }
            
        public:
            MatrixIterator operator++(int) noexcept {
                MatrixIterator result(m_data++, m_row_len);
                return result;
            }
            MatrixIterator &operator++() noexcept {
                ++m_data;
                return *this;
            }
            MatrixIterator operator--(int) noexcept {
                MatrixIterator result(m_data--, m_row_len);
                return result;
            }
            MatrixIterator &operator--() noexcept {
                --m_data;
                return *this;
            }

        public:
            reference operator[](const size_t size) const noexcept {
                return Row<T>(*(m_data + size), m_row_len);
            }
            
        public:
            MatrixIterator operator+(const difference_type add) const noexcept {
                return MatrixIterator(m_data + add, m_row_len);
            }
            MatrixIterator operator-(const difference_type sub) const noexcept {
                return MatrixIterator(m_data - sub, m_row_len);
            }
            MatrixIterator &operator+=(const difference_type add) noexcept {
                m_data += add;
                return *this;
            }
            MatrixIterator &operator-=(const difference_type sub) noexcept {
                m_data -= sub;
                return *this;
            }
            difference_type operator-(const MatrixIterator &other) const noexcept {
                return (m_data - other.m_data);
            }
    };

    // Construction rules.
    
    enum class ConstructAllocateRule {
        zero, possible_garbage
    };
    using CAR = ConstructAllocateRule;

    enum class ConstructOrientationRule {
        horizontal, vertical, main_diagonal, off_diagonal
    };
    using COR = ConstructOrientationRule;

    enum class ConstructContainerRule {
        shrink, expand, must_be_same
    };
    using CCR = ConstructContainerRule;

    enum class ConstructSquareRule {
        full, upper_half, lower_half, left_half, right_half, top_left_quarter, top_right_quarter, bottom_left_quarter, bottom_right_quarter, top_left_triangle, top_right_triangle, bottom_left_triangle, bottom_right_triangle, main_diagonal, off_diagonal
    };
    using CSR = ConstructSquareRule;
}