// Matrix.hpp
#pragma once

#include ".\impl\Headers.hpp"
#include ".\impl\Helper.hpp"
#include ".\impl\_matrix_impl_.hpp"

namespace math
{
    template <typename T>
    class Matrix {
        private:
            T **m_data = nullptr;
            math::matrix::Order m_order;

        public:
            Matrix() noexcept {}

            Matrix(const math::matrix::Order &order, const math::matrix::CAR construct_rule = math::matrix::CAR::zero) : m_order(order) {
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
            Matrix(const size_t row, const size_t column, const math::matrix::CAR construct_rule = math::matrix::CAR::zero) : Matrix(math::matrix::Order(row, column), construct_rule) {}

            Matrix(const math::matrix::Order &order, const T &to_copy) : m_order(order) {
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                T *end = nullptr;
                for (size_t i = 0; i < row; i++) {
                    try {
                        m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * column));
                        end = std::uninitialized_fill_n(m_data[i], column, to_copy);
                    } catch(...) {
                        math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, column);
                        throw;
                    }
                }
            }
            Matrix(const size_t row, const size_t column, const T &to_copy) : Matrix(math::matrix::Order(row, column), to_copy) {}
            
        public:
            Matrix(T *data, const size_t size, math::matrix::COR construct_rule = math::matrix::COR::horizontal) {
                T *end = nullptr;
                bool zero_exists = (math::helper::zero_vals.exists_of<T>());
                switch (construct_rule) {
                    case math::matrix::COR::horizontal :
                        m_order = math::matrix::Order(1, size);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*)));
                        try {
                            m_data[0] = static_cast<T*>(::operator new[](sizeof(T) * size));
                            end = std::uninitialized_copy_n(data, size, m_data[0]);
                        } catch(...) {
                            math::matrix::impl::destroy_data_continuous<T>(m_data, 0, end, size);
                            throw;
                        }
                        break;
                    case math::matrix::COR::vertical :
                        m_order = math::matrix::Order(size, 1);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T)));
                                std::construct_at(m_data[i], data[i]);
                            } catch(...) {
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, m_data[i], 1);
                                throw;
                            }
                        }
                        break;
                    case math::matrix::COR::main_diagonal :
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        m_order = math::matrix::Order(size, size);
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * size));
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
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, size);
                                throw;
                            }
                        }
                        break;
                    case math::matrix::COR::off_diagonal :
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        m_order = math::matrix::Order(size, size);
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * size));
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
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, size);
                                throw;
                            }
                        }
                        break;
                }
            }
            Matrix(T *data, const math::matrix::Order &order) : m_order(order) {
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
            Matrix(T *data, const size_t row, const size_t column) : Matrix(data, math::matrix::Order(row, column)) {}

        public:
            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, math::matrix::COR construct_rule = math::matrix::COR::horizontal) {
                T *end = nullptr;
                const size_t size = arr.size();
                auto Iter = arr.begin();
                bool zero_exists = math::helper::zero_vals.exists_of<T>();
                switch (construct_rule) {
                    case math::matrix::COR::horizontal :
                        m_order = math::matrix::Order(1, size);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*)));
                        try {
                            m_data[0] = static_cast<T*>(::operator new[](sizeof(T) * size));
                            end = std::uninitialized_copy(arr.begin(), arr.end(), m_data[0]);
                        } catch(...) {
                            math::matrix::impl::destroy_data<T>(m_data, 0, end, 1, size);
                            throw;
                        }
                        return;
                    case math::matrix::COR::vertical :
                        m_order = math::matrix::Order(size, 1);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T)));
                                end = std::construct_at(m_data[i], *Iter);
                                ++Iter;
                            } catch(...) {
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, m_order.column());
                                throw;
                            }
                        }
                        return;
                    case math::matrix::COR::main_diagonal :
                        m_order = math::matrix::Order(size, size);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * size));
                                if (zero_exists) {
                                    end = std::uninitialized_fill_n(m_data[i], size - 1 - i, math::helper::zero_vals.get_of<T>());
                                    end = std::construct_at(m_data[i] + size - 1 - i, *Iter);
                                    end = std::uninitialized_fill_n(m_data[i] + size - i, i, math::helper::zero_vals.get_of<T>());
                                }
                                else if constexpr (std::is_default_constructible_v<T>) {
                                    end = std::uninitialized_value_construct_n(m_data[i], size - 1 - i);
                                    end = std::construct_at(m_data[i] + i, *Iter);
                                    end = std::uninitialized_value_construct_n(m_data[i] + i + 1, i);
                                }
                                else throw std::logic_error("Cannot construct the matrix for this type because neither zero value is stored and neither is it default constructible.");
                                ++Iter;
                            } catch(...) {
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, m_order.column());
                                throw;
                            }
                        }
                        return;
                    case math::matrix::COR::off_diagonal :
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        m_order = math::matrix::Order(size, size);
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * size));
                                if (zero_exists) {
                                    end = std::uninitialized_fill_n(m_data[i], size - 1 - i, math::helper::zero_vals.get_of<T>());
                                    end = std::construct_at(m_data[i] + size - 1 - i, *Iter);
                                    end = std::uninitialized_fill_n(m_data[i] + size - i, i, math::helper::zero_vals.get_of<T>());
                                }
                                else if constexpr (std::is_default_constructible_v<T>) {
                                    end = std::uninitialized_value_construct_n(m_data[i], size - 1 - i);
                                    end = std::construct_at(m_data[i] + i, *Iter);
                                    end = std::uninitialized_value_construct_n(m_data[i] + i + 1, i);
                                }
                                else throw std::logic_error("Cannot construct the matrix for this type because neither zero value is stored and neither is it default constructible.");
                                ++Iter;
                            } catch(...) {
                                math::matrix::impl::destroy_data<T>(m_data, i, end, m_order.row(), m_order.column());
                                throw;
                            }
                        }
                        return;
                }
            }
            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, const math::matrix::Order &order) : m_order(order) {
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                const size_t size = arr.size();
                const size_t min = std::min(size, m_order.size());
                const bool zero_exists = math::helper::zero_vals.exists_of<T>();
                auto Iter = arr.begin();
                const auto IterEnd = arr.end();
                T *end = nullptr;
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                size_t j = 0;
                for (size_t i = 0; i < row; i++) {
                    try {
                        m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * column));
                        j = 0;
                        while (j < column && Iter != IterEnd) {
                            std::construct_at(m_data[i] + j, *Iter);
                            ++Iter;
                            ++j;
                        }
                        end = m_data[i] + j;
                        if (j != column) {
                            if (zero_exists) {
                                end = std::uninitialized_fill_n(m_data[i] + j, column - j, math::helper::zero_vals.get_of<T>());
                            }
                            else if constexpr (std::is_default_constructible_v<T>) {
                                end = std::uninitialized_value_construct_n(m_data[i] + j, column - j);
                            }
                            else throw std::logic_error("Cannot construct the matrix for this type because neither zero value is stored and neither is it default constructible.");
                        }
                    } catch(...) {
                        math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, column);
                        throw;
                    }
                }
            }
            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, const size_t row, const size_t column) : Matrix(arr, math::matrix::Order(row, column)) {}

        public:
            Matrix(T **data, const math::matrix::Order &order) : m_order(order) {
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                T* end = nullptr;
                for (size_t i = 0; i < row; i++) {
                    try {
                        m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * column));
                        end = std::uninitialized_copy(data[i], data[i] + column, m_data[i]);
                    } catch(...) {
                        math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, column);
                        throw;
                    }
                }
            }
            Matrix(T **data, const size_t row, const size_t column) : Matrix(data, math::matrix::Order(row, column)) {}
            
        public:
            template <typename U>
            requires math::helper::isTwoDArr<U, T>
            Matrix(const U &arr, const math::matrix::ConstructContainerRule construct_rule = math::matrix::CCR::must_be_same) {
                const size_t size = arr.size();
                if (size == 0) return;
                if (size == 1) {
                    *this = Matrix(*(arr.begin()), math::matrix::COR::horizontal);
                    return;
                }
                size_t row_size;
                auto Iter = arr.begin();
                auto end = arr.end();
                const bool zero_exists = math::helper::zero_vals.exists_of<T>();
                T *err_end = nullptr;
                size_t j;
                switch (construct_rule) {
                    case math::matrix::CCR::shrink :
                        row_size = Iter->size();
                        while (Iter != end) {
                            row_size = std::min(row_size, Iter->size());
                            ++Iter;
                        }
                        if (row_size == 0) return;
                        m_order = math::matrix::Order(size, row_size);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        Iter = arr.begin();
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * row_size));
                                err_end = std::uninitialized_copy_n(Iter->begin(), row_size, m_data[i]);
                            } catch(...) {
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, err_end, row_size);
                                throw;
                            }
                            ++Iter;
                        }
                        return;
                    case math::matrix::CCR::must_be_same :
                        row_size = Iter->size();
                        while (Iter != end) {
                            if (row_size != Iter->size()) throw std::invalid_argument("Rows of the provided container are not of the same size and rule chosen is math::matrix::ConstructContainerRule::must_be_same.");
                            ++Iter;
                        }
                        m_order = math::matrix::Order(size, row_size);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        Iter = arr.begin();
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * row_size));
                                err_end = std::uninitialized_copy(Iter->begin(), Iter->end(), m_data[i]);
                            } catch(...) {
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, err_end, row_size);
                                throw;
                            }
                            ++Iter;
                        }
                        return;
                    case math::matrix::CCR::expand :
                        row_size = Iter->size();
                        while (Iter != end) {
                            row_size = std::max(row_size, Iter->size());
                            ++Iter;
                        }
                        Iter = arr.begin();
                        m_order = math::matrix::Order(size, row_size);
                        m_data = static_cast<T**>(::operator new[](sizeof(T*) * size));
                        for (size_t i = 0; i < size; i++) {
                            try {
                                m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * row_size));
                                end = std::uninitialized_copy(Iter->begin(), Iter->end(), m_data[i]);
                                j = Iter->size();
                                if (j != row_size) {
                                    if (zero_exists) {
                                        err_end = std::uninitialized_fill_n(m_data[i] + j, row_size - j, math::helper::zero_vals.get_of<T>());
                                    }
                                    else if constexpr (std::is_default_constructible_v<T>) {
                                        err_end = std::uninitialized_value_construct_n(m_data[i] + j, row_size - j);
                                    }
                                    else throw std::logic_error("Cannot construct the matrix for this type because neither zero value is stored and neither is it default constructible.");
                                }
                            } catch(...) {
                                math::matrix::impl::destroy_data_continuous<T>(m_data, i, err_end, row_size);
                                throw;
                            }
                            ++Iter;
                        }
                        return;
                }
            }

        public:
            template <size_t C>
            Matrix(const T (*data)[C], const size_t num_rows) : m_order(math::matrix::Order(num_rows, C)) {
                T *end = nullptr;
                m_data = static_cast<T**>(::operator new[](sizeof(T*) * num_rows));
                for (size_t i = 0; i < num_rows; i++) {
                    try {
                        m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * C));
                        end = std::uninitialized_copy(data[i], data[i] + C, m_data[i]);
                    } catch(...) {
                        math::matrix::impl::destroy_data_continuous<T>(m_data, i, end, C);
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
                return *this;
            }

        public:
            ~Matrix() {
                const size_t row = m_order.row();
                const size_t col = m_order.column();
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    for (size_t i = 0; i < row; i++) {
                        std::destroy(m_data[i], m_data[i] + col);
                        ::operator delete[](m_data[i]);
                    }
                }
                else {
                    for (size_t i = 0; i < row; i++) {
                        ::operator delete[](m_data[i]);
                    }
                }
                ::operator delete[](m_data);
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
            math::matrix::Order order() const noexcept {
                return m_order;
            }
            size_t num_rows() const noexcept {
                return m_order.row();
            }
            size_t column_len() const noexcept {
                return m_order.row();
            }
            size_t num_columns() const noexcept {
                return m_order.column();
            }
            size_t row_len() const noexcept {
                return m_order.column();
            }
            size_t size() const noexcept {
                return m_order.size();
            }

        public:
            const T **data() const noexcept {
                return m_data;
            }
            math::matrix::Row<T> row(const size_t row) const {
                if (row >= m_order.row()) throw std::out_of_range("Cannot provide row object for the provided row number.");
                return math::matrix::Row<T>(m_data[row], m_order.column());
            }
            math::matrix::Row<T> operator[](const size_t row) const noexcept {
                return math::matrix::Row<T>(m_data[row], m_order.column());
            }
        
        public:
            bool is_square() const noexcept {
                return m_order.is_square();
            }
            bool is_row() const noexcept {
                return m_order.is_row();
            }
            bool is_column() const noexcept {
                return m_order.is_column();
            }
            bool is_same_dimension(const Matrix &other) const noexcept {
                return (m_order == other.m_order);
            }
            bool is_multipliable_dimension(const Matrix &other) const noexcept {
                return (m_order.column() == other.m_order.row());
            }
            bool is_opposite_dimension(const Matrix &other) const noexcept {
                return (m_order.transpose() == other.m_order);
            }
        
        public:
            Matrix &operator+=(const Matrix &other) requires math::helper::isAdditionPossible<T> {
                if (!is_same_dimension(other)) throw std::invalid_argument("Cannot add matrices of unequal order parameters.");
                const size_t num_rows = m_order.row();
                const size_t num_columns = m_order.column();
                #pragma omp parallel for
                for (size_t i = 0; i < num_rows; i++) {
                    T* first_row = m_data[i];
                    T* second_row = other.m_data[i];
                    #pragma omp parallel for
                    for (size_t j = 0; j < num_columns; j++) {
                        first_row[j] += second_row[j];
                    }
                }
                return *this;
            }

            Matrix operator+(const Matrix &other) const requires math::helper::isAdditionPossible<T> {
                Matrix temp(*this);
                temp += other;
                return temp;
            }

            Matrix &operator-=(const Matrix &other) requires math::helper::isSubtractionPossible<T> {
                if (!is_same_dimension(other)) throw std::invalid_argument("Cannot subtract matrices of unequal order parameters.");
                const size_t num_rows = m_order.row();
                const size_t num_columns = m_order.column();
                #pragma omp parallel for
                for (size_t i = 0; i < num_rows; i++) {
                    T* first_row = m_data[i];
                    T* second_row = other.m_data[i];
                    #pragma omp parallel for
                    for (size_t j = 0; j < num_columns; j++) {
                        first_row[j] -= second_row[j];
                    }
                }
                return *this;
            }

            Matrix operator-(const Matrix &other) const requires math::helper::isSubtractionPossible<T> {
                Matrix temp(*this);
                return (temp -= other);
            }

            Matrix &operator*=(const Matrix &other) requires math::helper::isMultiplicationPossible<T> && math::helper::isAdditionPossible<T> {
                *this = *this * other;
                return *this;
            }

            Matrix operator*(const Matrix &other) const requires math::helper::isMultiplicationPossible<T> && math::helper::isAdditionPossible<T> {
                if (!is_multipliable_dimension(other)) throw std::invalid_argument("Cannot multiply the matrices because the number of columns in first does not match the number of rows in the second.");
                Matrix result;
                if (m_order.is_zero()) return result;
                const size_t row = m_order.row();
                const size_t column = other.m_order.column();
                const size_t this_column = m_order.column();
                size_t d;
                result.m_order = math::matrix::Order(row, column);
                result.m_data = static_cast<T**>(::operator new[](sizeof(T*) * row));
                #pragma omp parallel for
                for (size_t i = 0; i < row; i++) {
                    try {
                        result.m_data[i] = static_cast<T*>(::operator new[](sizeof(T) * column));
                        const T &cached = m_data[i][0];
                        const T *const cache_data = other.m_data[0];
                        T *const data = result.m_data[i];
                        #pragma omp parallel for
                        for (d = 0; d < column; d++) {
                            std::construct_at(data + d, cached * cache_data[d]);
                        }
                    } catch(...) {
                        math::matrix::impl::destroy_data_continuous<T>(result.m_data, i, result.m_data[i] + d, column);
                        throw;
                    }
                }
                #pragma omp parallel for
                for (size_t i = 0; i < row; i++) {
                    T *const data = result.m_data[i];
                    #pragma omp parallel for
                    for (size_t k = 1; k < this_column; k++) {
                        const T &cached = m_data[i][k];
                        const T *const other_cached = other.m_data[k];
                        #pragma omp parallel for
                        for (size_t j = 0; j < column; j++) {
                            data[j] += cached * other_cached[j];
                        }
                    }
                }
                return result;
            }
        
        public:
            math::matrix::MatrixOneDIterator<T> begin_one_d() noexcept {
                return math::matrix::MatrixOneDIterator<T>(m_data, m_order.column());
            }
            math::matrix::MatrixOneDIterator<T> end_one_d() noexcept {
                return math::matrix::MatrixOneDIterator<T>(m_data, m_order.column(), m_order.size());
            }
            const math::matrix::MatrixOneDIterator<T> begin_one_d() const noexcept {
                return math::matrix::MatrixOneDIterator<T>(m_data, m_order.column());
            }
            const math::matrix::MatrixOneDIterator<T> end_one_d() const noexcept {
                return math::matrix::MatrixOneDIterator<T>(m_data, m_order.column(), m_order.size());
            }

            math::matrix::MatrixIterator<T> begin() noexcept {
                return math::matrix::MatrixIterator<T>(m_data, m_order.column());
            }
            math::matrix::MatrixIterator<T> end() noexcept {
                return math::matrix::MatrixIterator<T>(m_data + m_order.row(), m_order.column());
            }
            const math::matrix::MatrixIterator<T> begin() const noexcept {
                return math::matrix::MatrixIterator<T>(m_data, m_order.column());
            }
            const math::matrix::MatrixIterator<T> end() const noexcept {
                return math::matrix::MatrixIterator<T>(m_data + m_order.row(), m_order.column());
            }
    };
}