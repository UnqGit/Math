// _memory_alloc_.hpp
#pragma once

#include "Headers.hpp"

namespace math::memory::impl
{

    // Allocating row memory, a C++ wrapper on malloc.
    template <typename T>
    [[nodiscard("Shouldn't be allocating memory without getting a pointer to it.")]]
    T *allocate_memory(const size_t num_elements) {
        if (num_elements == 0) return nullptr;
        T *ptr = static_cast<T*>(std::malloc(sizeof(T) * num_elements));
        if (ptr) return ptr;
        throw std::bad_alloc{};
    }

    // Safely free-ing memory.
    template <typename T>
    void free_memory(T* &memory, const size_t created_items) {
        if (!std::is_trivially_destructible_v<T>) std::destroy(memory, memory + created_items);
        std::free(memory);
        memory = nullptr;
    }

    // Reallocating memory, a safer wrapper on realloc.
    template <typename T>
    T *reallocate_memory(T* &mem_ptr, const size_t old_num_elements, const size_t num_elements)
    requires (std::is_nothrow_move_constructible_v<T> || std::is_copy_constructible_v<T> || std::is_trivially_copyable_v<T>)
    {
        if (mem_ptr == nullptr) return allocate_memory<T>(num_elements);
        if (old_num_elements == num_elements) return mem_ptr;
        if (num_elements == 0) {
            free_memory<T>(mem_ptr, old_num_elements);
            return mem_ptr;
        }
        if (num_elements < old_num_elements) {
            if constexpr (!std::is_trivially_destructible_v<T>) std::destroy(mem_ptr + old_num_elements, mem_ptr + num_elements);
            mem_ptr = static_cast<T*>(std::realloc(mem_ptr, sizeof(T) * num_elements));
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
    void destroy_data_mem_err(T** &data, const size_t curr_i) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], 0);
        free_memory<T*>(data, 0);
    }

    // This is for when the memory is being allocated continuously and is constructed in the same loop and the error occurs in the memory allocation.
    template <typename T>
    void destroy_data_mem_err_continuous(T** &data, const size_t curr_i, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T*>(data, 0);
    }

    // This is for when the memory is allocated in a separate loop and constructed in another.
    template <typename T>
    void destroy_data(T** &data, const size_t curr_i, const size_t end_row_created_items, const size_t num_row, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T>(data[curr_i], end_row_created_items);
        for (size_t i = curr_i + 1; i < num_row; i++) free_memory<T>(data[i], 0);
        free_memory<T*>(data, 0);
    }

    // This is for when the memory is allocated and constructed in the same loop.
    template <typename T>
    void destroy_data_continuous(T** &data, const size_t curr_i, const size_t end_row_created_items, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T>(data[curr_i], end_row_created_items);
        free_memory<T*>(data, 0);
    }

    // ======DRY SECTOR======

    // Shorthand for when memory is allocated in a separate loop, with exception safety.
    template <typename T>
    void allocate_mem_2d_safe(T** &mem_ptr, const size_t curr_i, const size_t row_size) {
        try {
            mem_ptr[curr_i] = allocate_memory<T>(row_size);
        } catch(...) {
            destroy_data_mem_err<T>(mem_ptr, curr_i);
            throw;
        }
    }

    // Shorthand for allocating memory continuously in a loop with construction, with exception safety.
    template <typename T>
    void allocate_mem_2d_safe_continuous(T** &mem_ptr, const size_t curr_i, const size_t row_size) {
        try {
            mem_ptr[curr_i] = allocate_memory<T>(row_size);
        } catch(...) {
            destroy_data_mem_err_continuous<T>(mem_ptr, curr_i, row_size);
            throw;
        }
    }

    template <typename T>
    void mem_2d_safe_construct_at(T* &to_construct_at, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size, const auto &..._args) {
        try {
            std::construct_at(to_construct_at, _args);
        } catch(...) {
            destroy_data<T>(mem, curr_i, to_construct_at - mem[curr_i], num_rows, row_size);
            throw;
        }
    }

    template <typename T>
    void mem_2d_safe_construct_at_continuous(T* &to_construct_at, T** &mem, const size_t curr_i, const size_t row_size, const auto &..._args) {
        try {
            std::construct_at(to_construct_at, _args);
        } catch(...) {
            destroy_data_continuous<T>(mem, curr_i, to_construct_at - mem[curr_i], row_size);
            throw;
        }
    }

    template <typename T>
    void mem_2d_safe_uninit_fill_n(T* &to_construct_at, const T &val, const size_t size, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
    requires std::is_copy_constructible_v<T> {
        if constexpr (std::is_nothrow_copy_constructible_v<T>) {
            std::uninitialized_fill_n(to_construct_at, size, val);
            return;
        }
        for (size_t created_items = 0; created_items < size; created_items++) {
            try {
                std::construct_at(to_construct_at + created_items, val);
            } catch(...) {
                destroy_data<T>(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size);
                throw;
            }
        }

    }

    template <typename T>
    void mem_2d_safe_uninit_fill_n_continuous(T* &to_construct_at, const T &val, const size_t size, T** &mem, const size_t curr_i, const size_t row_size)
    requires std::is_copy_constructible_v<T> {
        if constexpr (std::is_nothrow_copy_constructible_v<T>) {
            std::uninitialized_fill_n(to_construct_at, size, val);
            return;
        }
        for (size_t created_items = 0; created_items < size; created_items++) {
            try {
                std::construct_at(to_construct_at + created_items, val);
            } catch(...) {
                destroy_data_continuous<T>(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size);
                throw;
            }
        }
    }

    template <typename T>
    void mem_2d_safe_uninit_valcon_n(T* &to_construct_at, const size_t size, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
    requires std::is_default_constructible_v<T> {
        if constexpr (std::is_nothrow_default_constructible_v<T>) {
            std::uninitialized_value_construct_n(to_construct_at, size);
            return;
        }
        for (size_t created_items = 0; created_items < size; created_items++) {
            try {
                std::construct_at(to_construct_at + created_items);
            } catch(...) {
                destroy_data<T>(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size);
                throw;
            }
        }
    }

    template <typename T>
    void mem_2d_safe_uninit_valcon_n_continuous(T* &to_construct_at, const size_t size, T** &mem, const size_t curr_i, const size_t row_size)
    requires std::is_default_constructible_v<T> {
        if constexpr (std::is_nothrow_default_constructible_v<T>) {
            std::uninitialized_value_construct_n(to_construct_at, size);
            return;
        }
        for (size_t created_items = 0; created_items < size; created_items++) {
            try {
                std::construct_at(to_construct_at + created_items);
            } catch(...) {
                destroy_data_continuous<T>(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size);
                throw;
            }
        }
    }

    template <typename T, std::input_iterator Iter>
    void mem_2d_safe_uninit_copy_n(T* &to_construct_at, const size_t size, Iter it, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
    requires std::is_copy_constructible_v<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
        if constexpr (std::is_nothrow_constructible_v<T>) {
            std::uninitialized_copy_n(it, size, to_construct_at);
            return;
        }
        for (size_t created_items = 0; created_items < size; created_items++, ++it) {
            try {
                std::construct_at(to_construct_at + created_items, *it);
            } catch(...) {
                destroy_data<T>(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size);
                throw;
            }
        }
    }

    template <typename T, std::input_iterator Iter>
    void mem_2d_safe_uninit_copy_n_continuous(T* &to_construct_at, const size_t size, Iter it, T** &mem, const size_t curr_i, const size_t row_size)
    requires std::is_copy_constructible_v<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
        if constexpr (std::is_nothrow_constructible_v<T>) {
            std::uninitialized_copy_n(it, size, to_construct_at);
            return;
        }
        for (size_t created_items = 0; created_items < size; created_items++, ++it) {
            try {
                std::construct_at(to_construct_at + created_items, *it);
            } catch(...) {
                destroy_data_continuous<T>(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size);
                throw;
            }
        }
    }
}