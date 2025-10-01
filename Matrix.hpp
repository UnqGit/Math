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

            void set_row(const size_t row) noexcept {
                m_rows = row;
            }

            void set_column(const size_t column) noexcept {
                m_columns = column;
            }
    };

    enum class ConstructAllocateRule {
        zero, possible_garbage
    };
    using CAR = ConstructAllocateRule;
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
                m_data = ::operator new[](sizeof(T*) * row);
                for (size_t i = 0; i < row; i++) {
                    m_data[i] = ::operator new[](sizeof(T) * column);
                }
                if (construct_rule == math::matrix::CAR::possible_garbage) {
                    if constexpr (std::is_trivially_constructible_v<T>) return;
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
                                math::matrix::impl::destroy_data(m_data, i, end, row, column);
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
                m_data = ::operator new[](sizeof(T*) * row);
                T *end = nullptr;
                for (size_t i = 0; i < row; i++) {
                    m_data[i] = ::operator new[](sizeof(T) * column);
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
            Matrix(const Matrix &other) : m_order(other.m_order) {
                
            }
        public: // temporary.
            const T **data() const noexcept {
                return (const T**)m_data;
            }
            const math::matrix::Order order() const noexcept {
                return m_order;
            }
    };
}