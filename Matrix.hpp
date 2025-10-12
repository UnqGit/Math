// Matrix.hpp
#pragma once

#include ".\impl\Headers.hpp"
#include ".\impl\Helper.hpp"
#include ".\impl\_matrix_impl_.hpp"
#include ".\impl\_memory_alloc_.hpp"

namespace math
{
    template <typename T>
    // Make your type no_throw_destructible first.
    requires ( std::is_nothrow_destructible_v<T> ) 
    class [[nodiscard("Discarding a Matrix construction - constructing a Matrix is an expensive operation.")]]
    Matrix {
        private:
            T **m_data = nullptr;
            math::matrix::Order m_order;

        public:
            Matrix() noexcept {}
            
            Matrix(const size_t size) : m_order(math::matrix::Order(size, size)) {
                if (m_order.is_zero()) return;
                const bool zero_exists = math::zero_vals.exists_of<T>();
                if constexpr (!std::is_default_constructible_v<T>) 
                    if (!zero_exists)
                        throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                m_data = math::memory::impl::allocate_memory<T*>(size);
                if (zero_exists) {
                    for (size_t i = 0; i < size; i++) {
                        math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                        math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], math::zero_vals.get_of<T>(), size, m_data, i, size);
                    }
                    return;
                }
                else if constexpr (std::is_default_constructible_v<T>) {
                    for (size_t i = 0; i < size; i++) {
                        math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                        math::memory::impl::mem_2d_safe_uninit_valcon_n_continuous<T>(m_data[i], size, m_data, i, size);
                    }
                    return;
                }
            }
            
            Matrix(const size_t size, const T &primary_value, const T &secondary_value, const math::matrix::ConstructSquareRule construct_rule)
            requires std::is_copy_constructible_v<T> : m_order(math::matrix::Order(size, size)) {
                if (m_order.is_zero()) return;
                switch(construct_rule) {
                    case math::matrix::CSR::right_half :
                        *this = Matrix(size, secondary_value, primary_value, math::matrix::CSR::left_half); return;
                    case math::matrix::CSR::lower_half :
                        *this = Matrix(size, secondary_value, primary_value, math::matrix::CSR::upper_half); return;
                    case math::matrix::CSR::bottom_right_triangle :
                        *this = Matrix(size, secondary_value, primary_value, math::matrix::CSR::top_left_triangle); return;
                    case math::matrix::CSR::bottom_left_triangle :
                        *this = Matrix(size, secondary_value, primary_value, math::matrix::CSR::top_right_triangle); return;
                    default: break;
                }
                m_data = math::memory::impl::allocate_memory<T*>(size);
                T *cache_data;
                bool is_primary = 0;
                for (size_t i = 0; i < size; i++) math::memory::impl::allocate_mem_2d_safe<T>(m_data, i, size);
                switch(construct_rule) {
                    case math::matrix::CSR::full :
                        for (size_t i = 0; i < size; i++) 
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, size, m_data, i, size, size);
                        return;
                    case math::matrix::CSR::upper_half :
                        for (size_t i = 0; i < (size>>1); i++) 
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, size, m_data, i, size, size);
                        for (size_t i = (size>>1); i < size; i++)
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                        return;
                    case math::matrix::CSR::left_half :
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, (size>>1), m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), secondary_value, (size>>1) + (size & 1), m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::top_left_triangle :
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, size - i, m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + size - i, secondary_value, i, m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::top_right_triangle :
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, i, m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + i, primary_value, size - i, m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::main_diagonal :
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, i, m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_construct_at<T>(m_data[i] + i, m_data, i, size, size, primary_value);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + i + 1, secondary_value, size - i - 1, m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::off_diagonal :
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size - 1 - i, m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_construct_at<T>(m_data[i] + size - 1 - i, m_data, i, size, size, primary_value);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + size - i, secondary_value, size - i, m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::top_left_quarter :
                        for (size_t i = 0; i < (size>>1); i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, (size>>1), m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), secondary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                        }
                        for (size_t i = (size>>1); i < size; i++)
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                        return;
                    case math::matrix::CSR::top_right_quarter :
                        for (size_t i = 0; i < (size>>1); i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, (size>>1), m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), primary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                        }
                        for (size_t i = (size>>1); i < size; i++)
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                        return;
                    case math::matrix::CSR::bottom_left_quarter :
                        for (size_t i = 0; i < (size>>1); i++)
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                        for (size_t i = (size>>1); i < size; i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, (size>>1), m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), secondary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::bottom_right_quarter :
                        for (size_t i = 0; i < (size>>1); i++)
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                        for (size_t i = (size>>1); i < size; i++) {
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, (size>>1), m_data, i, size, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), primary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::alternate :
                        for (size_t i = 0; i < size; i++) {
                            is_primary = (i % 2 == 0);
                            cache_data = m_data[i];
                            for (size_t j = 0; j < size; j++) {
                                math::memory::impl::mem_2d_safe_construct_at<T>(cache_data + j, m_data, i, size, size, is_primary ? primary_value : secondary_value);
                                is_primary = !is_primary;
                            }
                        }
                        return;
                    case math::matrix::CSR::alternate_row :
                        for (size_t i = 0; i < size; i++) {
                            is_primary = (i % 2 == 0);
                            math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], is_primary ? primary_value : secondary_value, size, m_data, i, size, size);
                        }
                        return;
                    case math::matrix::CSR::alternate_column :
                        for (size_t i = 0; i < size; i++) {
                            cache_data = m_data[i];
                            is_primary = true;
                            for (size_t j = 0; j < size; j++) {
                                math::memory::impl::mem_2d_safe_construct_at<T>(cache_data + j, m_data, i, size, size, is_primary ? primary_value : secondary_value);
                                is_primary = !is_primary;
                            }
                        }
                        return;
                    default: break;
                }
            }
            
            Matrix(const size_t size, const T &primary_value, const math::matrix::ConstructSquareRule construct_rule) 
            requires std::is_copy_constructible_v<T> {
                if (size == 0) return;
                const bool zero_exists = math::zero_vals.exists_of<T>();
                if (zero_exists) {
                    *this = Matrix(size, primary_value, math::zero_vals.get_of<T>(), construct_rule);
                    return;
                }
                else if constexpr (std::is_default_constructible_v<T>) {
                    *this = Matrix(size, primary_value, T{}, construct_rule);
                    return;
                }
                else throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            }

        public:
            Matrix(const math::matrix::Order &order, const math::matrix::ConstructAllocateRule construct_rule = math::matrix::CAR::zero) : m_order(order) {
                if (m_order.is_zero()) return;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                const bool zero_exists = math::zero_vals.exists_of<T>();
                if (construct_rule == math::matrix::CAR::possible_garbage) {
                    if constexpr (!(std::is_trivially_constructible_v<T> || std::is_default_constructible_v<T>))
                        if (!zero_exists)
                            throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                    m_data = math::memory::impl::allocate_memory<T*>(row);
                    for (size_t i = 0; i < row; i++) math::memory::impl::allocate_mem_2d_safe<T>(m_data, i, column);
                    if constexpr (std::is_trivially_constructible_v<T>) return;
                    else if constexpr (std::is_default_constructible_v<T>) {
                        for (size_t i = 0; i < row; i++)
                            math::memory::impl::mem_2d_safe_uninit_valcon_n<T>(m_data[i], column, m_data, i, row, column);
                        return;
                    }
                    for (size_t i = 0; i < row; i++)
                        math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], math::zero_vals.get_of<T>(), column, m_data, i, row, column);
                }
                else {
                    if (!zero_exists)
                        throw std::logic_error("The zero value is not stored of this type in math::zero_vals hence can't zero construct the Matrix.");
                    m_data = math::memory::impl::allocate_memory<T*>(row);
                    for (size_t i = 0; i < row; i++) math::memory::impl::allocate_mem_2d_safe<T>(m_data, i, column);
                    for (size_t i = 0; i < row; i++)
                        math::memory::impl::mem_2d_safe_uninit_fill_n<T>(m_data[i], math::zero_vals.get_of<T>(), column, m_data, i, row, column);
                }
            }
            Matrix(const size_t row, const size_t column, const math::matrix::ConstructAllocateRule construct_rule = math::matrix::CAR::zero) : Matrix(math::matrix::Order(row, column), construct_rule) {}

        public:
            Matrix(const math::matrix::Order &order, const T &to_copy)
            requires std::is_copy_constructible_v<T> : m_order(order) {
                if (m_order.is_zero()) return;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                m_data = math::memory::impl::allocate_memory<T*>(row);
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], to_copy, column, m_data, i, column);
                }
            }
            
            Matrix(const size_t row, const size_t column, const T &to_copy)
            requires std::is_copy_constructible_v<T> : Matrix(math::matrix::Order(row, column), to_copy) {}
            
        public:
            Matrix(T *data, const size_t size, math::matrix::ConstructOrientationRule construct_rule, const T &fallback_val)
            requires std::is_copy_constructible_v<T> {
                if (size == 0) return;
                switch (construct_rule) {
                    case math::matrix::COR::horizontal :
                        m_order = math::matrix::Order(1, size);
                        m_data = math::memory::impl::allocate_memory<T*>(1);
                        math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                        math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[0], size, data, m_data, 0, size);
                        return;
                    case math::matrix::COR::vertical :
                        m_order = math::matrix::Order(size, 1);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                            math::memory::impl::mem_2d_safe_construct_at_continuous<T>(m_data[i], m_data, i, size, data[i]);
                        }
                        return;
                    case math::matrix::COR::main_diagonal :
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        m_order = math::matrix::Order(size, size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, i, m_data, i, size);
                            math::memory::impl::mem_2d_safe_construct_at_continuous<T>(m_data[i] + i, m_data, i, size, data[i]);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + i + 1, fallback_val, size - i - 1, m_data, i, size);
                        }
                        return;
                    case math::matrix::COR::off_diagonal :
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        m_order = math::matrix::Order(size, size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, size - i - 1, m_data, i, size);
                            math::memory::impl::mem_2d_safe_construct_at_continuous<T>(m_data[i] + size - i - 1, m_data, i, size, data[i]);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + size - i, fallback_val, i, m_data, i, size);
                        }
                        return;
                }
            }

            Matrix(T *data, const size_t size, math::matrix::ConstructOrientationRule construct_rule = math::matrix::COR::horizontal)
            requires std::is_copy_constructible_v<T> {
                if (size == 0) return;
                const bool zero_exists = (math::zero_vals.exists_of<T>());
                switch (construct_rule) {
                    case math::matrix::COR::horizontal :
                        m_order = math::matrix::Order(1, size);
                        m_data = math::memory::impl::allocate_memory<T*>(1);
                        math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                        math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[0], size, data, m_data, 0, size);
                        return;
                    case math::matrix::COR::vertical :
                        m_order = math::matrix::Order(size, 1);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                            math::memory::impl::mem_2d_safe_construct_at_continuous<T>(m_data[i], m_data, i, size, data[i]);
                        }
                        return;
                    case math::matrix::COR::main_diagonal :
                    case math::matrix::COR::off_diagonal :
                        if constexpr (!std::is_default_constructible_v<T>)
                            if (!zero_exists)
                                throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                        if constexpr (!std::is_default_constructible_v<T>) *this = Matrix(data, size, construct_rule, math::zero_vals.get_of<T>());
                        else *this = Matrix(data, size, construct_rule, T{});
                        return;
                }
            }
            
            Matrix(T *data, const math::matrix::Order &order)
            requires std::is_copy_constructible_v<T> : m_order(order) {
                if (m_order.size()) return;
                const size_t row = order.row();
                const size_t column = order.column();
                m_data = math::memory::impl::allocate_memory<T*>(row);
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], column, data + i * column, m_data, i, column);
                }
            }
            
            Matrix(T *data, const size_t row, const size_t column)
            requires std::is_copy_constructible_v<T> : Matrix(data, math::matrix::Order(row, column)) {}

        public:
            Matrix(T *data, const size_t size, const math::matrix::Order &order, const T &fallback_val)
            requires std::is_copy_constructible_v<T> : m_order(order) {
                if (m_order.is_zero()) return;
                if (size >= m_order.size()) {
                    *this = Matrix(data, m_order);
                    return;
                }
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                size_t constructed_items = 0;
                m_data = math::memory::impl::allocate_memory<T*>(row);
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    bool is_remaining = size > constructed_items;
                    switch(is_remaining) {
                        case true:
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], std::min(size - constructed_items, column), data + constructed_items, m_data, i, column);
                            constructed_items += std::min(size - constructed_items, column);
                            if (constructed_items % column == 0) break;
                            [[fallthrough]];
                        case false:
                            if (constructed_items % column)
                                math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + (constructed_items % column), fallback_val, constructed_items % column, m_data, i, column);
                            else math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, column, m_data, i, column);
                            constructed_items += ((constructed_items % column) == 0 ? column : (constructed_items % column));
                    }
                }
            }

            Matrix(T *data, const size_t size, const math::matrix::Order &order)
            requires std::is_copy_constructible_v<T> : m_order(order) {
                if (m_order.is_zero()) return;
                if (size >= m_order.size()) {
                    *this = Matrix(data, m_order);
                    return;
                }
                if (math::zero_vals.exists_of<T>()) {
                    *this = Matrix(data, size, order, math::zero_vals.get_of<T>());
                    return;
                }
                else if constexpr (std::is_default_constructible_v<T>) {
                    *this = Matrix(data, size, order, T{});
                    return;
                }
                throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            }
            
            Matrix(T *data, const size_t size, const size_t row, const size_t column, const T &fallback_val)
            requires std::is_copy_constructible_v<T> : Matrix(data, size, math::matrix::Order(row, column), fallback_val) {}
            
            Matrix(T *data, const size_t size, const size_t row, const size_t column)
            requires std::is_copy_constructible_v<T> : Matrix(data, size, math::matrix::Order(row, column)) {} 
           
        public:
            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U& arr, const math::matrix::ConstructOrientationRule construct_rule, const T &fallback_val)
            requires std::is_copy_constructible_v<T> {
                const size_t size = arr.size();
                if (size == 0) return;
                auto Iter = arr.begin();
                switch (construct_rule) {
                    case math::matrix::COR::horizontal :
                        m_order = math::matrix::Order(1, size);
                        m_data = math::memory::impl::allocate_memory<T*>(1);
                        math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                        math::memory::impl::mem_2d_safe_uninit_copy_continuous<T>(m_data[0], arr.begin(), arr.end(), m_data, 0, size);
                        return;
                    case math::matrix::COR::vertical :
                        m_order = math::matrix::Order(size, 1);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], 1, Iter, m_data, i, 1); // This instead of math::memory::impl::mem_2d_safe_construct_at_continuous because we would have to pass *Iter as an argument but if the dereferencing throws an exception, it would leak memory.
                        }
                        return;
                    case math::matrix::COR::main_diagonal :
                        m_order = math::matrix::Order(size, size);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, i, m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i] + i, 1, Iter, m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + i + 1, fallback_val, size - i - 1, m_data, i, size);
                            if constexpr ( !noexcept( ++std::declval<std::decay_t<decltype(Iter)>>() )) try {++Iter;} _CATCH_MEM_ERR_CONT_(m_data, i + 1, size)
                            else ++Iter;
                        }
                        return;
                    case math::matrix::COR::off_diagonal :
                        m_order = math::matrix::Order(size, size);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, size - i - 1, m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i] + size - 1 - i, 1, Iter, m_data, i, size);
                            math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + size - i, fallback_val, i, m_data, i, size);
                            if constexpr ( !noexcept( ++std::declval<std::decay_t<decltype(Iter)>>() )) try {++Iter;} _CATCH_MEM_ERR_CONT_(m_data, i + 1, size)
                            else ++Iter;
                        }
                        return;
                }
            }

            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, const math::matrix::ConstructOrientationRule construct_rule = math::matrix::COR::horizontal)
            requires std::is_copy_constructible_v<T> {
                const size_t size = arr.size();
                if (size == 0) return;
                auto Iter = arr.begin();
                const bool zero_exists = math::zero_vals.exists_of<T>();
                switch (construct_rule) {
                    case math::matrix::COR::horizontal :
                        m_order = math::matrix::Order(1, size);
                        m_data = math::memory::impl::allocate_memory<T*>(1);
                        math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                        math::memory::impl::mem_2d_safe_uninit_copy_continuous<T>(m_data[0], arr.begin(), arr.end(), m_data, 0, size);
                        return;
                    case math::matrix::COR::vertical :
                        m_order = math::matrix::Order(size, 1);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], 1, Iter, m_data, i, 1); // This instead of math::memory::impl::mem_2d_safe_construct_at_continuous because we would have to pass *Iter as an argument but if the dereferencing throws an exception, it would leak memory.
                        }
                        return;
                    case math::matrix::COR::main_diagonal :
                    case math::matrix::COR::off_diagonal :
                        if (!std::is_default_constructible_v<T>)
                            if (!zero_exists)
                                throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                        if (!std::is_default_constructible_v<T>) *this = Matrix(arr, construct_rule, math::zero_vals.get_of<T>());
                        else *this = Matrix(arr, construct_rule, T{});
                }
            }
            
            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, const math::matrix::Order &order, const T &fallback_val)
            requires std::is_copy_constructible_v<T> : m_order(order) {
                if (m_order.is_zero()) return;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                const size_t size = arr.size();
                auto Iter = arr.begin();
                size_t j = 0, constructed_items = 0, k;
                m_data = math::memory::impl::allocate_memory<T*>(row);
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    bool is_remaining = (constructed_items < size);
                    switch (is_remaining) {
                        case true :
                            j = std::min(size - constructed_items, column);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], j, Iter, m_data, i, column);
                            constructed_items += j;
                            if (constructed_items % column == 0) break;
                            [[fallthrough]];
                        case false :
                            if (constructed_items % column == 0) math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, column, m_data, i, column);
                            else math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + j, fallback_val, column - j, m_data, i, column);
                    }
                }
            }

            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, const size_t row, const size_t column, const T &fallback_val)
            requires std::is_copy_constructible_v<T> : Matrix(arr, math::matrix::Order(row, column), fallback_val) {}

            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, const math::matrix::Order &order)
            requires std::is_copy_constructible_v<T> : m_order(order) {
                if (m_order.is_zero()) return;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                const size_t size = arr.size();
                const bool zero_exists = math::zero_vals.exists_of<T>();
                size_t k;
                if (size < order.size()) {
                    if constexpr (!std::is_default_constructible_v<T>)
                        if (!zero_exists)
                            throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                    if constexpr (!std::is_default_constructible_v<T>) *this = Matrix(arr, order, math::zero_vals.get_of<T>());
                    else *this = Matrix(arr, order, T{});
                    return;
                }
                auto Iter = arr.begin();
                m_data = math::memory::impl::allocate_memory<T*>(row);
                size_t j = 0;
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], column, Iter, m_data, i, column);
                }
            }
            
            template <typename U>
            requires math::helper::isOneDArr<U, T>
            Matrix(const U &arr, const size_t row, const size_t column)
            requires std::is_copy_constructible_v<T> : Matrix(arr, math::matrix::Order(row, column)) {}

        public:
            Matrix(T **data, const math::matrix::Order &order)
            requires std::is_copy_constructible_v<T> : m_order(order) {
                if (m_order.is_zero()) return;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                m_data = math::memory::impl::allocate_memory<T*>(row);
                T* end = nullptr;
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], column, data[i], m_data, i, column);
                }
            }
            
            Matrix(T **data, const size_t row, const size_t column)
            requires std::is_copy_constructible_v<T> : Matrix(data, math::matrix::Order(row, column)) {}
            
        public:
            template <typename U>
            requires math::helper::isTwoDArr<U, T>
            Matrix(const U &arr, const math::matrix::ConstructContainerRule construct_rule = math::matrix::CCR::must_be_same)
            requires std::is_copy_constructible_v<T> {
                const size_t size = arr.size();
                if (size == 0) return;
                if (size == 1) {
                    *this = Matrix(*(arr.begin()), math::matrix::COR::horizontal);
                    return;
                }
                size_t row_size;
                auto Iter = arr.begin();
                auto end = arr.end();
                const bool zero_exists = math::zero_vals.exists_of<T>();
                const size_t row_objects_created = 0;
                switch (construct_rule) {
                    case math::matrix::CCR::shrink :
                        row_size = (*Iter).size();
                        while (Iter != end) {
                            row_size = std::min(row_size, (*Iter).size());
                            if (row_size == 0) return;
                            ++Iter;
                        }
                        m_order = math::matrix::Order(size, row_size);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        Iter = arr.begin();
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], row_size, Iter, m_data, i, row_size);
                        }
                        return;
                    case math::matrix::CCR::must_be_same :
                        row_size = (*Iter).size();
                        while (Iter != end) {
                            if (row_size != (*Iter).size()) throw std::invalid_argument("Rows of the provided container are not of the same size and rule chosen is math::matrix::ConstructContainerRule::must_be_same.");
                            ++Iter;
                        }
                        m_order = math::matrix::Order(size, row_size);
                        if (row_size == 0) return;
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        Iter = arr.begin();
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], row_size, Iter, m_data, i, row_size);
                        }
                        return;
                    case math::matrix::CCR::expand :
                        row_size = (*Iter).size();
                        bool are_all_same = true;
                        size_t temp;
                        while (Iter != end) {
                            temp = std::max(row_size, (*Iter).size());
                            if (temp != row_size) are_all_same = false;
                            row_size = temp;
                            ++Iter;
                        }
                        if (row_size == 0) return;
                        Iter = arr.begin();
                        if (!are_all_same)
                            if constexpr (!std::is_default_constructible_v<T>)
                                if (!zero_exists) throw std::invalid_argument("Cannot construct the Matrix because the tag set was math::matrix::ConstructContainerRule::expand and all rows were not of the same size and the type is neither default constructible and neither is it's zero value stored in math::zero_vals.");
                        m_order = math::matrix::Order(size, row_size);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                            row_objects_created = math::memory::impl::mem_2d_safe_uninit_copy_continuous<T>(m_data[i], (*Iter).begin(), (*Iter).end(), m_data, i, row_size);
                            if (row_objects_created != row_size) {
                                if constexpr (!std::is_default_constructible_v<T>) 
                                    math::memory::impl::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + row_objects_created, math::zero_vals.get_of<T>(), row_size - row_objects_created, m_data, i, row_size);
                                else math::memory::impl::mem_2d_safe_uninit_valcon_n_continuous<T>(m_data[i] + row_objects_created, row_size - row_objects_created, m_data, i, row_size);
                            }
                            ++Iter;
                        }
                        return;
                    case math::matrix::CCR::are_same :
                        row_size = (*Iter).size();
                        if (row_size == 0) return;
                        m_order = math::matrix::Order(size, row_size);
                        m_data = math::memory::impl::allocate_memory<T*>(size);
                        for (size_t i = 0; i < size; i++) {
                            if ((*Iter).size() != row_size) {
                                math::memory::impl::destroy_data_mem_err_continuous<T>(m_data, i, row_size);
                                throw std::logic_error("Promised attribute math::matrix::ConstructConainerRule::are_same was not satisfied in construction of the Matrix.");
                            }
                            m_data[i] = math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                            math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], row_size, Iter, m_data, i, row_size);
                            ++Iter;
                        }
                        return;
                }
            }

        public:
            template <size_t C>
            Matrix(const T (*data)[C], const size_t num_rows)
            requires std::is_copy_constructible_v<T> : m_order(math::matrix::Order(num_rows, C)) {
                if (m_order.is_zero()) return;
                T *end = nullptr;
                m_data = math::memory::impl::allocate_memory<T*>(num_rows);
                for (size_t i = 0; i < num_rows; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, C);
                    math::memory::impl::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], C, data[i], m_data, i, C);
                }
            }

        public:
            Matrix(const math::matrix::Order &order, const std::function<T()> &t_creation)
            requires (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>) : m_order(order) {
                if (m_order.is_zero()) return;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                size_t j;
                m_data = math::memory::impl::allocate_memory<T*>(row);
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    if constexpr ( noexcept(t_creation) ) { for (j = 0; j < column; j++) std::construct_at(m_data[i] + j, t_creation()); }
                    else _TRY_CONSTRUCT_AT_LOOP_(J, (J < column), (j++), m_data[i], t_creation()) _CATCH_DES_DATA_CONT_(m_data, i, j, column)
                }
            }
            
            Matrix(const size_t row, const size_t column, const std::function<T()> &t_creation)
            requires (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>) : Matrix(math::matrix::Order(row, column), t_creation) {}

            Matrix(const math::matrix::Order &order, const std::function<T(size_t, size_t)> &t_creation)
            requires (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>) : m_order(order) {
                if (m_order.is_zero()) return;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                size_t j;
                m_data = math::memory::impl::allocate_memory<T*>(row);
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                    if constexpr ( noexcept(t_creation) ) { for (j = 0; j < column; j++) std::construct_at(m_data[i] + j, t_creation(i, j)); }
                    else _TRY_CONSTRUCT_AT_LOOP_(J, (J < column), (j++), m_data[i], t_creation(i, j)) _CATCH_DES_DATA_CONT_(m_data, i, j, column)
                }
            }
            
            Matrix(const size_t row, const size_t column, const std::function<T(size_t, size_t)> &t_creation)
            requires (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>) : Matrix(math::matrix::Order(row, column), t_creation) {}

        public:
            Matrix(const Matrix &other) : Matrix(other.m_data, other.m_order) {} // Delegate to Matrix(T** data, const math::matrix::Order &order) {} constructor.
            Matrix(Matrix &&other) noexcept {
                this->swap(other); // Default value for data members for this is there 0 state, so now 'other' is in 0 state.
            }
            Matrix &operator=(const Matrix &other) {
                if (this != &other) {
                    Matrix temp(other);
                    this->swap(temp);
                }
                return *this;
            }
            Matrix &operator=(Matrix &&other) noexcept {
                if (this != &other) this->swap(other);
                return *this;
            }

        public:
            ~Matrix() noexcept {
                if (m_data != nullptr) math::memory::impl::destroy_data_mem_err_continuous<T>(m_data, m_order.row(), m_order.column());
            }
            void reset() noexcept {
                *this = Matrix{}; // Call the default constructor.
            }

        public:
            [[nodiscard("Result of (non-const)T &operator() was ignored.")]]
            T &operator()(const size_t row, const size_t column) noexcept {
                return m_data[row][column];
            }
            const T &operator()(const size_t row, const size_t column) const noexcept {
                return m_data[row][column];
            }
            
            [[nodiscard("Result of (non-const)at method was ignored.")]]
            T &at(const size_t row, const size_t column) {
                if ((row >= m_order.row()) || (column >= m_order.column())) throw std::out_of_range("Provided index does not exist within the bounds of this Matrix.");
                return m_data[row][column];
            }
            const T &at(const size_t row, const size_t column) const {
                if ((row >= m_order.row()) || (column >= m_order.column())) throw std::out_of_range("Provided index does not exist within the bounds of this Matrix.");
                return m_data[row][column];
            }

        public:
            void swap(Matrix &other) noexcept {
                m_order.swap(other.m_order);
                std::swap(m_data, other.m_data);
            }

        public:
            [[nodiscard("Result of order method was ignored.")]]
            math::matrix::Order order() const noexcept {
                return m_order;
            }
            
            [[nodiscard("Result of num_rows method was ignored.")]]
            size_t num_rows() const noexcept {
                return m_order.row();
            }
            
            [[nodiscard("Result of column_len method was ignored.")]]
            size_t column_len() const noexcept {
                return m_order.row();
            }
            
            [[nodiscard("Result of num_columns method was ignored.")]]
            size_t num_columns() const noexcept {
                return m_order.column();
            }
            
            [[nodiscard("Result of row_len method was ignored.")]]
            size_t row_len() const noexcept {
                return m_order.column();
            }
            
            [[nodiscard("Result of size method was ignored.")]]
            size_t size() const noexcept {
                return m_order.size();
            }

        public:
            const T **const data() const noexcept {
                return m_data;
            }
            T *const operator[](const size_t row) noexcept {
                return m_data[row];
            }
            const T *const operator[] (const size_t row) const noexcept {
                return m_data[row];
            }
            math::matrix::Row<T> row(const size_t row) const {
                if (row >= m_order.row()) throw std::out_of_range("Cannot provide row object for the provided row number.");
                return math::matrix::Row<T>(m_data[row], m_order.column());
            }
            math::matrix::Column<T> column(const size_t col) const {
                if (col >= m_order.column()) throw std::out_of_range("Cannot access the column on the provided index as it exceeds the number of columns present in the matrix.");
                return math::matrix::Column<T>(m_data, col, m_order.row());
            }
            math::matrix::Row<T> row(const size_t row) {
                if (row >= m_order.row()) throw std::out_of_range("Cannot provide row object for the provided row number.");
                return math::matrix::Row<T>(m_data[row], m_order.column());
            }
            math::matrix::Column<T> column(const size_t col) {
                if (col >= m_order.column()) throw std::out_of_range("Cannot access the column on the provided index as it exceeds the number of columns present in the matrix.");
                return math::matrix::Column<T>(m_data, col, m_order.row());
            }

        public:
            [[nodiscard("Result of is_square method was ignored.")]]
            bool is_square() const noexcept {
                return m_order.is_square();
            }
            
            [[nodiscard("Result of is_row method was ignored.")]]
            bool is_row() const noexcept {
                return m_order.is_row();
            }
            
            [[nodiscard("Result of is_column method was ignored.")]]
            bool is_column() const noexcept {
                return m_order.is_column();
            }
            
            [[nodiscard("Result of is_tall method was ignored.")]]
            bool is_tall() const noexcept {
                return m_order.is_tall();
            }
            
            [[nodiscard("Result of is_wide method was ignored.")]]
            bool is_wide() const noexcept {
                return m_order.is_wide();
            }
            
            [[nodiscard("Result of is_same_dimension(const Matrix&) method was ignored.")]]
            bool is_same_dimension(const Matrix &other) const noexcept {
                return (m_order == other.m_order);
            }
            
            [[nodiscard("Result of is_multipliable_dimension(const Matrix&) method was ignored.")]]
            bool is_multipliable_dimension(const Matrix &other) const noexcept {
                return (m_order.column() == other.m_order.row());
            }
            
            [[nodiscard("Result of is_opposite_dimension(const Matrix&) method was ignored.")]]
            bool is_opposite_dimension(const Matrix &other) const noexcept {
                return (m_order.transpose() == other.m_order);
            }
        
        public:
            Matrix &operator+=(const Matrix &other)
            requires math::isAdditionPossible<T> {
                if (!is_same_dimension(other)) throw std::invalid_argument("Cannot add matrices of unequal order parameters.");
                const size_t num_rows = m_order.row();
                const size_t num_columns = m_order.column();
                #pragma omp parallel for collapse(2) schedule(static)
                for (size_t i = 0; i < num_rows; i++) {
                    for (size_t j = 0; j < num_columns; j++) {
                        m_data[i][j] += other.m_data[i][j];
                    }
                }
                return *this;
            }

            [[nodiscard("Result of + operator method was ignored.")]]
            Matrix operator+(const Matrix &other) const
            requires math::isAdditionPossible<T> {
                Matrix temp(*this);
                temp += other;
                return temp;
            }

            Matrix &operator-=(const Matrix &other)
            requires math::isSubtractionPossible<T> {
                if (!is_same_dimension(other)) throw std::invalid_argument("Cannot subtract matrices of unequal order parameters.");
                const size_t num_rows = m_order.row();
                const size_t num_columns = m_order.column();
                #pragma omp parallel for collapse(2) schedule(static)
                for (size_t i = 0; i < num_rows; i++) {
                    for (size_t j = 0; j < num_columns; j++) {
                        m_data[i][j] -= other.m_data[i][j];
                    }
                }
                return *this;
            }

            [[nodiscard("Result of - operator method was ignored.")]]
            Matrix operator-(const Matrix &other) const
            requires math::isSubtractionPossible<T> {
                Matrix temp(*this);
                return (temp -= other);
            }

            Matrix &operator*=(const Matrix &other)
            requires math::isMultiplicationPossible<T> && math::isAdditionPossible<T> {
                *this = *this * other;
                return *this;
            }

            [[nodiscard("Result of * operator_ method was ignored.")]]
            Matrix operator*(const Matrix &other) const
            requires math::isMultiplicationPossible<T> && math::isAdditionPossible<T> {
                if (!is_multipliable_dimension(other)) throw std::invalid_argument("Cannot multiply the matrices because the number of columns in first does not match the number of rows in the second.");
                Matrix result;
                if (m_order.is_zero()) return result;
                const size_t row = m_order.row();
                const size_t column = other.m_order.column();
                const size_t this_column = m_order.column();
                T *to_transfer = math::memory::impl::allocate_memory<T*>(row);
                size_t d = 0;
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(to_transfer, i, column);
                    const T &cached = m_data[i][0];
                    const T *const cache_data = other.m_data[0];
                    T *const data = to_transfer[i];
                    if constexpr ( noexcept(std::declval<T>() * std::declval<T>()) ) for (d = 0; d < column; d++) std::construct_at(data + d, cached * cache_data[d]);
                    else _TRY_CONSTRUCT_AT_LOOP_(d, (d < column), (d++), data, cached * cache_data[d]) _CATCH_DES_DATA_CONT_(to_transfer, i, d, column)
                }
                // it is fine till here if an exception is called and the destructor of result is called because the order is zero and hence it wouldn't try to free memory.
                std::swap(result.m_data, to_transfer); // m_data was nullptr before this.
                result.m_order = math::matrix::Order(row, column);
                if (result.is_wide()) {
                    for (size_t i = 0; i < row; i++) {
                        T *const data = result.m_data[i];
                        for (size_t k = 1; k < this_column; k++) {
                            const T &cached = m_data[i][k];
                            const T *const other_cached = other.m_data[k];
                            #pragma omp parallel for
                            for (size_t j = 0; j < column; j++) {
                                data[j] += cached * other_cached[j];
                            }
                        }
                    }
                }
                else {
                    #pragma omp parallel for collapse(2)
                    for (size_t i = 0; i < row; i++) {
                        for (size_t k = 1; k < this_column; k++) {
                            const T &cached = m_data[i][k];
                            const T *other_cached = other.m_data[k];
                            T *const data = result.m_data[i];
                            for (size_t j = 0; j < column; j++) {
                                data[j] += cached * other_cached[j];
                            }
                        }
                    }
                }
                return result;
            }

        public:
            // Row one dimension iterators.
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

            // Column one dimension iterators.
            math::matrix::MatrixOneDColumnIterator<T> begin_c_one_d() noexcept {
                return math::matrix::MatrixOneDColumnIterator<T>(m_data, m_order.column());
            }
            math::matrix::MatrixOneDColumnIterator<T> end_c_one_d() noexcept {
                return math::matrix::MatrixOneDColumnIterator<T>(m_data, m_order.column(), m_order.size());
            }
            const math::matrix::MatrixOneDColumnIterator<T> begin_c_one_d() const noexcept {
                return math::matrix::MatrixOneDColumnIterator<T>(m_data, m_order.column());
            }
            const math::matrix::MatrixOneDColumnIterator<T> end_c_one_d() const noexcept {
                return math::matrix::MatrixOneDColumnIterator<T>(m_data, m_order.column(), m_order.size());
            }

            // Iterators which provide row view object for each row.
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

        public:
            [[nodiscard("Result of transpose method was ignored.")]]
            Matrix transpose() const
            requires std::is_copy_constructible_v<T> {
                Matrix result;
                if (m_order.is_zero()) return result;
                const size_t row = m_order.column();
                const size_t column = m_order.row();
                T *to_transfer = math::memory::impl::allocate_memory<T*>(row);
                size_t j;
                for (size_t i = 0; i < row; i++) {
                    math::memory::impl::allocate_mem_2d_safe_continuous<T>(to_transfer, i, column);
                    T *data = to_transfer[i];
                    for (j = 0; j < column; j++)
                        math::memory::impl::mem_2d_safe_construct_at_continuous<T>(data + j, m_data, i, column, m_data[j][i]);
                }
                std::swap(result.m_data, to_transfer); // Automatically sets to_transfer to nullptr.
                result.m_order = m_order.transpose();
                return result;
            }

            Matrix &transpose_in_place()
            requires (std::is_copy_constructible_v<T> || std::is_nothrow_swappable_v<T>) {
                if (m_order.is_zero()) return *this;
                if constexpr (std::is_nothrow_swappable_v<T>) {
                    if (m_order.is_square()) {
                        const size_t size = m_order.row();
                        for (size_t i = 0; i < size; i++) {
                            T *const data = m_data[i];
                            #pragma omp parallel for schedule(static)
                            for (size_t j = i + 1; j < size; j++) {
                                std::swap(data[j], m_data[j][i]);
                            }
                        }
                    }
                    else if constexpr (std::is_copy_constructible_v<T>) *this = this->transpose();
                    else throw std::logic_error("Cannot do transposition on this Matrix because it is neither a square Matrix and is not copy constructible to create a new Matrix.");
                }
                else if constexpr (std::is_copy_constructible_v<T>) *this = this->transpose();
                else throw std::logic_error("Cannot do transposition on this Matrix because it is neither a square Matrix(or the type is not swappable) and is not copy constructible to create a new Matrix.");
                return *this;
            }
    
        public:
            [[nodiscard("Result of trace method was ignored.")]]
            T trace() const
            requires math::isAdditionPossible<T> {
                if (!(this->is_square())) throw std::logic_error("Cannot find trace of a non square Matrix.");
                const bool zero_exists = math::zero_vals.exists_of<T>();
                if (m_order.is_zero()) {
                    if (zero_exists) {
                        return math::zero_vals.get_of<T>();
                    }
                    else if constexpr (std::is_default_constructible_v<T>) {
                        return T{};
                    }
                    else throw std::logic_error("Cannot provide the value of trace for a zero size Matrix with a type of which neither a default constructor exists nor is the zero value stored.");
                }
                const size_t size = m_order.row();
                if constexpr (std::is_copy_constructible_v<T>) {
                    if (zero_exists) {
                        T result(math::zero_vals.get_of<T>());
                        for (size_t i = 0; i < size; i++) {
                            result += m_data[i][i];
                        }
                        return result;
                    }
                    else {
                        T result(m_data[0][0]);
                        for (size_t i = 1; i < size; i++) {
                            result += m_data[i][i];
                        }
                        return result;
                    }
                }
                else if constexpr (std::is_default_constructible_v<T>) {
                    T result{};
                    for (size_t i = 0; i < size; i++) {
                        result += m_data[i][i];
                    }
                    return result;
                }
                else throw std::logic_error("Cannot provide the trace of the Matrix because it is not copy constructible for storing initial zero/default value and is neither default constructible.");
            }

            [[nodiscard("Result of is_zero method was ignored.")]]
            bool is_zero() const
            requires math::isEqualityOperationPossible<T> {
                const bool zero_exists = math::zero_vals.exists_of<T>();
                if (zero_exists) {
                    const T &to_check_from = math::zero_vals.get_of<T>();
                    for (const auto &row : *this) {
                        for (const auto &elem : row) {
                            if (!math::is_equal(to_check_from, elem)) return false;
                        }
                    }
                    return true;
                }
                else if constexpr (std::is_default_constructible_v<T>) {
                    T to_check_from{};
                    for (const auto &row : *this) {
                        for (const auto &elem : row) {
                            if (!math::is_equal(to_check_from, elem)) return false;
                        }
                    }
                    return true;
                }
                else throw std::logic_error("Cannot check for is_zero property of the Matrix as the zero value(stored in math::zero_vals or defautlt construction for the type) is not defined.");
            }
            
            [[nodiscard("Result of are_all_same_as(const T&) method was ignored.")]]
            bool are_all_same_as(const T &to_check_from) const
            requires math::isEqualityOperationPossible<T> {
                for (const auto &row : *this) {
                    for (const auto &elem : row) {
                        if (!math::is_equal(to_check_from, elem)) return false;
                    }
                }
                return true;
            }

            [[nodiscard("Result of are_all_same method was ignored.")]]
            bool are_all_same() const
            requires math::isEqualityOperationPossible<T> {
                if (m_order.size() < 2) return true;
                const auto end = this->end_one_d() - 1;
                for (auto it = this->begin_one_d(); it != end; ) {
                    if (!math::is_equal(*it, *(++it))) return false;
                }
                return true;
            }

            [[nodiscard("Result of count method was ignored.")]]
            size_t count(const T &to_find) const
            requires math::isEqualityOperationPossible<T> {
                size_t result{};
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                #pragma omp parallel for collapse(2) schedule(static) reduction(+:result)
                for (size_t i = 0; i < row; i++) {
                    for (size_t j = 0; j < column; j++) {
                        result += math::is_equal(to_find, m_data[i][j]);
                    }
                }
                return result;
            }
    
        public:
            [[nodiscard("Result of == operator was ignored.")]]
            bool operator==(const Matrix &other) const {
                if (m_order != other.m_order) return false;
                if (m_order.is_zero()) return true;
                if (this == &other) return true;
                const size_t row = m_order.row();
                const size_t column = m_order.column();
                for (size_t r = 0; r < row; r++) {
                    const T *const this_cache_data = m_data[r];
                    const T *const other_cache_data = other.m_data[r];
                    for (size_t c = 0; c < column; c++) {
                        if (!math::is_equal(this_cache_data[c], other_cache_data[c])) return false;
                    }
                }
                return true;
            }

            [[nodiscard("Result of != operator was ignored.")]]
            bool operator!=(const Matrix &other) const {
                return !(*this == other);
            }
    
        public:
            void shrink_columns_by(const size_t shrink_amount) noexcept {
                const size_t col = m_order.column();
                if (shrink_amount < col) {
                    const size_t row = m_order.row();
                    const size_t new_num_elements = col - shrink_amount;
                    for (size_t i = 0; i < row; i++) math::memory::impl::reallocate_memory<T>(m_data[i], col, new_num_elements);
                    m_order.set_column(new_num_elements);
                }
                else this->reset();
            }

            void extend_columns_by(const size_t extend_amount)
            requires std::is_copy_constructible_v<T> || std::is_default_constructible_v<T> {
                if (extend_amount == 0 || m_order.row() == 0) return;
                const bool zero_exists = math::zero_vals.exists_of<T>();
                if constexpr (!std::is_default_constructible_v<T>) 
                    if (!zero_exists)
                        throw std::logic_error("Cannot extend the columns of this matrix without any arguments provided because the zero value(either being default constructible or a value being stored in math::zero_vals) (or is not being able to copied if its zero value is stored) for this type does not exist.");
                const size_t row = m_order.row();
                const size_t old_col = m_data.column();
                const size_t new_col = old_col + extend_amount;
                size_t j;
                if constexpr (std::is_copy_constructible_v<T>) {
                    if (!zero_exists) goto DEFAULT_CONSTRUCT;
                    const T &zero = math::zero_vals.get_of<T>;
                    for (size_t i = 0; i < row; i++) {
                        try { math::memory::impl::reallocate_memory(m_data[i], old_col, new_col); } _CATCH_REW_RLC_(m_data, i, old_col, new_col, 0)
                        if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i] + old_col, extend_amount, zero);
                        else _TRY_CONSTRUCT_AT_LOOP_(j, (j < extend_amount), (j++), m_data[i], zero) _CATCH_REW_RLC_(m_data, i, old_col, new_col, j)
                    }
                    m_order.set_column(new_col);
                    return;
                }
                else goto DEFAULT_CONSTRUCT;
                DEFAULT_CONSTRUCT:
                    if constexpr (!std::is_default_constructible_v<T>) throw std::logic_error("Cannot extend the columns of this matrix without any arguments provided because the zero value(either being default constructible or a value being stored in math::zero_vals) (or is not being able to copied if its zero value is stored) for this type does not exist.");
                    for (size_t i = 0; i < row; i++) {
                        try { math::memory::impl::reallocate_memory(m_data[i], old_col, new_col); } _CATCH_REW_RLC_(m_data, i, old_col, new_col, 0)
                        if constexpr (std::is_nothrow_default_constructible_v<T>) std::uninitialized_value_construct_n(m_data[i] + old_col, extend_amount);
                        else _TRY_CONSTRUCT_AT_LOOP_(j, (j < extend_amount), (j++), m_data[i]) _CATCH_REW_RLC_(m_data, i, old_col, new_col, j)
                    }
                    m_order.set_column(new_col);
                    return;
            }

            void extend_columns_by(const size_t extend_amount, const T &copy_val)
            requires std::is_copy_constructible_v<T> {
                if (extend_amount == 0 || m_order.row() == 0) return;
                const size_t row = m_order.row();
                const size_t col = m_order.column();
                const size_t new_col = col + extend_amount;
                size_t j;
                for (size_t i = 0; i < row; i++) {
                    try { math::memory::impl::reallocate_memory(m_data[i], col, new_col); } _CATCH_REW_RLC_(m_data, i, col, new_col, 0)
                    if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i] + col, extend_amount, copy_val);
                    else _TRY_CONSTRUCT_AT_LOOP_(j, (j < extend_amount), (j++), (m_data[i] + col), zero) _CATCH_REW_RLC_(m_data, i, old_col, new_col, j)
                }
                m_order.set_column(new_col);
            }
    
        public:
            void shrink_rows_by(const size_t shrink_amount) noexcept {
                if (shrink_amount < m_order.row()) {
                    const size_t row = m_order.row();
                    const size_t col = m_order.column();
                    for (size_t i = row - shrink_amount; i < row; i++) math::memory::impl::free_memory<T>(m_data[i], col);
                    math::memory::impl::reallocate_memory<T*>(m_data, row, row - shrink_amount); // It is better to free the memory first because T* is trivially destructible and would not destroy the memory by itself.
                }
                else this->reset();
            }
    
            void extend_rows_by(const size_t extend_amount) 
            requires std::is_copy_constructible_v<T> || std::is_default_constructible_v<T> {
                if (extend_amount == 0 || m_order.column() == 0) return;
                const bool zero_exists = math::zero_vals.exists_of<T>();
                const size_t row = m_order.row();
                const size_t new_num_rows = row + extend_amount;
                const size_t col = m_order.column();
                size_t j;
                if constexpr (!std::is_default_constructible_v<T>)
                    if (!zero_exists)
                        throw std::logic_error("Cannot extend the rows of this matrix without any arguments provided because the zero value(either being default constructible or a value being stored in math::zero_vals) (or is not being able to copied if its zero value is stored) for this type does not exist.");
                math::memory::impl::reallocate_memory<T*>(m_data, row, new_num_rows);
                for (size_t i = row; i < new_num_rows; i++) {
                    if constexpr (std::is_copy_constructible_v<T>) {
                        if (!zero_exists) goto DEFAULT_CASE;
                        const T &zero_val = math::zero_vals.get_of<T>();
                        if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i], col, zero_val);
                        else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i], zero_val) _CATCH_REW_RLR_(m_data, i, row, new_num_rows, col, j)
                        continue;
                    }
                    else {
                        goto DEFAULT_CASE; continue;
                    }
                    DEFAULT_CASE:
                        if constexpr (std::is_nothrow_default_constructible_v<T>) std::uninitialized_value_construct_n(m_data[i], col);
                        else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i]) _CATCH_REW_RLR_(m_data, i, row, new_num_rows, col, j)
                }
            }
    
            void extend_rows_by(const size_t extend_amount, const T &copy_val)
            requires std::is_copy_constructible_v<T> {
                if (extend_amount == 0 || m_order.column() == 0) return;
                const size_t row = m_order.row();
                const size_t col = m_order.column();
                const size_t new_num_rows = row + extend_amount;
                math::memory::impl::reallocate_memory<T*>(m_data, row, new_num_rows);
                for (size_t i = row; i < new_num_rows; i++) {
                    if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i], col, copy_val);
                    else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i], copy_val) _CATCH_REW_RLR_(m_data, i, row, new_num_rows, col, j)
                }
            }
    
        public:
            void shrink_by(const size_t row_shrink_amount, const size_t col_shrink_amount) noexcept {
                const size_t num_row = m_order.row();
                const size_t num_col = m_order.column();
                if (row_shrink_amount < num_row && col_shrink_amount < num_col) {
                    const size_t new_num_row = m_order.row() - row_shrink_amount;
                    const size_t new_num_col = m_order.column() - col_shrink_amount;
                    for (size_t i = new_num_row; i < num_row; i++) math::memory::impl::free_memory<T>(m_data[i], num_col);
                    math::memory::impl::reallocate_memory<T>(m_data, num_row, new_num_row);
                    for (size_t i = 0; i < new_num_row; i++) math::memory::impl::reallocate_memory<T>(m_data[i], num_col, new_num_col);
                }
                else this->reset();
            }

            void shrink_by(const size_t row_shrink_amount, const size_t col_shrink_amount) noexcept {
                this->shrink_by(row_shrink_amount, col_shrink_amount);
            }

            void shrink_by(const size_t shrink_amount) noexcept {
                this->shrink_by(shrink_amount, shrink_amount);
            }
            
        public:
            void extend_by(const size_t row_extend_amount, const size_t col_extend_amount)
            requires std::is_default_constructible_v<T> || std::is_copy_constructible_v<T> {
                if (!m_order.is_zero()) {
                    this->extend_columns_by(col_extend_amount);
                    try { this->extend_rows_by(row_extend_amount); } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
                }
                else *this = Matrix(math::matrix::Order(row_extend_amount, col_extend_amount));
            }

            void extend_by(const size_t extend_amount)
            requires std::is_default_constructible_v<T> || std::is_copy_constructible_v<T> {
                this->extend_by(extend_amount, extend_amount);
            }
            
            void extend_by(const size_t row_extend_amount, const size_t col_extend_amount, const T &copy_val)
            requires std::is_copy_constructible_v<T> {
                if (!m_order.is_zero()) {
                    this->extend_columns_by(row_extend_amount, copy_val);
                    try { this->extend_rows_by(row_extend_amount, copy_val); } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
                }
                else *this = Matrix(math::matrix::Order(row_extend_amount, col_extend_amount), copy_val);
            }
    
            void extend_by(const size_t extend_amount, const T &copy_val)
            requires std::is_copy_constructible_v<T> {
                this->extend_by(extend_amount, extend_amount, copy_val);
            }

            void extend_by(const size_t row_extend_amount, const size_t col_extend_amount, const T &row_extend_val, const T &col_extend_val)
            requires std::is_copy_constructible_v<T> {
                if (!m_order.is_zero()) {
                    this->extend_columns_by(col_extend_amount, col_extend_val);
                    try { this->extend_rows_by(row_extend_amount, row_extend_val); } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
                }
                else *this = Matrix(math::matrix::Order(row_extend_amount, col_extend_amount), row_extend_val);
            }

            void extend_by(const size_t extend_amount, const T &row_extend_val, const T &col_extend_val)
            requires std::is_copy_constructible_v<T> {
                this->extend_by(extend_amount, extend_amount, row_extend_val, col_extend_val);
            }

            void extend_by(const size_t row_extend_amount, const size_t col_extend_amount, const T &row_extend_val, const T &col_extend_val, const T &common_extend_val)
            requires std::is_copy_constructible_v<T> {
                if (!m_order.is_zero()) {
                    const size_t col = m_order.column();
                    this->extend_columns_by(col_extend_amount, col_extend_val);
                    if (row_extend_amount != 0) { try {
                        const size_t row = m_order.row();
                        const size_t new_col = m_order.column();
                        const size_t new_num_rows = row + row_extend_amount;
                        math::memory::impl::reallocate_memory<T*>(m_data, row, new_num_rows);
                        for (size_t i = row; i < new_num_rows; i++) {
                            if constexpr (std::is_nothrow_copy_constructible_v<T>) {
                                std::uninitialized_fill_n(m_data[i], col, row_extend_val);
                                std::uninitialized_fill_n(m_data[i] + col, col_extend_amount, common_extend_val);
                            }
                            else try {
                                for (j = 0; j < col; j++) std::construct_at(m_data[i] + j, col_extend_val);
                                for (; j < new_col; j++) std::construct_at(m_data[i] + j, common_extend_val);
                            } _CATCH_REW_RLR_(m_data, i, row, new_num_rows, col, j)
                        } } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
                    }
                }
                else *this = Matrix(math::matrix::Order(row_extend_amount, col_extend_amount), row_extend_val);
            }

            void extend_by(const size_t extend_amount, const T &row_extend_val, const T &col_extend_val, const T &common_extend_val)
            requires std::is_copy_constructible_v<T> {
                this->extend_by(extend_amount, extend_amount, row_extend_val, col_extend_val, common_extend_val);
            }
    };
}