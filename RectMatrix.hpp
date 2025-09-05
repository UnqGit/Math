#pragma once

#include <stdint.h>
#include <limits>
#include <initializer_list>
#include <algorithm>

namespace Matrix {

    typedef enum class Direction : char {
        VERTICAL, HORIZONTAL
    } DR;

    typedef enum class Modifier : char {
        EXTEND, SHRINK, SAME_SIZE_MUST
    } MD;

    typedef class Order {
        private:
            size_t m_rows;
            size_t m_columns;
        
        public:
            Order(size_t rows, size_t columns): m_rows(rows), m_columns(columns) {}

        public:
            inline size_t get_rows() const noexcept {
                return m_rows;
            }

            inline size_t get_columns() const noexcept {
                return m_columns;
            }
    } OR;

    template <typename Type>
    class Rect {
        private:
            size_t m_rows, m_columns;
            Type *m_data;
        
        // CONSTRUCTORS.
        public:
            Rect() {
                m_rows = 0;
                m_columns = 0;
                m_data = new Type[0]();
            }

            Rect(const Matrix::Order &ord): m_rows(ord.get_rows()), m_columns(ord.get_columns()) {
                m_data = new Type[this->size()]();
            }

            Rect(const Matrix::Order &ord, const Type &to_copy): m_rows(ord.get_rows()), m_columns(ord.get_columns()) {
                m_data = new Type[this->size()];
                for (const auto &t: *this) {
                    t = to_copy;
                }
            }
            
            Rect(const size_t rows, const size_t columns): m_rows(rows), m_columns(columns) {
                m_data = new Type[this->size()]();
            }

            Rect(const size_t rows, const size_t columns, const Type &to_copy): m_rows(rows), m_columns(columns) {
                m_data = new Type[this->size()];
                for (const auto &t: *this) {
                    t = to_copy;
                }
            }

            Rect(const Type * const array, const size_t rows, const size_t columns): m_rows(rows), m_columns(columns) {
                if (rows * columns == 0) {
                    m_rows = 0;
                    m_columns = 0;
                }
                m_data = new Type[rows * columns];
                std::copy(array, array + rows * columns, m_data);
            }

            Rect(const std::initializer_list<Type> &il, const size_t rows, const size_t columns): m_rows(rows), m_columns(columns) {
                if (rows * columns == 0) {
                    m_rows = 0;
                    m_columns = 0;
                }
                if (rows * columns != il.size()) m_data = new Type[il.size()]();
                else m_data = new Type[il.size()];
                size_t min = std::min(rows * columns, il.size());
                std::copy(il.begin(), il.end(), m_data);
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
                m_data = new Type[il.size()];
                std::copy(il.begin(), il.end(), m_data);
            }
            
            Rect(const std::initializer_list<std::initializer_list<Type>> &ill, const  Matrix::Modifier modi = Matrix::MD::EXTEND) {
                size_t i = 0;
                auto FALLBACK = [&]() -> void {
                    m_rows = 0;
                    m_columns = 0;
                    m_data = new Type[0]();
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
                        m_data = new Type[ill.size() * max_size]();
                        size_t row_num = 0;
                        for (const std::initializer_list<Type> &il: ill) {
                            std::copy(il.begin(), il.begin() + il.size(), m_data + (i++) * max_size);
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
                        m_data = new Type[ill.size() * min_size];
                        for (const std::initializer_list<Type> &il: ill) {
                            std::copy(il.begin(), il.begin() + min_size, m_data + (i++) * min_size);
                        }
                        break;
                    case Matrix::MD::SAME_SIZE_MUST:
                        if (ill.size() != 0) {
                            size_t col_size = ill[0].size();
                            for (const std::initializer_list<Type> &il: ill) {
                                if (il.size() != col_size) break;
                            }
                            if (i != ill.size()) throw std::logic_error("Every row doesn't contain the same number of columns. Err: at Rect construction.");
                            m_rows = ill.size();
                            m_columns = ill[0].size();
                            m_data = new Type[m_rows * m_columns];
                            i = 0;
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
                m_data = new Type[m_rows * m_columns];
                std::copy(other.m_data, other.m_data + m_rows * m_columns, m_data);
            }

            Rect(Rect &&other): m_rows(other.m_rows), m_columns(other.m_columns), m_data(other.m_data) {
                other.m_data = nullptr;
                other.m_rows = 0;
                other.m_columns = 0;
            }

            Rect& operator=(const Rect &other) {
                if (this == &other) return *this;
                m_rows = other.m_rows;
                m_columns = other.m_columns;
                if (m_data != nullptr) delete[] m_data;
                m_data = new Type[m_columns * m_rows];
                std::copy(other.m_data, other.m_data + m_rows * other.m_columns, m_data);
                return *this;
            }

            Rect& operator=(Rect &&other) {
                if (this == &other) return *this;
                if (m_data != nullptr) delete[] m_data;
                m_rows = other.m_rows;
                m_columns = other.m_columns;
                m_data = other.m_data;
                other.m_data = nullptr;
                other.m_rows = 0;
                other.m_columns = 0;
                return *this;
            }

            ~Rect() {
                if (m_data != nullptr) delete[] m_data;
            }

        // ACCESSING
        public:
            // No bounds checking.
            inline Type& operator()(const size_t row, const size_t column) noexcept {
                return m_data + row * m_columns + column;
            }

            const inline Type& operator()(const size_t row, const size_t column) const noexcept {
                return m_data + row * m_columns + column;
            }

            inline Type& at(const size_t row, const size_t column) {
                if (row >= m_rows || column >= m_columns) throw std::logic_error("Cannot access the outside bounds of matrix.");
                return m_data + row * m_columns + column;
            }
            
            const inline Type& at(const size_t row, const size_t column) const {
                if (row >= m_rows || column >= m_columns) throw std::logic_error("Cannot access the outside bounds of matrix.");
                return m_data + row * m_columns + column;
            }

            inline Type *begin() noexcept {
                return m_data;
            }

            const inline *begin() const noexcept {
                return m_data;
            }

            inline Type *end() noexcept {
                return m_data + m_rows * m_columns;
            }
            
            const inline Type *end() const noexcept {
                return m_data + m_rows * m_columns;
            }

            inline size_t size() const noexcept {
                return m_rows * m_columns;
            }

            inline Matrix::OR order() const noexcept {
                return Matrix::OR(m_rows, m_columns);
            }            
            // TO-DO: Under Construction


    };

}