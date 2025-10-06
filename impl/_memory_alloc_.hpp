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
    void destroy_data(T** &data, const size_t curr_i, const T *const end_curr_i, const size_t num_row, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T>(data[curr_i], end_curr_i - data[curr_i]);
        for (size_t i = curr_i + 1; i < num_row; i++) free_memory<T>(data[i], row_size);
        free_memory<T*>(data, 0);
    }

    // This is for when the memory is allocated and constructed in the same loop.
    template <typename T>
    void destroy_data_continuous(T** &data, const size_t curr_i, const T *const end_curr_i, const size_t row_size) {
        for (size_t i = 0; i < curr_i; i++) free_memory<T>(data[i], row_size);
        free_memory<T>(data[curr_i], 0);
        free_memory<T*>(data, 0);
    }

}