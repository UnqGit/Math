#pragma once

#include "Headers.hpp"
#include "Helper.hpp"

namespace math::matrix
{
    class Order {
        private:
            size_t m_rows, m_columns;
        
        public:
            Order (const size_t row, const size_t column) noexcept : m_rows(row), m_columns(column) {
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
        
        public:
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
}

namespace math
{
    template <typename T>
    class Matrix {
        private:
            T **m_matrix_start;
            math::matrix::Order m_order;
    };
}