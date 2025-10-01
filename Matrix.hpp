#pragma once

#include "Headers.hpp"
#include "Helper.hpp"

namespace math::matrix::impl
{
    template <typename T>
    void destroy_data(T **const data, const size_t curr_i, const T *const end_curr_i, const size_t row_size, const size_t col_size) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < curr_i; i++) {
                std::destroy(data[i], data[i] + col_size);
            }
            std::destroy(data[curr_i], end_curr_i);
        }
        for (size_t i = 0; i < row_size; i++) {
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
            }
    
            void set_column(const size_t column) noexcept {
                m_columns = column;
            }

        };

    template <typename T, size_t row_num>
    class RowIterator {
        private:
            T* m_ptr;
        
        public:
            RowIterator(const T* ptr) noexcept : m_ptr(ptr) {}
            RowIterator(const RowIterator &other) noexcept : m_ptr(other.m_ptr) {}

        public:
            bool operator==(const RowIterator other) const noexcept {
                return (m_ptr == other.m_ptr);
            }
            bool operator!=(const RowIterator other) const noexcept {
                return (m_ptr != other.m_ptr);
            }
            bool operator>(const RowIterator other) const noexcept {
                return (m_ptr > other.m_ptr);
            }
            bool operator<(const RowIterator other) const noexcept {
                return (m_ptr < other.m_ptr);
            }
            bool operator>=(const RowIterator other) const noexcept {
                return (m_ptr>=other.m_ptr);
            }
            bool operator<=(const RowIterator other) const noexcept {
                return (m_ptr<=other.m_ptr);
            }

        public:
            T operator*() noexcept {
                return *m_ptr;
            }
            const T operator*() const noexcept {
                return static_cast<const T*>(m_ptr);
            }
        
        public:
            RowIterator operator++() noexcept {
                RowIterator prev(m_ptr++);
                return prev;
            }
            RowIterator operator++(int) noexcept {
                ++m_ptr;
                return *this;
            }
            RowIterator operator--() noexcept {
                RowIterator prev(m_ptr--);
                return prev;
            }
            RowIterator operator--(int) noexcept {
                --m_ptr;
                return *this;
            }

        public:
            RowIterator operator+(size_t add) const noexcept {
                return RowIterator(m_ptr + add);
            }
            RowIterator operator-(size_t sub) const noexcept {
                return RowIterator(m_ptr - sub);
            }
            RowIterator operator+=(size_t add) noexcept {
                m_ptr += add;
                return *this;
            }
            RowIterator operator-=(size_t sub) noexcept {
                m_ptr -= sub;
                return *this;
            }
            long long operator-(const RowIterator &other) const noexcept {
                return (m_ptr - other.m_ptr);
            }
    };

    template <typename T, size_t row_num>
    class Row {
        private:
            T *m_data;
            size_t m_col_size;
        public:
            T &at(const size_t index) {
                if (index >= m_col_size) throw std::out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
                return m_data[index];
            }
            const T &at(const size_t index) const {
                if (index >= m_col_size) throw std::out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
                return m_data[index];
            }

        public:
            RowIterator<T, row_num> begin() noexcept {
                return RowIterator<T, row_num>(m_data);
            }
            RowIterator<T, row_num> end() noexcept {
                return RowIterator<T, row_num>(m_data + m_col_size);
            }
            const RowIterator<T, row_num> begin() const noexcept {
                return RowIterator<T, row_num>(m_data);
            }
            const RowIterator<T, row_num> end() const noexcept {
                return RowIterator<T, row_num>(m_data + m_col_size);
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
}

namespace math
{
    template <typename T>
    class Matrix {
        private:
            T **m_data = nullptr;
            math::matrix::Order m_order;

        public:
            Matrix() noexcept {}

            Matrix(const math::matrix::Order &order, const math::matrix::CAR construct_rule = math::matrix::CAR::possible_garbage) : m_order(order) {
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                for (size_t i = 0; i < row; i++) {
                    m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * column));
                }
                if (construct_rule == math::matrix::CAR::possible_garbage) {
                    if constexpr (std::is_trivially_constructible_v<T>) return;
                    else if constexpr (std::is_default_constructible_v<T>) {
                        T *end = nullptr;
                        for (size_t i = 0; i < row; i++) {
                            try {
                                end = std::uninitialized_default_construct_n(m_data[i], column);
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, end, row, column);
                                throw;
                            }
                        }
                        return;
                    }
                    if (math::helper::zero_vals.exists_of<T>()) {
                        const T zero_val(math::helper::zero_vals.get_of<T>());
                        T *end = nullptr;
                        for (size_t i = 0; i < row; i++) {
                            try {
                                end = std::uninitialized_fill_n(m_data[i], column, zero_val);
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, end, row, column);
                                throw;
                            }
                        }
                        return;
                    }
                    if constexpr (std::is_default_constructible_v<T>) {
                        T *end = nullptr;
                        for (size_t i = 0; i < row; i++) {
                            try {
                                end = std::uninitialized_value_construct_n(m_data[i], column);
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, end, row, column);
                                throw;
                            }
                        }
                    }
                    else throw std::logic_error("Cannot construct the matrix for this type because neither zero value is stored and neither is it default constructible.");
                }
                else {
                    if (math::helper::zero_vals.exists_of<T>()) {
                        T zero_val(math::helper::zero_vals.get_of<T>());
                        T *end = nullptr;
                        for (size_t i = 0; i < row; i++) {
                            try {
                                end = std::uninitialized_fill_n(m_data[i], column, zero_val);
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, end, row, column);
                                throw;
                            }
                        }
                    }
                    else throw std::logic_error("The zero value is not stored of this type in math::helper::zero_vals hence can't zero construct the matrix.");
                }
            }
            Matrix(const size_t row, const size_t column, const math::matrix::CAR construct_rule = math::matrix::CAR::possible_garbage) : Matrix(math::matrix::Order(row, column), construct_rule) {}

            Matrix(const math::matrix::Order &order, const T &to_copy) : m_order(order) {
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                T *end = nullptr;
                for (size_t i = 0; i < row; i++) {
                    m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * column));
                }
                for (size_t i = 0; i < row; i++) {
                    try {
                        end = std::uninitialized_fill_n(m_data[i], column, to_copy);
                    } catch(...) {
                        math::matrix::impl::destroy_data<T>(m_data, i, end, row, column);
                        throw;
                    }
                }
            }
            Matrix(const size_t row, const size_t column, const T &to_copy) : Matrix(math::matrix::Order(row, column), to_copy) {}

        public:
            Matrix(const T **const data, const math::matrix::Order &order) : m_order(order) {
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                T* end = nullptr;
                for (size_t i = 0; i < row; i++) {
                    m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * column));
                }
                for (size_t i = 0; i < row; i++) {
                    try {
                        end = std::uninitialized_copy(data[i], data[i] + column, m_data[i]);
                    } catch(...) {
                        math::matrix::impl::destroy_data<T>(m_data, i, end, row, column);
                        throw;
                    }
                }
            }
            Matrix(const T **const data, const size_t row, const size_t column) : Matrix(data, math::matrix::Order(row, column)) {}

            Matrix(const T *const data, const size_t size, math::matrix::COR construct_rule = math::matrix::COR::horizontal) {
                T *end = nullptr;
                bool zero_exists = (math::helper::zero_vals.exists_of<T>());
                switch (construct_rule) {
                    case math::matrix::COR::horizontal :
                        m_order = math::matrix::Order(1, size);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*)));
                        try {
                            end = std::uninitialized_copy(data, data + size, m_data[0]);
                        } catch(...) {
                            math::matrix::impl::destroy_data<T>(m_data, 0, end, m_order.row(), m_order.column());
                            throw;
                        }
                        return;
                    case math::matrix::COR::vertical :
                        m_order = math::matrix::Order(size, 1);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        for (size_t i = 0; i < size; i++) {
                            m_data[i] = static_cast<T*>(::operator new[](sizeof(T)));
                            try {
                                std::construct_at(m_data[i], data[i]);
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, m_data[i], m_order.row(), m_order.column());
                                throw;
                            }
                        }
                        return;
                    case math::matrix::COR::main_diagonal :
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        m_order = math::matrix::Order(size, size);
                        for (size_t i = 0; i < size; i++) {
                            m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * size));
                            try {
                                if (zero_exists) {
                                    end = std::uninitialized_fill_n(m_data[i], i, math::helper::zero_vals.get_of<T>());
                                    end = std::construct_at(m_data[i] + i, data[i]);
                                    end = std::uninitialized_fill_n(m_data[i] + i + 1, size - 1 - i, math::helper::zero_vals.get_of<T>());
                                }
                                else if constexpr (std::is_default_constructible_v<T>) {
                                    end = std::uninitialized_value_construct_n(m_data[i], i);
                                    end = std::construct_at(m_data[i] + i, data[i]);
                                    end = std::uninitialized_value_construct_n(m_data[i] + i + 1, size - 1 - i);
                                }
                                else throw std::logic_error("Cannot construct the matrix for this type because neither zero value is stored and neither is it default constructible.");
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, end, m_order.row(), m_order.column());
                                throw;
                            }
                        }
                        return;
                    case math::matrix::COR::off_diagonal :
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        m_order = math::matrix::Order(size, size);
                        for (size_t i = 0; i < size; i++) {
                            m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * size));
                            try {
                                if (zero_exists) {
                                    end = std::uninitialized_fill_n(m_data[i], size - 1 - i, math::helper::zero_vals.get_of<T>());
                                    end = std::construct_at(m_data[i] + size - 1 - i, data[i]);
                                    end = std::uninitialized_fill_n(m_data[i] + size - i, i, math::helper::zero_vals.get_of<T>());
                                }
                                else if constexpr (std::is_default_constructible_v<T>) {
                                    end = std::uninitialized_value_construct_n(m_data[i], size - 1 - i);
                                    end = std::construct_at(m_data[i] + i, data[i]);
                                    end = std::uninitialized_value_construct_n(m_data[i] + i + 1, i);
                                }
                                else throw std::logic_error("Cannot construct the matrix for this type because neither zero value is stored and neither is it default constructible.");
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, end, m_order.row(), m_order.column());
                                throw;
                            }
                        }
                        return;
                }
            }
        
        public:
            Matrix(const Matrix &other) : Matrix(other.m_data, other.m_order) {}
            Matrix(Matrix &&other) noexcept : m_order(other.m_order), m_data(other.m_data) {
                other.m_data = nullptr;
                other.m_order = math::matrix::Order();
            }
            Matrix &operator=(const Matrix &other) {
                if (this == &other) return *this;
                Matrix temp(other);
                (*this).swap(temp);
                return *this;
            }
            Matrix &operator=(Matrix &&other) {
                if (this == &other) return *this;
                if (m_data != nullptr) std::destroy_at(this);
                (*this).swap(other);
                other.m_order = math::matrix::Order();
            }

        public:
            ~Matrix() {
                math::matrix::impl::destroy_data(m_data, m_order.row(), m_data[m_order.row()] + m_order.column(), m_order.row(), m_order.column());
            }

        public:
            T &operator()(const size_t row, const size_t column) noexcept {
                return m_data[row][column];
            }
            const T &operator()(const size_t row, const size_t column) const noexcept {
                return m_data[row][column];
            }
            T &at(const size_t row, const size_t column) {
                if ((row >= m_order.row()) || (column >= m_order.column())) throw std::out_of_range("Provided index does not exist within the bounds of this matrix.");
                return m_data[row][column];
            }
            const T &at(const size_t row, const size_t column) const {
                if ((row >= m_order.row()) || (column >= m_order.column())) throw std::out_of_range("Provided index does not exist within the bounds of this matrix.");
                return m_data[row][column];
            }

        public:
            void swap(Matrix &other) noexcept {
                m_order.swap(other.m_order);
                std::swap(m_data, other.m_data);
            }

        public:
            const math::matrix::Order &order() const noexcept {
                return m_order;
            }
    };
}