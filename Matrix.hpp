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

    template <typename Container, typename RequiredData>
    concept isOneDArr = requires(Container obj) {
        { obj.size() } -> std::integral;
        { *(obj.begin()) } -> std::same_as<RequiredData&>;
        { obj.begin() } -> std::input_iterator;
        { obj.end() } -> std::input_iterator;
    };

    template <typename Container, typename RequiredData>
    concept isTwoDArr = requires(Container obj) {
        { obj.size() } -> std::integral;
        { obj.begin() } -> std::input_iterator;
        { obj.end() } -> std::input_iterator;

        requires isOneDArr<std::remove_reference_t<decltype(*obj.begin())>, RequiredData>;
    };
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
            RowIterator(const RowIterator &other) noexcept : m_ptr(other.m_ptr) {}

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
                return (m_ptr>=other.m_ptr);
            }
            bool operator<=(const RowIterator &other) const noexcept {
                return (m_ptr<=other.m_ptr);
            }

        public:
            reference operator*() noexcept {
                return *m_ptr;
            }
            const reference operator*() const noexcept {
                return *(static_cast<const T*>(m_ptr));
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
            difference_type operator-(const RowIterator &other) const noexcept {
                return (m_ptr - other.m_ptr);
            }
    };

    template <typename T>
    class Row {
        private:
            T *m_data;
            size_t m_row_len;

        public:
            Row(T *data, const size_t row_len) noexcept : m_data(data), m_row_len(row_len) {}

        public:
            T &at(const size_t index) {
                if (index >= m_row_len) throw std::out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
                return m_data[index];
            }
            const T &at(const size_t index) const {
                if (index >= m_row_len) throw std::out_of_range("Index is greater than number of elements in the row and hence can't be accessed.");
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
            Matrix(const T **data, const math::matrix::Order &order) : m_order(order) {
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
            Matrix(const T **data, const size_t row, const size_t column) : Matrix(data, math::matrix::Order(row, column)) {}

        public:
            Matrix(const T *data, const size_t size, math::matrix::COR construct_rule = math::matrix::COR::horizontal) {
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
            Matrix(const T *data, const math::matrix::Order &order) : m_order(order) {
                const size_t row = order.row();
                const size_t column = order.column();
                T *end = nullptr;
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                for (size_t i = 0; i < row; i++) {
                    m_data = static_cast<T*>(::operator new[](sizeof(T) * column));
                    try {
                        end = std::uninitialized_copy(data + i * column, data + (i + 1) * column, m_data[i]);
                    } catch(...) {
                        math::matrix::impl::destroy_data<T>(m_data, i, end, row, column);
                        throw;
                    }
                }
            }
            Matrix(const T *data, const size_t row, const size_t column) : Matrix(data, math::matrix::Order(row, column)) {}

        public:
            template <size_t C>
            Matrix(const T (*data)[C], const size_t num_rows) : m_order(math::matrix::Order(num_rows, C)) {
                T *end = nullptr;
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * num_rows));
                for (size_t i = 0; i < num_rows; i++) {
                    m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * C));
                    try {
                        end = std::uninitialized_copy(data[i], data[i] + C, m_data[i]);
                    } catch(...) {
                        math::matrix::impl::destroy_data<T>(m_data, i, end, num_rows, C);
                        throw;
                    }
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

            size_t num_rows() const noexcept {
                return m_order.row();
            }

            size_t num_columns() const noexcept {
                return m_order.column();
            }

            size_t size() const noexcept {
                return m_order.size();
            }

            bool is_square() const noexcept {
                return m_order.is_square();
            }

            bool is_row() const noexcept {
                return m_order.is_row();
            }

            bool is_column() const noexcept {
                return m_order.is_column();
            }

        public:
            math::matrix::Row<T> row(size_t row) const {
                if (row >= m_order.row()) throw std::out_of_range("Cannot provide row object for the provided row number.");
                return math::matrix::Row<T>(m_data[row], m_order.column());
            }
    };
}