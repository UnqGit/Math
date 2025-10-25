// MatrixUtils.hpp
#pragma once

#include "..\..\Helper\Helper.hpp"

_MMATRIX_START_
// A lightweight wrapper on two size_t(s).
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
            _STD_ swap(m_rows, m_columns);
        }
        void swap(Order &other) noexcept {
            _STD_ swap(m_rows, other.m_rows);
            _STD_ swap(m_columns, other.m_columns);
        }
        void set_row(const size_t row) noexcept {
            m_rows = row;
            if (m_rows == 0) m_columns = 0;
        }  
        void set_column(const size_t column) noexcept {
            m_columns = column;
            if (m_columns == 0) m_rows = 0;
        }
        void reset() noexcept {
            m_rows = 0; m_columns = 0;
        }
    };

// So we can use the matrix in STL functions like _STD_ sort using begin_one_d and end_one_d.
_MTEMPL_ class MatrixOneDIterator {
    public:
        using iterator_category = _STD_ random_access_iterator_tag;
        using difference_type   = _STD_ ptrdiff_t;
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

// So we can use the matrix in STL functions like _STD_ sort using begin_one_d and end_one_d.
_MTEMPL_ class MatrixOneDColumnIterator {
    public:
        using iterator_category = _STD_ random_access_iterator_tag;
        using difference_type   = _STD_ ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

    private:
        T** m_data;
        size_t m_row_size;
        size_t m_index;

    public:
        MatrixOneDColumnIterator(T **data, const size_t row_len, const size_t index = 0) noexcept : m_data(data), m_row_size(row_len), m_index(index) {}
        MatrixOneDColumnIterator(const MatrixOneDColumnIterator &other) noexcept = default;
    
    public:
        _NODISC_ bool operator==(const MatrixOneDColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_index == other.m_index));
        }
        _NODISC_ bool operator!=(const MatrixOneDColumnIterator &other) const noexcept {
            return ((m_data != other.m_data) || (m_index != other.m_index));
        }
        _NODISC_ bool operator>(const MatrixOneDColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_index > other.m_index));
        }
        _NODISC_ bool operator<(const MatrixOneDColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_index < other.m_index));
        }
        _NODISC_ bool operator<=(const MatrixOneDColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_index <= other.m_index));
        }
        _NODISC_ bool operator>=(const MatrixOneDColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_index >= other.m_index));
        }

    public:
        reference operator*() noexcept {
            return m_data[m_index % m_row_size][m_index / m_row_size];
        }
        pointer operator->() noexcept {
            return (m_data[m_index % m_row_size] + (m_index / m_row_size));
        }
        const reference operator*() const noexcept {
            return m_data[m_index % m_row_size][m_index / m_row_size];
        }
        const pointer operator->() const noexcept {
            return (m_data[m_index % m_row_size] + (m_index / m_row_size));
        }

    public:
        reference operator[](const size_t index) const noexcept {
            return m_data[(m_index + index) % m_row_size][(m_index + index) / m_row_size];
        }

    public:
        MatrixOneDColumnIterator operator++(int) noexcept {
            MatrixOneDColumnIterator prev(m_data, m_row_size, m_index++);
            return prev;
        }
        MatrixOneDColumnIterator &operator++() noexcept {
            ++m_index;
            return *this;
        }
        MatrixOneDColumnIterator operator--(int) noexcept {
            MatrixOneDColumnIterator prev(m_data, m_row_size, m_index--);
            return prev;
        }
        MatrixOneDColumnIterator &operator--() noexcept {
            --m_index;
            return *this;
        }
    
    public:
        _NODISC_ MatrixOneDColumnIterator operator+(const difference_type add) const noexcept {
            return MatrixOneDColumnIterator(m_data, m_row_size, m_index + add);
        }
        _NODISC_ MatrixOneDColumnIterator operator-(const difference_type sub) const noexcept {
            return MatrixOneDColumnIterator(m_data, m_row_size, m_index - sub);
        }
        MatrixOneDColumnIterator &operator+=(const difference_type add) noexcept {
            m_index += add;
            return *this;
        }
        MatrixOneDColumnIterator &operator-=(const difference_type sub) noexcept {
            m_index -= sub;
            return *this;
        }
        _NODISC_ difference_type operator-(const MatrixOneDColumnIterator &other) const noexcept {
            return (static_cast<long long>(m_index) - static_cast<long long>(other.m_index));
        }
};

// Column Iterator for view type column object of a row major matrix
_MTEMPL_ class ColumnIterator {
    public:
        using iterator_category = _STD_ random_access_iterator_tag;
        using difference_type   = _STD_ ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

    private:
        T **m_data;
        size_t m_col_index;

    public:
        ColumnIterator(T **data, const size_t col) : m_data(data), m_col_index(col) {}
        ColumnIterator(const ColumnIterator &other) noexcept = default;

    public:
        _NODISC_ bool operator==(const ColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_col_index == other.m_col_index));
        }
        _NODISC_ bool operator!=(const ColumnIterator &other) const noexcept {
            return ((m_data != other.m_data) || (m_col_index != other.m_col_index));
        }
        _NODISC_ bool operator>(const ColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_col_index > other.m_col_index));
        }
        _NODISC_ bool operator<(const ColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_col_index < other.m_col_index));
        }
        _NODISC_ _NODISC_ bool operator<=(const ColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_col_index <= other.m_col_index));
        }
        _NODISC_ bool operator>=(const ColumnIterator &other) const noexcept {
            return ((m_data == other.m_data) && (m_col_index >= other.m_col_index));
        }

    public:
        reference operator*() noexcept {
            return (*m_data)[m_col_index];
        }
        pointer operator->() noexcept {
            return (*m_data) + m_col_index;
        }
        const reference operator*() const noexcept {
            return (*m_data)[m_col_index];
        }
        const pointer operator->() const noexcept {
            return (*m_data) + m_col_index;
        }

    public:
        reference operator[](const size_t index) const noexcept {
            return (*(m_data + index))[m_col_index];
        }

    public:
        ColumnIterator operator++(int) noexcept {
            ColumnIterator prev(m_data++, m_col_index);
            return prev;
        }
        ColumnIterator &operator++() noexcept {
            ++m_data;
            return *this;
        }
        ColumnIterator operator--(int) noexcept {
            ColumnIterator prev(m_data--, m_col_index);
            return prev;
        }
        ColumnIterator &operator--() noexcept {
            --m_data;
            return *this;
        }

    public:
        _NODISC_ ColumnIterator operator+(const difference_type add) const noexcept {
            return ColumnIterator(m_data + add, m_col_index);
        }
        _NODISC_ ColumnIterator operator-(const difference_type sub) const noexcept {
            return ColumnIterator(m_data - sub, m_col_index);
        }
        ColumnIterator &operator+=(const difference_type add) noexcept {
            m_data += add;
            return *this;
        }
        ColumnIterator &operator-=(const difference_type sub) noexcept {
            m_data -= sub;
            return *this;
        }
        _NODISC_ ColumnIterator operator-(const ColumnIterator &other) const noexcept {
            return (m_data - other.m_data);
        }
};

// A view type row container.
_MTEMPL_ class Row {
    private:
        T *&m_data;
        size_t m_row_len;

    public:
        Row(T *&data, const size_t row_len) noexcept : m_data(data), m_row_len(row_len) {}
        Row(const Row &other) noexcept = default;

    public:
        _NODISC_ T &at(const size_t index) {
            if (index >= m_row_len) throw _STD_ out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
            return m_data[index];
        }
        const T &at(const size_t index) const {
            if (index >= m_row_len) throw _STD_ out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
            return m_data[index];
        }
        
        _NODISC_ T &operator[](const size_t index) noexcept {
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
        _NODISC_ size_t size() const noexcept {
            return m_row_len;
        }

        _NODISC_ bool is_same_as(const Row &other) const {
            return ((this == &other) || ((m_data == other.m_data) && (m_row_len == other.m_row_len)));
        }

        _NODISC_ bool are_all_same_as(const T &val_to_compare) const
        requires _MATH_ isEqualityOperationPossible<T> {
            if (m_row_len == 0) return false;
            for (size_t i = 0; i < m_row_len; i++)
                if (!_MATH_ is_equal(m_data[i], val_to_compare)) return false;
            return true;
        }

        _NODISC_ bool are_all_same() const
        requires _MATH_ isEqualityOperationPossible<T> {
            if (m_row_len < 2) return true;
            for (size_t i = 0; i < m_row_len - 1; i++)
                if (!_MATH_ is_equal(m_data[i], m_data[i + 1])) return false;
            return true;
        }

        _NODISC_ bool is_zero() const
        requires _MATH_ isEqualityOperationPossible<T> {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw _STD_ logic_error("Cannot check for is_zero property of the row as the zero value(stored in _MATH_ zero_vals or defautlt construction for the type) is not defined.");
            if constexpr (!_DFLT_CSTR_)
                return are_all_same_as(_GET_ZERO_);
            else return are_all_same_as(T{});
        }

        _NODISC_
        size_t count(const T &val_to_compare) const
        requires _MATH_ isEqualityOperationPossible<T> {
            size_t count = 0;
            for (size_t i = 0; i < m_row_len; i++) count += _MATH_ is_equal(m_data[i], val_to_compare);
            return count;
        }

    public:
        _NODISC_ bool operator==(const Row &other) const
        requires _MATH_ isEqualityOperationPossible<T> {
            if (this->is_same_as(other)) return true;
            for (size_t i = 0; i < m_row_len; i++)
                if (!_MATH_ is_equal(m_data[i], other.m_data[i])) return false;
            return true;
        }

        _NODISC_ bool operator!=(const Row &other) const
        requires _MATH_ isEqualityOperationPossible<T> {
            return !((*this)==other);
        }
};

// A view type column container.
_MTEMPL_ class Column {
    private:
        T **m_data;
        size_t m_column_index;
        size_t m_num_rows;

    public:
        Column(T **data, const size_t col, const size_t row) noexcept : m_data(data), m_column_index(col), m_num_rows(row) {}
        Column(const Column &other) noexcept = default;

    public:
        _NODISC_ T &at(const size_t index) {
            if (index >= m_num_rows) _STD_ out_of_range("Cannot access provided row on this column because it exceeds the number of rows in the matrix.");
            return m_data[index][m_column_index];
        }
        const T &at(const size_t index) const {
            if (index >= m_num_rows) _STD_ out_of_range("Cannot access provided row on this column because it exceeds the number of rows in the matrix.");
            return m_data[index][m_column_index];
        }
        
        _NODISC_ T &operator[](const size_t index) noexcept {
            return m_data[index][m_column_index];
        }
        const T &operator[](const size_t index) const noexcept {
            return m_data[index][m_column_index];
        }

    public:
        ColumnIterator<T> begin() noexcept {
            return ColumnIterator<T>(m_data, m_column_index);
        }
        const ColumnIterator<T> begin() const noexcept {
            return ColumnIterator<T>(m_data, m_column_index);
        }
        ColumnIterator<T> end() noexcept {
            return ColumnIterator<T>(m_data + m_num_rows, m_column_index);
        }
        const ColumnIterator<T> end() const noexcept {
            return ColumnIterator<T>(m_data + m_num_rows, m_column_index);
        }

    public:
        _NODISC_ size_t size() const noexcept {
            return m_num_rows;
        }

        _NODISC_ bool is_same_as(const Column &other) const noexcept {
            return ((this == &other) && ((m_data == other.m_data) || (m_column_index == other.m_column_index) || (m_num_rows == other.m_num_rows)));
        }

        _NODISC_ bool operator==(const Column &other) const
        requires _MATH_ isEqualityOperationPossible<T> {
            if (this->is_same_as(other)) return true;
            for (size_t i = 0; i < m_num_rows; i++) {
                if (!_MATH_ is_equal(m_data[i][m_column_index], other.m_data[i][m_column_index])) return false;
            }
            return true;
        }

        _NODISC_ bool operator!=(const Column &other) const
        requires _MATH_ isEqualityOperationPossible<T> {
            return !((*this)==other);
        }
};

// Iterator for the matrix class.
_MTEMPL_ class MatrixIterator {
    public:
        using iterator_category = _STD_ random_access_iterator_tag;
        using difference_type   = _STD_ ptrdiff_t;
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
        _NODISC_ bool operator==(const MatrixIterator &other) const noexcept {
            return (m_data == other.m_data);
        }
        _NODISC_ bool operator!=(const MatrixIterator &other) const noexcept {
            return (m_data != other.m_data);
        }
        _NODISC_ bool operator>(const MatrixIterator &other) const noexcept {
            return (m_data > other.m_data);
        }
        _NODISC_ bool operator<(const MatrixIterator &other) const noexcept {
            return (m_data < other.m_data);
        }
        _NODISC_ bool operator>=(const MatrixIterator &other) const noexcept {
            return (m_data >= other.m_data);
        }
        _NODISC_ bool operator<=(const MatrixIterator &other) const noexcept {
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
        _NODISC_ MatrixIterator operator+(const difference_type add) const noexcept {
            return MatrixIterator(m_data + add, m_row_len);
        }
        _NODISC_ MatrixIterator operator-(const difference_type sub) const noexcept {
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
        _NODISC_ difference_type operator-(const MatrixIterator &other) const noexcept {
            return (m_data - other.m_data);
        }
};

// Construction rules.

enum class ConstructAllocateRule : bool {
    zero, possible_garbage
};
using CAR = ConstructAllocateRule;

enum class ConstructOrientationRule : char {
    horizontal, vertical, main_diagonal, off_diagonal
};
using COR = ConstructOrientationRule;

enum class ConstructContainerRule : char {
    shrink, expand, must_be_same, are_same
};
using CCR = ConstructContainerRule;

enum class ConstructSquareRule : char {
    full, upper_half, lower_half, left_half, right_half, top_left_quarter, top_right_quarter, bottom_left_quarter, bottom_right_quarter, top_left_triangle, top_right_triangle, bottom_left_triangle, bottom_right_triangle, main_diagonal, off_diagonal, alternate, alternate_row, alternate_column
};
using CSR = ConstructSquareRule;
_MATH_END_