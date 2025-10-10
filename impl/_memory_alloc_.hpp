// _memory_alloc_.hpp
#pragma once

#include "Headers.hpp"

#define _PRE_INC_2_(x, y) ++x; ++y;

namespace {
    template <typename T, std::input_iterator Iter>
    requires std::is_nothrow_copy_constructible_v<T>
    inline static void nothrow_copy_construct(T *to_construct_at, Iter &begin, Iter &end, size_t &constructed_items) noexcept {
        if constexpr (!std::random_access_iterator<Iter>) {
            while (begin != end) {
                std::construct_at(to_construct_at + constructed_items, *begin);
                _PRE_INC_2_(begin, constructed_items)
            }
        }
        else {
            std::uninitialized_copy(begin, end, to_construct_at);
            constructed_items = end - begin;
        }
    }
}

// Destructor of the math::Classes are noexcept(true) because the class itself can only be made if the std::is_nothrow_destructible_v<T> type_trait is true and hence the free mem function is fine being noexcept
namespace math::memory::impl
{
    // Allocating row memory, a C++ wrapper on malloc.
    template <typename T>
    [[nodiscard("Shouldn't be allocating memory without getting a pointer to it.")]]
    inline T *allocate_memory(const size_t num_elements) {
        if (num_elements == 0) return nullptr;
        T *ptr = static_cast<T*>(std::malloc(sizeof(T) * num_elements));
        if (ptr) return ptr;
        throw std::bad_alloc{};
    }

    // Safely free-ing memory.
    template <typename T>
    requires ( std::is_nothrow_destructible_v<T> )
    inline void free_memory(T* &memory, const size_t created_items) noexcept {
        if (memory == nullptr) return;
        if constexpr ( !std::is_trivially_destructible_v<T> ) std::destroy_n(memory, created_items);
        std::free(memory);
        memory = nullptr;
    }

    // Reallocating memory, a safer wrapper on realloc.
    template <typename T>
    inline T *reallocate_memory(T* &mem_ptr, const size_t old_num_elements, const size_t num_elements)
    requires (std::is_nothrow_move_constructible_v<T> || std::is_copy_constructible_v<T> || std::is_trivially_copyable_v<T>)
    {
        if (mem_ptr == nullptr) return allocate_memory<T>(num_elements);
        if (old_num_elements == num_elements) return mem_ptr;
        if (num_elements == 0) {
            free_memory<T>(mem_ptr, old_num_elements);
            return mem_ptr;
        }
        if (num_elements < old_num_elements) {
            if constexpr (!std::is_trivially_destructible_v<T>) std::destroy(mem_ptr + num_elements, mem_ptr + old_num_elements);
            T *temp = static_cast<T*>(std::realloc(mem_ptr, sizeof(T) * num_elements));
            if (temp) mem_ptr = temp;
            else throw std::bad_alloc{}; 
            return mem_ptr;
        }
        if constexpr (std::is_trivially_copyable_v<T>) {
            T *temp = static_cast<T*>(std::realloc(mem_ptr, sizeof(T) * num_elements));
            if (!temp) throw std::bad_alloc{};
            mem_ptr = temp;
            return mem_ptr;
        }
        T *temp = static_cast<T*>(std::malloc(sizeof(T) * num_elements));
        if (!temp) throw std::bad_alloc{};
        if constexpr (std::is_nothrow_move_constructible_v<T>) {
            std::uninitialized_move_n(mem_ptr, old_num_elements, temp);
            std::free(mem_ptr);
        }
        else if constexpr (std::is_copy_constructible_v<T>) {
            T *end = nullptr;
            try {
                end = std::uninitialized_copy_n(mem_ptr, old_num_elements, temp);
            } catch(...) {
                free_memory<T>(temp, end - temp);
                throw;
            }
            free_memory<T>(mem_ptr, old_num_elements);
        }
        mem_ptr = temp;
        return mem_ptr;
    }

    // This is for when the error occurs in a pure memore allocation loop.
    template <typename T>
    inline void destroy_data_mem_err(T** &data, const size_t curr_i) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], 0);
        free_memory<T*>(data, 0);
    }

    // This is for when the memory is being allocated continuously and is constructed in the same loop and the error occurs in the memory allocation.
    template <typename T>
    inline void destroy_data_mem_err_continuous(T** &data, const size_t curr_i, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T*>(data, 0);
    }

    // This is for when the memory is allocated in a separate loop and constructed in another.
    template <typename T>
    inline void destroy_data(T** &data, const size_t curr_i, const size_t end_row_created_items, const size_t num_row, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T>(data[curr_i], end_row_created_items);
        for (size_t i = curr_i + 1; i < num_row; i++) free_memory<T>(data[i], 0);
        free_memory<T*>(data, 0);
    }

    // This is for when the memory is allocated and constructed in the same loop.
    template <typename T>
    inline void destroy_data_continuous(T** &data, const size_t curr_i, const size_t end_row_created_items, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T>(data[curr_i], end_row_created_items);
        free_memory<T*>(data, 0);
    }

    // ======DRY SECTOR======

    #define _CATCH_MEM_ERR_(x, y) catch(...) { destroy_data_mem_err<T>(x, y); throw; }

    #define _CATCH_MEM_ERR_CONT_(x, y, z) catch(...) { destroy_data_mem_err_continuous<T>(x, y, z); throw; }

    #define _CATCH_DES_DATA_(x, y, z, a, b) catch(...) { destroy_data<T>(x, y, z, a, b); throw; }

    #define _CATCH_DES_DATA_CONT_(x, y, z, a) catch(...) { destroy_data_continuous<T>(x, y, z, a); throw; }

    #define _TRY_CONSTRUCT_AT_(x, ...) try { std::construct_at(x, ##__VA_ARGS__); }

    // Shorthand for when memory is allocated in a separate loop, with exception safety.
    template <typename T>
    inline void allocate_mem_2d_safe(T** &mem_ptr, const size_t curr_i, const size_t row_size) {
        try { mem_ptr[curr_i] = allocate_memory<T>(row_size); } _CATCH_MEM_ERR_(mem_ptr, curr_i)
    }

    // Shorthand for allocating memory continuously in a loop with construction, with exception safety.
    template <typename T>
    inline void allocate_mem_2d_safe_continuous(T** &mem_ptr, const size_t curr_i, const size_t row_size) {
        try { mem_ptr[curr_i] = allocate_memory<T>(row_size); } _CATCH_MEM_ERR_CONT_(mem_ptr, curr_i, row_size)
    }

    template <typename T, typename ...Args>
    inline void mem_2d_safe_construct_at(T* to_construct_at, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size, Args&&... _args) {
        _TRY_CONSTRUCT_AT_(to_construct_at, std::forward<Args>(_args)...)
        _CATCH_DES_DATA_(mem, curr_i, to_construct_at - mem[curr_i], num_rows, row_size)
    }

    template <typename T, typename ...Args>
    inline void mem_2d_safe_construct_at_continuous(T* to_construct_at, T** &mem, const size_t curr_i, const size_t row_size, Args&&... _args) {
        _TRY_CONSTRUCT_AT_(to_construct_at, std::forward<Args>(_args)...)
        _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at - mem[curr_i], row_size)
    }

    template <typename T>
    inline void mem_2d_safe_uninit_fill_n(T* to_construct_at, const T &val, const size_t size, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
    requires std::is_copy_constructible_v<T> {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            for (size_t created_items = 0; created_items < size; created_items++) {
                _TRY_CONSTRUCT_AT_(to_construct_at + created_items, val)
                _CATCH_DES_DATA_(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size)
            }
        } else std::uninitialized_fill_n(to_construct_at, size, val);
    }

    template <typename T>
    inline void mem_2d_safe_uninit_fill_n_continuous(T* to_construct_at, const T &val, const size_t size, T** &mem, const size_t curr_i, const size_t row_size)
    requires std::is_copy_constructible_v<T> {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            for (size_t created_items = 0; created_items < size; created_items++) {
                _TRY_CONSTRUCT_AT_(to_construct_at + created_items, val)
                _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size)
            }
        } else std::uninitialized_fill_n(to_construct_at, size, val);
    }

    template <typename T>
    inline void mem_2d_safe_uninit_valcon_n(T* to_construct_at, const size_t size, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
    requires std::is_default_constructible_v<T> {
        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            for (size_t created_items = 0; created_items < size; created_items++) {
                try { std::construct_at(to_construct_at + created_items); } _CATCH_DES_DATA_(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size)
            }
        } else std::uninitialized_value_construct_n(to_construct_at, size);
    }

    template <typename T>
    inline void mem_2d_safe_uninit_valcon_n_continuous(T* to_construct_at, const size_t size, T** &mem, const size_t curr_i, const size_t row_size)
    requires std::is_default_constructible_v<T> {
        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            for (size_t created_items = 0; created_items < size; created_items++) {
                try { std::construct_at(to_construct_at + created_items); } _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size)
            }
        } else std::uninitialized_value_construct_n(to_construct_at, size);
    }

    template <typename T, std::input_iterator Iter>
    inline void mem_2d_safe_uninit_copy_n(T* to_construct_at, const size_t size, Iter it, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
    requires std::is_copy_constructible_v<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            for (size_t created_items = 0; created_items < size; ++created_items, ++it) {
                _TRY_CONSTRUCT_AT_(to_construct_at + created_items, *it)
                _CATCH_DES_DATA_(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size)
            }
        } else std::uninitialized_copy_n(it, size, to_construct_at);
    }

    template <typename T, std::input_iterator Iter>
    inline void mem_2d_safe_uninit_copy_n_continuous(T* to_construct_at, const size_t size, Iter it, T** &mem, const size_t curr_i, const size_t row_size)
    requires std::is_copy_constructible_v<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            for (size_t created_items = 0; created_items < size; ++created_items, ++it) {
                _TRY_CONSTRUCT_AT_(to_construct_at + created_items, *it)
                _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size)
            }
        } else std::uninitialized_copy_n(it, size, to_construct_at);
    }

    template <typename T, std::input_iterator Iter>
    inline size_t mem_2d_safe_uninit_copy(T* to_construct_at, Iter begin, Iter end, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
    requires std::is_copy_constructible_v<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
        size_t constructed_items = 0;
        if constexpr (!std::is_nothrow_copy_constructible_v<T> || !noexcept( *std::declval<Iter>() ) || !noexcept( ++std::declval<Iter>() ) ) {
            while (begin != end) {
                _TRY_CONSTRUCT_AT_(to_construct_at + constructed_items, *begin)
                _CATCH_DES_DATA_(mem, curr_i, constructed_items, num_rows, row_size)
                _PRE_INC_2_(begin, constructed_items)
            }
        }
        else ::nothrow_copy_construct(to_construct_at, begin, end, constructed_items);
        return constructed_items;
    }
    
    template <typename T, std::input_iterator Iter>
    inline size_t mem_2d_safe_uninit_copy_continuous(T* to_construct_at, Iter begin, Iter end, T** &mem, const size_t curr_i, const size_t row_size)
    requires std::is_copy_constructible_v<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
        size_t constructed_items = 0;
        if constexpr (!std::is_nothrow_copy_constructible_v<T> || !noexcept( *std::declval<Iter>() ) || !noexcept( ++std::declval<Iter>() ) ) {
            while (begin != end) {
                _TRY_CONSTRUCT_AT_(to_construct_at + constructed_items, *begin)
                _CATCH_DES_DATA_CONT_(mem, curr_i, constructed_items, row_size)
                _PRE_INC_2_(begin, constructed_items)
            }
        }
        else ::nothrow_copy_construct(to_construct_at, begin, end, constructed_items);
        return constructed_items;
    }

    template <typename T>
    inline void rewind_col_reallocate_2d_mem(T **mem_ptr, const size_t curr_i, const size_t rewind_size, const size_t extended_size, const size_t curr_i_created_items) noexcept {
        for (size_t i = 0; i < curr_i; i++) reallocate_memory<T>(mem_ptr[i], extended_size, rewind_size);
        reallocate_memory<T>(mem_ptr[curr_i], rewind_size + curr_i_created_items, rewind_size);
        /*
            Does the same thing as:
            std::destroy(m_data[i] + old_col, m_data[i] + old_col + j); // If not trivially destructible
            std::realloc(m_data[i], old_col * sizeof(T));
        */
    }

    template <typename T>
    inline void rewind_row_reallocate_2d_mem(T **mem_ptr, const size_t curr_i, const size_t rewind_size, const size_t new_size, const size_t column_size, const size_t curr_i_created_items) noexcept {
        for (size_t i = rewind_size; i < curr_i; i++) math::memory::impl::free_memory<T>(mem_ptr[i], colum_size);
        math::memory::impl::free_memory<T>(mem_ptr[curr_i], curr_i_created_items);
        math::memory::impl::reallocate_memory<T*>(mem_ptr, new_size, rewind_size); // Because T* is trivially copyable, free-ing memory is important.
    }

    #define _CATCH_REW_RLC_(x, y, z, a, b) catch(...) { rewind_col_reallocate_2d_mem<T>(x, y, z, a, b); throw; }
    #define _CATCH_REW_RLR_(x, y, z, a, b, c) catch(...) { rewind_row_reallocate_2d_mem<T>(x, y, z, a, b, c); throw; }
}