// _matrix_impl_.hpp
#pragma once

#include "Headers.hpp"

namespace math::matrix::impl
{
    // This is for when the memory is allocated in a separate loop and constructed in another.
    template <typename T>
    void destroy_data(T **const data, const size_t curr_i, const T *const end_curr_i, const size_t num_row, const size_t row_size) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < curr_i; i++) {
                std::destroy(data[i], data[i] + row_size);
                ::operator delete[](data[i]);
            }
            std::destroy(data[curr_i], end_curr_i);
            for (size_t i = curr_i; i < num_row; i++) {
                ::operator delete[](data[i]);
            }
        }
        else {
            for (size_t i = 0; i < num_row; i++) {
                ::operator delete[](data[i]);
            }
        }
        ::operator delete[](data);
    }

    // This is for when the memory is allocated and constructed in the same loop.
    template <typename T>
    void destroy_data_continuous(T **const data, const size_t curr_i, const T *const end_curr_i, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                std::destroy(data[i], data[i] + row_size);
            }
            ::operator delete[](data[i]);
        }
        if constexpr (!std::is_trivially_destructible_v<T>) {
            std::destroy(data[curr_i], end_curr_i);
        }
        ::operator delete[](data[curr_i]);
        ::operator delete[](data);
    }

    // This is for when the error occurs in a pure memore allocation loop.
    template <typename T>
    void destroy_data_mem_err(T **const data, const size_t curr_i) {
        for (size_t i = 0; i < curr_i; i++) {
            ::operator delete[](data[i]);
        }
        ::operator delete[](data);
    }

    // This is for when the memory is being allocated continuously and is constructed in the same loop and the error occurs in the memory allocation.
    template <typename T>
    void destroy_data_mem_err_continuous(T **const data, const size_t curr_i, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) {
            if constexpr (!std::is_trivially_destructible_v<T>) std::destroy(data[i], data[i] + row_size);
            ::operator delete[](data[i]);
        }
        ::operator delete[](data);
    }
}

namespace math::matrix
{
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

    // random access Iterator class.
    template <typename T>
    class RowIterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = T;
            using pointer           = T*;
            using reference         = T&;

        private:
            pointer m_ptr;
        
        public:
            RowIterator(pointer ptr) noexcept : m_ptr(ptr) {}
            RowIterator(const RowIterator &other) noexcept = default;

        public:
            bool operator==(const RowIterator &other) const noexcept {
                return (m_ptr == other.m_ptr);
            }
            bool operator!=(const RowIterator &other) const noexcept {
                return (m_ptr != other.m_ptr);
            }
            bool operator>(const RowIterator &other) const noexcept {
                return (m_ptr > other.m_ptr);
            }
            bool operator<(const RowIterator &other) const noexcept {
                return (m_ptr < other.m_ptr);
            }
            bool operator>=(const RowIterator &other) const noexcept {
                return (m_ptr >= other.m_ptr);
            }
            bool operator<=(const RowIterator &other) const noexcept {
                return (m_ptr <= other.m_ptr);
            }

        public:
            reference operator*() noexcept {
                return *m_ptr;
            }
            pointer operator->() noexcept {
                return m_ptr;
            }
            const reference operator*() const noexcept {
                return *m_ptr;
            }
            const pointer operator->() const noexcept {
                return m_ptr;
            }
            
        public:
            RowIterator operator++(int) noexcept {
                RowIterator prev(m_ptr++);
                return prev;
            }
            RowIterator &operator++() noexcept {
                ++m_ptr;
                return *this;
            }
            RowIterator operator--(int) noexcept {
                RowIterator prev(m_ptr--);
                return prev;
            }
            RowIterator &operator--() noexcept {
                --m_ptr;
                return *this;
            }

        public:
            RowIterator operator+(const difference_type add) const noexcept {
                return RowIterator(m_ptr + add);
            }
            RowIterator operator-(const difference_type sub) const noexcept {
                return RowIterator(m_ptr - sub);
            }
            RowIterator &operator+=(const difference_type add) noexcept {
                m_ptr += add;
                return *this;
            }
            RowIterator &operator-=(const difference_type sub) noexcept {
                m_ptr -= sub;
                return *this;
            }
            difference_type operator-(const RowIterator &other) const noexcept {
                return (m_ptr - other.m_ptr);
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
            reference operator*() const noexcept {
                return m_data[m_index / m_row_size][m_index % m_row_size];
            }
            pointer operator->() const noexcept {
                return (m_data[m_index / m_row_size] + (m_index % m_row_size));
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

    template <typename T>
    class MatrixIterator;

    // A view type container.
    template <typename T>
    class Row {
        friend class MatrixIterator<T>;
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
            RowIterator<T> begin() noexcept {
                return RowIterator<T>(m_data);
            }
            RowIterator<T> end() noexcept {
                return RowIterator<T>(m_data + m_row_len);
            }
            const RowIterator<T> begin() const noexcept {
                return RowIterator<T>(m_data);
            }
            const RowIterator<T> end() const noexcept {
                return RowIterator<T>(m_data + m_row_len);
            }

        public:
            size_t size() const noexcept {
            return m_row_len;
        }
    };

    template <typename T>
    class MatrixIterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = Row<T>;
            using pointer           = Row<T>*;
            using reference         = Row<T>&;
        
        private:
            T** m_data;
            value_type m_cached_row;
        
        public:
            MatrixIterator(T **data, const size_t row_size) noexcept : m_data(data), m_cached_row(Row<T>(*m_data, row_size)) {}
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
                return m_cached_row;
            }
            pointer operator->() noexcept {
                return &m_cached_row;
            }
            const reference operator*() const noexcept {
                return m_cached_row;
            }
            const pointer operator->() const noexcept {
                return &m_cached_row;
            }

        public:
            MatrixIterator operator++(int) noexcept {
                MatrixIterator result(m_data++, m_cached_row.size());
                m_cached_row.m_data = *m_data;
                return result;
            }
            MatrixIterator &operator++() noexcept {
                ++m_data;
                m_cached_row.m_data = *m_data;
                return *this;
            }
            MatrixIterator operator--(int) noexcept {
                MatrixIterator result(m_data--, m_cached_row.size());
                m_cached_row.m_data = *m_data;
                return result;
            }
            MatrixIterator &operator--() noexcept {
                --m_data;
                m_cached_row.m_data = *m_data;
                return *this;
            }

        public:
            MatrixIterator operator+(const difference_type add) const noexcept {
                return MatrixIterator(m_data + add, m_cached_row.size());
            }
            MatrixIterator operator-(const difference_type sub) const noexcept {
                return MatrixIterator(m_data - sub, m_cached_row.size());
            }
            MatrixIterator &operator+=(const difference_type add) noexcept {
                m_data += add;
                m_cached_row.m_data = *m_data;
                return *this;
            }
            MatrixIterator &operator-=(const difference_type sub) noexcept {
                m_data -= sub;
                m_cached_row.m_data = *m_data;
                return *this;
            }
            difference_type operator-(const MatrixIterator &other) const noexcept {
                return (m_data - other.m_data);
            }
    };

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
}