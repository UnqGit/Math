// Matrix.hpp
#pragma once

#include "Helper\MatrixUtils.hpp"
#include "..\Helper\Helper.hpp"
#include "..\Memory\TwoDCstrHelper.hpp"

#define _ROW_COL_ const size_t row = m_order.row(); const size_t col = m_order.column();
#define _ORD_ZERO_RET_ if (m_order.is_zero()) return;

namespace math {
// Make your type no_throw_destructible first.
_MTEMPL_ requires NothrDtor<T> class _NODISC_ Matrix {
    public:
        using order_t = matrix::Order;

    private:
        T **m_data = nullptr;
        order_t m_order;

    public:
        constexpr Matrix() noexcept {}
        
        Matrix(const size_t size) : m_order(order_t(size, size)) {
            _ORD_ZERO_RET_ _ZERO_EXISTS_
            _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            m_data = math::memory::allocate_memory<T*>(size);
            if (zero_exists)
                for (size_t i = 0; i < size; i++) {
                    math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                    math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], _GET_ZERO_, size, m_data, i, size);
                }
            else if constexpr (DfltCtor<T>)
                for (size_t i = 0; i < size; i++) {
                    math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                    math::memory::mem_2d_safe_uninit_valcon_n_continuous<T>(m_data[i], size, m_data, i, size);
                }
        }
        
        Matrix(const size_t size, const T &primary_value, const T &secondary_value, const math::matrix::ConstructSquareRule construct_rule)
        requires CpyCtor<T> : m_order(order_t(size, size)) {
            _ORD_ZERO_RET_ switch(construct_rule) {
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
            m_data = math::memory::allocate_2d_safe_memory<T>(size, size);
            T *cache_data;
            bool is_primary = 0;
            switch(construct_rule) {
                case math::matrix::CSR::full :
                    for (size_t i = 0; i < size; i++) 
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, size, m_data, i, size, size);
                    return;
                case math::matrix::CSR::upper_half :
                    for (size_t i = 0; i < (size>>1); i++) 
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, size, m_data, i, size, size);
                    for (size_t i = (size>>1); i < size; i++)
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                    return;
                case math::matrix::CSR::left_half :
                    for (size_t i = 0; i < size; i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, (size>>1), m_data, i, size, size);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), secondary_value, (size>>1) + (size & 1), m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::top_left_triangle :
                    for (size_t i = 0; i < size; i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, size - i, m_data, i, size, size);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + size - i, secondary_value, i, m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::top_right_triangle :
                    for (size_t i = 0; i < size; i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, i, m_data, i, size, size);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + i, primary_value, size - i, m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::main_diagonal :
                    for (size_t i = 0; i < size; i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, i, m_data, i, size, size);
                        math::memory::mem_2d_safe_construct_at<T>(m_data[i] + i, m_data, i, size, size, primary_value);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + i + 1, secondary_value, size - i - 1, m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::off_diagonal :
                    for (size_t i = 0; i < size; i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size - 1 - i, m_data, i, size, size);
                        math::memory::mem_2d_safe_construct_at<T>(m_data[i] + size - 1 - i, m_data, i, size, size, primary_value);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + size - i, secondary_value, size - i, m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::top_left_quarter :
                    for (size_t i = 0; i < (size>>1); i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, (size>>1), m_data, i, size, size);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), secondary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                    }
                    for (size_t i = (size>>1); i < size; i++)
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                    return;
                case math::matrix::CSR::top_right_quarter :
                    for (size_t i = 0; i < (size>>1); i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, (size>>1), m_data, i, size, size);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), primary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                    }
                    for (size_t i = (size>>1); i < size; i++)
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                    return;
                case math::matrix::CSR::bottom_left_quarter :
                    for (size_t i = 0; i < (size>>1); i++)
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                    for (size_t i = (size>>1); i < size; i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], primary_value, (size>>1), m_data, i, size, size);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), secondary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::bottom_right_quarter :
                    for (size_t i = 0; i < (size>>1); i++)
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, size, m_data, i, size, size);
                    for (size_t i = (size>>1); i < size; i++) {
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], secondary_value, (size>>1), m_data, i, size, size);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i] + (size>>1), primary_value, ((size>>1) + (size&1)), m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::alternate :
                    for (size_t i = 0; i < size; i++) {
                        is_primary = (i % 2 == 0);
                        cache_data = m_data[i];
                        for (size_t j = 0; j < size; j++) {
                            math::memory::mem_2d_safe_construct_at<T>(cache_data + j, m_data, i, size, size, is_primary ? primary_value : secondary_value);
                            is_primary = !is_primary;
                        }
                    }
                    return;
                case math::matrix::CSR::alternate_row :
                    for (size_t i = 0; i < size; i++) {
                        is_primary = (i % 2 == 0);
                        math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], is_primary ? primary_value : secondary_value, size, m_data, i, size, size);
                    }
                    return;
                case math::matrix::CSR::alternate_column :
                    for (size_t i = 0; i < size; i++) {
                        cache_data = m_data[i];
                        is_primary = true;
                        for (size_t j = 0; j < size; j++) {
                            math::memory::mem_2d_safe_construct_at<T>(cache_data + j, m_data, i, size, size, is_primary ? primary_value : secondary_value);
                            is_primary = !is_primary;
                        }
                    }
                    return;
                default: break;
            }
        }
        
        Matrix(const size_t size, const T &primary_value, const math::matrix::ConstructSquareRule construct_rule) 
        requires CpyCtor<T> {
            if (size == 0) return;
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
            if (zero_exists) *this = Matrix(size, primary_value, _GET_ZERO_, construct_rule);
            else if constexpr (DfltCtor<T>) *this = Matrix(size, primary_value, T{}, construct_rule);
        }

    public:
        Matrix(const order_t &order, const math::matrix::ConstructAllocateRule construct_rule = math::matrix::CAR::zero) : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_ _ZERO_EXISTS_
            if (construct_rule == math::matrix::CAR::possible_garbage) {
                if constexpr (!(std::is_trivially_constructible_v<T> || DfltCtor<T>))
                    if (!zero_exists)
                        throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                m_data = math::memory::allocate_2d_safe_memory<T>(row, col);
                if constexpr (!std::is_trivially_constructible_v<T>)
                    for (size_t i = 0; i < row; i++) {
                        if constexpr (!DfltCtor<T>) math::memory::mem_2d_safe_uninit_fill_n<T>(m_data[i], _GET_ZERO_, col, m_data, i, row, col);
                        else math::memory::mem_2d_safe_uninit_valcon_n<T>(m_data[i], col, m_data, i, row, col);
                    }
            }
            else {
                if (!zero_exists)
                    throw std::logic_error("The zero value is not stored of this type in zero_vals hence can't zero construct the Matrix.");
                m_data = math::memory::allocate_memory<T*>(row);
                for (size_t i = 0; i < row; i++) {
                    math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                    math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], _GET_ZERO_, col, m_data, i, col);
                }
            }
        }
        Matrix(const size_t row, const size_t column, const math::matrix::ConstructAllocateRule construct_rule = math::matrix::CAR::zero) : Matrix(order_t(row, column), construct_rule) {}

    public:
        Matrix(const order_t &order, const T &to_copy)
        requires CpyCtor<T> : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_
            m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], to_copy, col, m_data, i, col);
            }
        }
        
        Matrix(const size_t row, const size_t column, const T &to_copy)
        requires CpyCtor<T> : Matrix(order_t(row, column), to_copy) {}
        
    public:
        Matrix(read_ptr<T> data, const size_t size, math::matrix::ConstructOrientationRule construct_rule, const T &fallback_val)
        requires CpyCtor<T> {
            if (size == 0) return;
            switch (construct_rule) {
                case math::matrix::COR::horizontal :
                    m_order = order_t(1, size);
                    m_data = math::memory::allocate_memory<T*>(1);
                    math::memory::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                    math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[0], size, data, m_data, 0, size);
                    return;
                case math::matrix::COR::vertical :
                    m_order = order_t(size, 1);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                        math::memory::mem_2d_safe_construct_at_continuous<T>(m_data[i], m_data, i, size, data[i]);
                    }
                    return;
                case math::matrix::COR::main_diagonal :
                    m_data = math::memory::allocate_memory<T*>(size);
                    m_order = order_t(size, size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, i, m_data, i, size);
                        math::memory::mem_2d_safe_construct_at_continuous<T>(m_data[i] + i, m_data, i, size, data[i]);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + i + 1, fallback_val, size - i - 1, m_data, i, size);
                    }
                    return;
                case math::matrix::COR::off_diagonal :
                    m_data = math::memory::allocate_memory<T*>(size);
                    m_order = order_t(size, size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, size - i - 1, m_data, i, size);
                        math::memory::mem_2d_safe_construct_at_continuous<T>(m_data[i] + size - i - 1, m_data, i, size, data[i]);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + size - i, fallback_val, i, m_data, i, size);
                    }
                    return;
            }
        }

        Matrix(read_ptr<T> data, const size_t size, math::matrix::ConstructOrientationRule construct_rule = math::matrix::COR::horizontal)
        requires CpyCtor<T> {
            if (size == 0) return;
            const bool zero_exists = (zero_vals.exists_of<T>());
            switch (construct_rule) {
                case math::matrix::COR::horizontal :
                    m_order = order_t(1, size);
                    m_data = math::memory::allocate_memory<T*>(1);
                    math::memory::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                    math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[0], size, data, m_data, 0, size);
                    return;
                case math::matrix::COR::vertical :
                    m_order = order_t(size, 1);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                        math::memory::mem_2d_safe_construct_at_continuous<T>(m_data[i], m_data, i, size, data[i]);
                    }
                    return;
                case math::matrix::COR::main_diagonal :
                case math::matrix::COR::off_diagonal :
                    _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                    if constexpr (!DfltCtor<T>) *this = Matrix(data, size, construct_rule, _GET_ZERO_);
                    else *this = Matrix(data, size, construct_rule, T{});
                    return;
            }
        }
        
        Matrix(read_ptr<T> data, const order_t &order)
        requires CpyCtor<T> : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_
            m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], col, data + i * col, m_data, i, col);
            }
        }
        
        Matrix(read_ptr<T> data, const size_t row, const size_t column)
        requires CpyCtor<T> : Matrix(data, order_t(row, column)) {}

    public:
        Matrix(read_ptr<T> data, const size_t size, const order_t &order, const T &fallback_val)
        requires CpyCtor<T> : m_order(order) {
            _ORD_ZERO_RET_
            if (size >= m_order.size()) {
                *this = Matrix(data, m_order);
                return;
            }
            _ROW_COL_
            size_t constructed_items = 0;
            m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                bool is_remaining = size > constructed_items;
                switch(is_remaining) {
                    case true:
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], std::min(size - constructed_items, col), data + constructed_items, m_data, i, col);
                        constructed_items += std::min(size - constructed_items, col);
                        if (constructed_items % col == 0) break;
                        [[fallthrough]];
                    case false:
                        if (constructed_items % col)
                            math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + (constructed_items % col), fallback_val, constructed_items % col, m_data, i, col);
                        else math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, col, m_data, i, col);
                        constructed_items += ((constructed_items % col) == 0 ? col : (constructed_items % col));
                }
            }
        }

        Matrix(read_ptr<T> data, const size_t size, const order_t &order)
        requires CpyCtor<T> : m_order(order) {
            _ORD_ZERO_RET_
            if (size >= m_order.size()) {
                *this = Matrix(data, m_order);
                return;
            }
            if (zero_vals.exists_of<T>()) {
                *this = Matrix(data, size, order, _GET_ZERO_);
                return;
            }
            else if constexpr (DfltCtor<T>) {
                *this = Matrix(data, size, order, T{});
                return;
            }
            throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
        }
        
        Matrix(read_ptr<T> data, const size_t size, const size_t row, const size_t column, const T &fallback_val)
        requires CpyCtor<T> : Matrix(data, size, order_t(row, column), fallback_val) {}
        
        Matrix(read_ptr<T> data, const size_t size, const size_t row, const size_t column)
        requires CpyCtor<T> : Matrix(data, size, order_t(row, column)) {} 
        
    public:
        _MTMPLU_ requires math::helper::isOneDArr<U, T>
        Matrix(const U& arr, const math::matrix::ConstructOrientationRule construct_rule, const T &fallback_val)
        requires CpyCtor<T> {
            const size_t size = arr.size();
            if (size == 0) return;
            auto Iter = arr.begin();
            switch (construct_rule) {
                case math::matrix::COR::horizontal :
                    m_order = order_t(1, size);
                    m_data = math::memory::allocate_memory<T*>(1);
                    math::memory::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                    math::memory::mem_2d_safe_uninit_copy_continuous<T>(m_data[0], arr.begin(), arr.end(), m_data, 0, size);
                    return;
                case math::matrix::COR::vertical :
                    m_order = order_t(size, 1);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], 1, Iter, m_data, i, 1); // This instead of math::memory::mem_2d_safe_construct_at_continuous because we would have to pass *Iter as an argument but if the dereferencing throws an exception, it would leak memory.
                    }
                    return;
                case math::matrix::COR::main_diagonal :
                    m_order = order_t(size, size);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, i, m_data, i, size);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i] + i, 1, Iter, m_data, i, size);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + i + 1, fallback_val, size - i - 1, m_data, i, size);
                        if constexpr ( !noexcept( ++std::declval<std::decay_t<decltype(Iter)>>() )) try {++Iter;} _CATCH_MEM_ERR_CONT_(m_data, i + 1, size)
                        else ++Iter;
                    }
                    return;
                case math::matrix::COR::off_diagonal :
                    m_order = order_t(size, size);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, size);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, size - i - 1, m_data, i, size);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i] + size - 1 - i, 1, Iter, m_data, i, size);
                        math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + size - i, fallback_val, i, m_data, i, size);
                        if constexpr ( !noexcept( ++std::declval<std::decay_t<decltype(Iter)>>() )) try {++Iter;} _CATCH_MEM_ERR_CONT_(m_data, i + 1, size)
                        else ++Iter;
                    }
                    return;
            }
        }

        _MTMPLU_ requires math::helper::isOneDArr<U, T>
        Matrix(const U &arr, const math::matrix::ConstructOrientationRule construct_rule = math::matrix::COR::horizontal)
        requires CpyCtor<T> {
            const size_t size = arr.size();
            if (size == 0) return;
            auto Iter = arr.begin();
            _ZERO_EXISTS_
            switch (construct_rule) {
                case math::matrix::COR::horizontal :
                    m_order = order_t(1, size);
                    m_data = math::memory::allocate_memory<T*>(1);
                    math::memory::allocate_mem_2d_safe_continuous<T>(m_data, 0, size);
                    math::memory::mem_2d_safe_uninit_copy_continuous<T>(m_data[0], arr.begin(), arr.end(), m_data, 0, size);
                    return;
                case math::matrix::COR::vertical :
                    m_order = order_t(size, 1);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, 1);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], 1, Iter, m_data, i, 1); // This instead of math::memory::mem_2d_safe_construct_at_continuous because we would have to pass *Iter as an argument but if the dereferencing throws an exception, it would leak memory.
                    }
                    return;
                case math::matrix::COR::main_diagonal :
                case math::matrix::COR::off_diagonal :
                    _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                    if constexpr (!DfltCtor<T>) *this = Matrix(arr, construct_rule, _GET_ZERO_);
                    else *this = Matrix(arr, construct_rule, T{});
            }
        }
        
        _MTMPLU_ requires math::helper::isOneDArr<U, T>
        Matrix(const U &arr, const order_t &order, const T &fallback_val)
        requires CpyCtor<T> : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_
            const size_t size = arr.size();
            auto Iter = arr.begin();
            size_t j = 0, constructed_items = 0, k;
            m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                bool is_remaining = (constructed_items < size);
                switch (is_remaining) {
                    case true :
                        j = std::min(size - constructed_items, col);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], j, Iter, m_data, i, col);
                        constructed_items += j;
                        if (constructed_items % col == 0) break;
                        [[fallthrough]];
                    case false :
                        if (constructed_items % col == 0) math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i], fallback_val, col, m_data, i, col);
                        else math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + j, fallback_val, col - j, m_data, i, col);
                }
            }
        }

        _MTMPLU_ requires math::helper::isOneDArr<U, T>
        Matrix(const U &arr, const size_t row, const size_t column, const T &fallback_val)
        requires CpyCtor<T> : Matrix(arr, order_t(row, column), fallback_val) {}

        _MTMPLU_ requires math::helper::isOneDArr<U, T>
        Matrix(const U &arr, const order_t &order)
        requires CpyCtor<T> : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_ _ZERO_EXISTS_
            const size_t size = arr.size();
            size_t k;
            if (size < order.size()) {
                _NO_ZERO_COND_ throw std::logic_error("Cannot construct the Matrix for this type because neither zero value is stored and neither is it default constructible.");
                if constexpr (!DfltCtor<T>) *this = Matrix(arr, order, _GET_ZERO_);
                else *this = Matrix(arr, order, T{});
                return;
            }
            auto Iter = arr.begin();
            m_data = math::memory::allocate_memory<T*>(row);
            size_t j = 0;
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], col, Iter, m_data, i, col);
            }
        }
        
        _MTMPLU_ requires math::helper::isOneDArr<U, T>
        Matrix(const U &arr, const size_t row, const size_t column)
        requires CpyCtor<T> : Matrix(arr, order_t(row, column)) {}

    public:
        Matrix(read_ptr2d<T> data, const order_t &order)
        requires CpyCtor<T> : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_
            m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], col, data[i], m_data, i, col);
            }
        }
        
        Matrix(read_ptr2d<T> data, const size_t row, const size_t column)
        requires CpyCtor<T> : Matrix(data, order_t(row, column)) {}
        
    public:
        _MTMPLU_ requires math::helper::isTwoDArr<U, T>
        Matrix(const U &arr, const math::matrix::ConstructContainerRule construct_rule = math::matrix::CCR::must_be_same)
        requires CpyCtor<T> {
            const size_t size = arr.size();
            if (size == 0) return;
            if (size == 1) {
                *this = Matrix(*(arr.begin()), math::matrix::COR::horizontal);
                return;
            }
            size_t row_size;
            auto Iter = arr.begin();
            auto end = arr.end();
            _ZERO_EXISTS_
            const size_t row_objects_created = 0;
            switch (construct_rule) {
                case math::matrix::CCR::shrink :
                    row_size = (*Iter).size();
                    while (Iter != end) {
                        row_size = std::min(row_size, (*Iter).size());
                        if (row_size == 0) return;
                        ++Iter;
                    }
                    m_order = order_t(size, row_size);
                    m_data = math::memory::allocate_memory<T*>(size);
                    Iter = arr.begin();
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], row_size, Iter, m_data, i, row_size);
                    }
                    return;
                case math::matrix::CCR::must_be_same :
                    row_size = (*Iter).size();
                    while (Iter != end) {
                        if (row_size != (*Iter).size()) throw std::invalid_argument("Rows of the provided container are not of the same size and rule chosen is math::matrix::ConstructContainerRule::must_be_same.");
                        ++Iter;
                    }
                    m_order = order_t(size, row_size);
                    if (row_size == 0) return;
                    m_data = math::memory::allocate_memory<T*>(size);
                    Iter = arr.begin();
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], row_size, Iter, m_data, i, row_size);
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
                        _NO_ZERO_COND_ throw std::invalid_argument("Cannot construct the Matrix because the tag set was math::matrix::ConstructContainerRule::expand and all rows were not of the same size and the type is neither default constructible and neither is it's zero value stored in zero_vals.");
                    m_order = order_t(size, row_size);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                        row_objects_created = math::memory::mem_2d_safe_uninit_copy_continuous<T>(m_data[i], (*Iter).begin(), (*Iter).end(), m_data, i, row_size);
                        if (row_objects_created != row_size) {
                            if constexpr (!DfltCtor<T>) 
                                math::memory::mem_2d_safe_uninit_fill_n_continuous<T>(m_data[i] + row_objects_created, _GET_ZERO_, row_size - row_objects_created, m_data, i, row_size);
                            else math::memory::mem_2d_safe_uninit_valcon_n_continuous<T>(m_data[i] + row_objects_created, row_size - row_objects_created, m_data, i, row_size);
                        }
                        ++Iter;
                    }
                    return;
                case math::matrix::CCR::are_same :
                    row_size = (*Iter).size();
                    if (row_size == 0) return;
                    m_order = order_t(size, row_size);
                    m_data = math::memory::allocate_memory<T*>(size);
                    for (size_t i = 0; i < size; i++) {
                        if ((*Iter).size() != row_size) {
                            math::memory::destroy_data_mem_err_continuous<T>(m_data, i, row_size);
                            throw std::logic_error("Promised attribute math::matrix::ConstructConainerRule::are_same was not satisfied in construction of the Matrix.");
                        }
                        m_data[i] = math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, row_size);
                        math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], row_size, Iter, m_data, i, row_size);
                        ++Iter;
                    }
                    return;
            }
        }

    public:
        template <size_t C>
        Matrix(read_ptr<T> data[C], const size_t row)
        requires CpyCtor<T> : m_order(order_t(row, C)) {
            _ORD_ZERO_RET_ m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, C);
                math::memory::mem_2d_safe_uninit_copy_n_continuous<T>(m_data[i], C, data[i], m_data, i, C);
            }
        }

    public:
        Matrix(const order_t &order, const std::function<T()> &t_creation)
        requires (CpyCtor<T> || MvCtor<T>) : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_ size_t j;
            m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, col);
                if constexpr ( noexcept(t_creation) ) { for (j = 0; j < col; j++) std::construct_at(m_data[i] + j, t_creation()); }
                else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i], t_creation()) _CATCH_DES_DATA_CONT_(m_data, i, j, col)
            }
        }
        
        Matrix(const size_t row, const size_t column, const std::function<T()> &t_creation)
        requires (CpyCtor<T> || MvCtor<T>) : Matrix(order_t(row, column), t_creation) {}

        Matrix(const order_t &order, const std::function<T(size_t, size_t)> &t_creation)
        requires (CpyCtor<T> || MvCtor<T>) : m_order(order) {
            _ORD_ZERO_RET_ _ROW_COL_ size_t j;
            m_data = math::memory::allocate_memory<T*>(row);
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(m_data, i, column);
                if constexpr ( noexcept(t_creation) ) { for (j = 0; j < col; j++) std::construct_at(m_data[i] + j, t_creation(i, j)); }
                else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i], t_creation(i, j)) _CATCH_DES_DATA_CONT_(m_data, i, j, col)
            }
        }
        
        Matrix(const size_t row, const size_t column, const std::function<T(size_t, size_t)> &t_creation)
        requires (CpyCtor<T> || MvCtor<T>) : Matrix(order_t(row, column), t_creation) {}

    public:
        Matrix(const Matrix &other) : Matrix(other.m_data, other.m_order) {} // Delegate to Matrix(T** data, const order_t &order) {} constructor.
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
            if (m_data != nullptr) math::memory::destroy_data_mem_err_continuous<T>(m_data, m_order.row(), m_order.column());
        }
        void reset() noexcept {
            *this = Matrix{}; // Call the default constructor.
        }

    public:
        _NODISC_ T &operator()(const size_t row, const size_t column) noexcept {
            return m_data[row][column];
        }
        const T &operator()(const size_t row, const size_t column) const noexcept {
            return m_data[row][column];
        }
        
        _NODISC_ T &at(const size_t row, const size_t column) {
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
        _NODISC_ order_t order() const noexcept {
            return m_order;
        }
        
        _NODISC_ size_t num_rows() const noexcept {
            return m_order.row();
        }
        
        _NODISC_ size_t column_len() const noexcept {
            return m_order.row();
        }
        
        _NODISC_ size_t num_columns() const noexcept {
            return m_order.column();
        }
        
        _NODISC_ size_t row_len() const noexcept {
            return m_order.column();
        }
        
        _NODISC_ size_t size() const noexcept {
            return m_order.size();
        }

    public:
        read_ptr2d<T> data() const noexcept {
            return m_data;
        }
        const_ptr<T> operator[](const size_t row) noexcept {
            return m_data[row];
        }
        read_ptr<T> operator[] (const size_t row) const noexcept {
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
        _NODISC_ bool is_square() const noexcept {
            return m_order.is_square();
        }
        
        _NODISC_ bool is_row() const noexcept {
            return m_order.is_row();
        }
        
        _NODISC_ bool is_column() const noexcept {
            return m_order.is_column();
        }
        
        _NODISC_ bool is_tall() const noexcept {
            return m_order.is_tall();
        }
        
        _NODISC_ bool is_wide() const noexcept {
            return m_order.is_wide();
        }
        
        _NODISC_ bool is_same_dimension(const Matrix &other) const noexcept {
            return (m_order == other.m_order);
        }
        
        _NODISC_ bool is_multipliable_dimension(const Matrix &other) const noexcept {
            return (m_order.column() == other.m_order.row());
        }
        
        _NODISC_ bool is_opposite_dimension(const Matrix &other) const noexcept {
            return (m_order.transpose() == other.m_order);
        }
    
    public:
        Matrix &operator+=(const Matrix &other)
        requires compoundAddition<T> {
            if (!is_same_dimension(other)) throw std::invalid_argument("Cannot add matrices of unequal order parameters.");
            _ROW_COL_
            if constexpr ( noexcept( std::declval<std::decay_t<T>>() += std::declval<std::decay_t<T>>() ) ) {
                #pragma omp parallel for collapse(2) schedule(static)
                for (size_t i = 0; i < row; i++)
                    for (size_t j = 0; j < col; j++)
                        m_data[i][j] += other.m_data[i][j];
            }
            else {
                T **result;
                try { result = math::memory::allocate_2d_safe_memory<T>(row, col); }
                catch(...) { throw std::runtime_error("Could not do addition for this matrix aa an error occured during memory allocation(which was required as the operator(+=) isn't noexcept)."); }
                if constexpr ( noexcept( std::declval<std::decay_t<T>>() + std::declval<std::decay_t<T>>() ) && std::is_nothrow_copy_constructible_v<T> ) {
                    #pragma omp parallel for collapse(2) schedule(static)
                    for (size_t i = 0; i < row; i++)
                        for (size_t j = 0; j < col; j++)
                            std::construct_at(result[i] + j, m_data[i][j] + other.m_data[i][j]);
                }
                else {
                    size_t j;
                    for (size_t i = 0; i < row; i++) {
                        _TRY_CONSTRUCT_AT_LOOP_((j = 0), (j < col), (j++), result[i], m_data[i][j] + other.m_data[i][j])
                        catch(...) {
                            math::memory::destroy_data<T>(m_data, i, j, row, col);
                            throw std::runtime_error("Cannot add the two matrices because of error that occured in either copy construction of the matrix or the operator(+: binary) for the template type T failed(which was required because the template type doesn't have noexcept operator(+=), or failed (noexcept(T+T) && nothrow_copy_constructible))");
                        }
                    }
                }
                Matrix result_mat;
                std::swap(result_mat.m_data, result);
                result_mat.m_order = m_order;
                this->swap(result_mat);
            }
            return *this;
        }

        _NODISC_ Matrix operator+(const Matrix &other) const
        requires compoundAddition<T> {
            Matrix temp(*this);
            temp += other;
            return temp;
        }

        Matrix &operator-=(const Matrix &other)
        requires compoundSubtraction<T> {
            if (!is_same_dimension(other)) throw std::invalid_argument("Cannot subtract matrices of unequal order parameters.");
            _ROW_COL_
            if constexpr ( noexcept( std::declval<std::decay_t<T>>() -= std::declval<std::decay_t<T>>() ) ) {
                #pragma omp parallel for collapse(2) schedule(static)
                for (size_t i = 0; i < row; i++)
                    for (size_t j = 0; j < col; j++)
                        m_data[i][j] -= other.m_data[i][j];
            }
            else {
                T **result;
                try { result = math::memory::allocate_2d_safe_memory<T>(row, col); }
                catch(...) { throw std::runtime_error("Could not do subtraction for this matrix aa an error occured during memory allocation(which was required as the operator(-=) isn't noexcept for the template type T)."); }
                if constexpr ( noexcept( std::declval<std::decay_t<T>>() - std::declval<std::decay_t<T>>() ) && std::is_nothrow_copy_constructible_v<T> ) {
                    #pragma omp parallel for collapse(2) schedule(static)
                    for (size_t i = 0; i < row; i++)
                        for (size_t j = 0; j < col; j++)
                            std::construct_at(result[i] + j, m_data[i][j] - other.m_data[i][j]);
                }
                else {
                    size_t j;
                    for (size_t i = 0; i < row; i++) {
                        _TRY_CONSTRUCT_AT_LOOP_((j = 0), (j < col), (j++), result[i], m_data[i][j] - other.m_data[i][j])
                        catch(...) {
                            math::memory::destroy_data<T>(m_data, i, j, row, col);
                            throw std::runtime_error("Cannot subtract the two matrices because of error that occured in either copy construction of the matrix or the operator(-: binary) for the template type T failed(which was required because the template type doesn't have noexcept operator(-=), or failed (noexcept(T-T) && is_nothrow_copy_constructible_v))");
                        }
                    }
                }
                Matrix result_mat;
                std::swap(result_mat.m_data, result);
                result_mat.m_order = m_order;
                this->swap(result_mat);
            }
            return *this;
        }

        _NODISC_ Matrix operator-(const Matrix &other) const
        requires compoundSubtraction<T> {
            Matrix temp(*this);
            return (temp -= other);
        }

        Matrix &operator*=(const Matrix &other)
        requires compoundMultiplication<T> && compoundAddition<T> {
            *this = *this * other;
            return *this;
        }

        _NODISC_ Matrix operator*(const Matrix &other) const
        requires compoundMultiplication<T> && compoundAddition<T> {
            if (!is_multipliable_dimension(other)) throw std::invalid_argument("Cannot multiply the matrices because the number of columns in first does not match the number of rows in the second.");
            Matrix result;
            if (m_order.is_zero()) return result;
            const size_t row = m_order.row();
            const size_t column = other.m_order.column();
            const size_t this_column = m_order.column();
            T *to_transfer = math::memory::allocate_memory<T*>(row);
            size_t d = 0;
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(to_transfer, i, column);
                const T &cached = m_data[i][0];
                const T *const cache_data = other.m_data[0];
                T *const data = to_transfer[i];
                if constexpr ( noexcept(_DECL_ * _DECL_) ) for (d = 0; d < column; d++) std::construct_at(data + d, cached * cache_data[d]);
                else _TRY_CONSTRUCT_AT_LOOP_(d, (d < column), (d++), data, cached * cache_data[d]) _CATCH_DES_DATA_CONT_(to_transfer, i, d, column)
            }
            // it is fine till here if an exception is called and the destructor of result is called because the order is zero and hence it wouldn't try to free memory.
            std::swap(result.m_data, to_transfer); // m_data was nullptr before this.
            result.m_order = order_t(row, column);
            if (result.is_wide()) {
                for (size_t i = 0; i < row; i++) {
                    T *const data = result.m_data[i];
                    for (size_t k = 1; k < this_column; k++) {
                        const T &cached = m_data[i][k];
                        const T *const other_cached = other.m_data[k];
                        #pragma omp parallel for
                        for (size_t j = 0; j < column; j++) data[j] += cached * other_cached[j];
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
                        for (size_t j = 0; j < column; j++) data[j] += cached * other_cached[j];
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
        _NODISC_ Matrix transpose() const
        requires CpyCtor<T> {
            Matrix result;
            if (m_order.is_zero()) return result;
            _ROW_COL_
            T *to_transfer = math::memory::allocate_memory<T*>(row);
            size_t j;
            for (size_t i = 0; i < row; i++) {
                math::memory::allocate_mem_2d_safe_continuous<T>(to_transfer, i, col);
                T* data = to_transfer[i];
                for (j = 0; j < col; j++)
                    math::memory::mem_2d_safe_construct_at_continuous<T>(data + j, m_data, i, col, m_data[j][i]);
            }
            std::swap(result.m_data, to_transfer); // Automatically sets to_transfer to nullptr.
            result.m_order = m_order.transpose();
            return result;
        }

        Matrix &transpose_in_place()
        requires (CpyCtor<T> || std::is_nothrow_swappable_v<T>) {
            if (m_order.is_zero()) return *this;
            if constexpr (std::is_nothrow_swappable_v<T>) {
                if (m_order.is_square()) {
                    const size_t size = m_order.row();
                    for (size_t i = 0; i < size; i++) {
                        T *const data = m_data[i];
                        #pragma omp parallel for schedule(static)
                        for (size_t j = i + 1; j < size; j++) std::swap(data[j], m_data[j][i]);
                    }
                }
                else if constexpr (CpyCtor<T>) *this = this->transpose();
                else throw std::logic_error("Cannot do transposition on this Matrix because it is neither a square Matrix and is not copy constructible to create a new Matrix.");
            }
            else if constexpr (CpyCtor<T>) *this = this->transpose();
            else throw std::logic_error("Cannot do transposition on this Matrix because it is neither a square Matrix(or the type is not swappable) and is not copy constructible to create a new Matrix.");
            return *this;
        }

    public:
        _NODISC_ T trace() const
        requires compoundAddition<T> {
            if (!(this->is_square())) throw std::logic_error("Cannot find trace of a non square Matrix.");
            _ZERO_EXISTS_
            if (m_order.is_zero()) {
                _NO_ZERO_COND_ throw std::logic_error("Cannot provide the value of trace for a zero size Matrix with a type of which neither a default constructor exists nor is the zero value stored.");
                if (zero_exists) return _GET_ZERO_;
                else return T{};
            }
            const size_t size = m_order.row();
            if constexpr (CpyCtor<T>) {
                if (zero_exists) {
                    T result(_GET_ZERO_);
                    for (size_t i = 0; i < size; i++) result += m_data[i][i];
                    return result;
                }
                else {
                    T result(m_data[0][0]);
                    for (size_t i = 1; i < size; i++) result += m_data[i][i];
                    return result;
                }
            }
            else if constexpr (DfltCtor<T>) {
                T result{};
                for (size_t i = 0; i < size; i++) result += m_data[i][i];
                return result;
            }
            else throw std::logic_error("Cannot provide the trace of the Matrix because it is not copy constructible for storing initial zero/default value and is neither default constructible.");
        }

        _NODISC_ bool is_zero() const
        requires isEqualityOperationPossible<T> {
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw std::logic_error("Cannot check for is_zero property of the Matrix as the zero value(stored in zero_vals or defautlt construction for the type) is not defined.");
            _ROW_COL_
            const T &to_check_from = zero_exists ? _GET_ZERO_ : T{};
            for (size_t i = 0; i < row; i++)
                for (size_t j = 0; j < col; j++)
                    if (!is_equal(to_check_from, m_data[i][j])) return false;
            return true;
        }
        
        _NODISC_ bool are_all_same_as(const T &to_check_from) const
        requires isEqualityOperationPossible<T> {
            _ROW_COL_
            for (size_t i = 0; i < row; i++)
                for (size_t j = 0; j < col; j++)
                    if (!is_equal(to_check_from, m_data[i][j])) return false;
            return true;
        }

        _NODISC_ bool are_all_same() const
        requires isEqualityOperationPossible<T> {
            if (m_order.size() < 2) return true;
            const auto end = this->end_one_d() - 1;
            for (auto it = this->begin_one_d(); it != end; )
                if (!is_equal(*it, *(++it))) return false;
            return true;
        }

        _NODISC_ size_t count(const T &to_find) const
        requires isEqualityOperationPossible<T> {
            size_t result{};
            _ROW_COL_
            #pragma omp parallel for collapse(2) schedule(static) reduction(+:result)
            for (size_t i = 0; i < row; i++)
                for (size_t j = 0; j < col; j++)
                    result += is_equal(to_find, m_data[i][j]);
            return result;
        }

    public:
        _NODISC_ bool operator==(const Matrix &other) const {
            if (m_order != other.m_order) return false;
            if (m_order.is_zero()) return true;
            if (this == &other) return true;
            _ROW_COL_
            for (size_t r = 0; r < row; r++) {
                const T *const this_cache_data = m_data[r];
                const T *const other_cache_data = other.m_data[r];
                for (size_t c = 0; c < col; c++)
                    if (!is_equal(this_cache_data[c], other_cache_data[c])) return false;
            }
            return true;
        }

        _NODISC_ bool operator!=(const Matrix &other) const {
            return !(*this == other);
        }

    public:
        void shrink_columns_by(const size_t shrink_amount) noexcept {
            _ROW_COL_
            if (shrink_amount < col) {
                const size_t new_num_elements = col - shrink_amount;
                for (size_t i = 0; i < row; i++) math::memory::reallocate_memory<T>(m_data[i], col, new_num_elements);
                m_order.set_column(new_num_elements);
            }
            else this->reset();
        }

        void extend_columns_by(const size_t extend_amount)
        requires CpyCtor<T> || DfltCtor<T> {
            if (extend_amount == 0 || m_order.row() == 0) return;
            _ZERO_EXISTS_
            _NO_ZERO_COND_ throw std::logic_error("Cannot extend the columns of this matrix without any arguments provided because the zero value(either being default constructible or a value being stored in zero_vals) (or is not being able to copied if its zero value is stored) for this type does not exist.");
            _ROW_COL_
            const size_t new_col = col + extend_amount;
            size_t j;
            if constexpr (CpyCtor<T>) {
                if (!zero_exists) goto DEFAULT_CONSTRUCT;
                const T &zero = _GET_ZERO_;
                for (size_t i = 0; i < row; i++) {
                    math::memory::reallocate_memory(m_data[i], col, new_col);
                    if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i] + col, extend_amount, zero);
                    else _TRY_CONSTRUCT_AT_LOOP_(j, (j < extend_amount), (j++), m_data[i], zero) _CATCH_REW_RLC_(m_data, i, col, new_col, j)
                }
                m_order.set_column(new_col);
                return;
            }
            else goto DEFAULT_CONSTRUCT;
            DEFAULT_CONSTRUCT:
                if constexpr (!DfltCtor<T>) throw std::logic_error("Cannot extend the columns of this matrix without any arguments provided because the zero value(either being default constructible or a value being stored in zero_vals) (or is not being able to copied if its zero value is stored) for this type does not exist.");
                for (size_t i = 0; i < row; i++) {
                    math::memory::reallocate_memory(m_data[i], col, new_col);
                    if constexpr (std::is_nothrow_default_constructible_v<T>) std::uninitialized_value_construct_n(m_data[i] + col, extend_amount);
                    else _TRY_CONSTRUCT_AT_LOOP_(j, (j < extend_amount), (j++), m_data[i]) _CATCH_REW_RLC_(m_data, i, col, new_col, j)
                }
                m_order.set_column(new_col);
                return;
        }

        void extend_columns_by(const size_t extend_amount, const T &copy_val)
        requires CpyCtor<T> {
            if (extend_amount == 0 || m_order.row() == 0) return;
            _ROW_COL_
            const size_t new_col = col + extend_amount;
            size_t j;
            for (size_t i = 0; i < row; i++) {
                math::memory::reallocate_memory(m_data[i], col, new_col);
                if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i] + col, extend_amount, copy_val);
                else _TRY_CONSTRUCT_AT_LOOP_(j, (j < extend_amount), (j++), (m_data[i] + col), copy_val) _CATCH_REW_RLC_(m_data, i, col, new_col, j)
            }
            m_order.set_column(new_col);
        }

    public:
        void shrink_rows_by(const size_t shrink_amount) noexcept {
            if (shrink_amount < m_order.row()) {
                _ROW_COL_
                for (size_t i = row - shrink_amount; i < row; i++) math::memory::free_memory<T>(m_data[i], col);
                math::memory::reallocate_memory<T*>(m_data, row, row - shrink_amount); // It is better to free the memory first because T* is trivially destructible and would not destroy the memory by itself.
                m_order.set_row(row - shrink_amount);
            }
            else this->reset();
        }

        void extend_rows_by(const size_t extend_amount) 
        requires CpyCtor<T> || DfltCtor<T> {
            if (extend_amount == 0 || m_order.column() == 0) return;
            _ZERO_EXISTS_
            _ROW_COL_
            const size_t new_row = row + extend_amount;
            size_t j;
            _NO_ZERO_COND_ throw std::logic_error("Cannot extend the rows of this matrix without any arguments provided because the zero value(either being default constructible or a value being stored in zero_vals) (or is not being able to copied if its zero value is stored) for this type does not exist.");
            math::memory::reallocate_memory<T*>(m_data, row, new_row);
            for (size_t i = row; i < new_row; i++) {
                if constexpr (CpyCtor<T>) {
                    if (!zero_exists) goto DEFAULT_CASE;
                    const T &zero_val = _GET_ZERO_;
                    if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i], col, zero_val);
                    else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i], zero_val) _CATCH_REW_RLR_(m_data, i, row, new_row, col, j)
                }
                else goto DEFAULT_CASE;
                continue;
                DEFAULT_CASE:
                    if constexpr (std::is_nothrow_default_constructible_v<T>) std::uninitialized_value_construct_n(m_data[i], col);
                    else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i]) _CATCH_REW_RLR_(m_data, i, row, new_row, col, j)
            }
            m_order.set_row(new_row);
        }

        void extend_rows_by(const size_t extend_amount, const T &copy_val)
        requires CpyCtor<T> {
            if (extend_amount == 0 || m_order.column() == 0) return;
            _ROW_COL_
            const size_t new_row = row + extend_amount;
            size_t j;
            math::memory::reallocate_memory<T*>(m_data, row, new_row);
            for (size_t i = row; i < new_row; i++) {
                if constexpr (std::is_nothrow_copy_constructible_v<T>) std::uninitialized_fill_n(m_data[i], col, copy_val);
                else _TRY_CONSTRUCT_AT_LOOP_(j, (j < col), (j++), m_data[i], copy_val) _CATCH_REW_RLR_(m_data, i, row, new_row, col, j)
            }
            m_order.set_row(new_row);
        }

    public:
        void shrink_by(const size_t row_shrink_amount, const size_t col_shrink_amount) noexcept {
            this->shrink_rows_by(row_shrink_amount);
            this>shrink_columns_by(col_shrink_amount);
        }

        void shrink_by(const size_t shrink_amount) noexcept {
            this->shrink_by(shrink_amount, shrink_amount);
        }
        
    public:
        void extend_by(const size_t row_extend_amount, const size_t col_extend_amount)
        requires DfltCtor<T> || CpyCtor<T> {
            if (!m_order.is_zero()) {
                this->extend_columns_by(col_extend_amount);
                try { this->extend_rows_by(row_extend_amount); } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
            }
            else *this = Matrix(order_t(row_extend_amount, col_extend_amount));
        }

        void extend_by(const size_t extend_amount)
        requires DfltCtor<T> || CpyCtor<T> {
            this->extend_by(extend_amount, extend_amount);
        }

        void extend_by(const size_t row_extend_amount, const size_t col_extend_amount, const T &copy_val)
        requires CpyCtor<T> {
            if (!m_order.is_zero()) {
                this->extend_columns_by(row_extend_amount, copy_val);
                try { this->extend_rows_by(row_extend_amount, copy_val); } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
            }
            else *this = Matrix(order_t(row_extend_amount, col_extend_amount), copy_val);
        }

        void extend_by(const size_t extend_amount, const T &copy_val)
        requires CpyCtor<T> {
            this->extend_by(extend_amount, extend_amount, copy_val);
        }

        void extend_by(const size_t row_extend_amount, const size_t col_extend_amount, const T &row_extend_val, const T &col_extend_val)
        requires CpyCtor<T> {
            if (!m_order.is_zero()) {
                this->extend_columns_by(col_extend_amount, col_extend_val);
                try { this->extend_rows_by(row_extend_amount, row_extend_val); } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
            }
            else *this = Matrix(order_t(row_extend_amount, col_extend_amount), row_extend_val);
        }

        void extend_by(const size_t extend_amount, const T &row_extend_val, const T &col_extend_val)
        requires CpyCtor<T> {
            this->extend_by(extend_amount, extend_amount, row_extend_val, col_extend_val);
        }

        void extend_by(const size_t row_extend_amount, const size_t col_extend_amount, const T &row_extend_val, const T &col_extend_val, const T &common_extend_val)
        requires CpyCtor<T> {
            if (!m_order.is_zero()) {
                this->extend_columns_by(col_extend_amount, col_extend_val);
                if (row_extend_amount != 0) try {
                    _ROW_COL_
                    const size_t new_col = col + col_extend_amount;
                    const size_t new_row = row + row_extend_amount;
                    size_t j;
                    math::memory::reallocate_memory<T*>(m_data, row, new_row);
                    for (size_t i = row; i < new_row; i++) {
                        if constexpr (std::is_nothrow_copy_constructible_v<T>) {
                            std::uninitialized_fill_n(m_data[i], col, row_extend_val);
                            std::uninitialized_fill_n(m_data[i] + col, col_extend_amount, common_extend_val);
                        }
                        else try {
                            for (j = 0; j < col; j++) std::construct_at(m_data[i] + j, col_extend_val);
                            for (; j < new_col; j++) std::construct_at(m_data[i] + j, common_extend_val);
                        } _CATCH_REW_RLR_(m_data, i, row, new_row, col, j)
                    }
                } catch(...) { this->shrink_columns_by(col_extend_amount); throw; }
            }
            else *this = Matrix(order_t(row_extend_amount, col_extend_amount), row_extend_val);
        }

        void extend_by(const size_t extend_amount, const T &row_extend_val, const T &col_extend_val, const T &common_extend_val)
        requires CpyCtor<T> {
            this->extend_by(extend_amount, extend_amount, row_extend_val, col_extend_val, common_extend_val);
        }
};

}