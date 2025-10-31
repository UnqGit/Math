// MemoryAlloc.hpp
#pragma once
#include "..\Helper\Headers.hpp"

namespace math::memory::impl {
using aligned_alloc_t = void* (*)(size_t, size_t);
using free_t = void(*)(void*);
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
    #include <malloc.h>
    inline void *msc_aligned_alloc(size_t alignment, size_t size) {
        return _aligned_malloc(size, alignment);
    }
    aligned_alloc_t aligned_allocate = msc_aligned_alloc;
    free_t free = _aligned_free;
#else
    aligned_alloc_t aligned_allocate = std::aligned_alloc;
    free_t free = std::free;
#endif
}

// Destructor of the math::Classes are noexcept(true) because the class itself can only be made if the std::is_nothrow_destructible_v<T> type_trait is true and hence the free mem function is fine being noexcept
namespace math::memory {
/**
 * @brief Allocating row memory, a C++ wrapper on malloc.
 * @tparam T Type of the elements to allocate memory for.
 * @param num_elements Number of elements to allocate memory for.
 * @throws std::bad_alloc If the memory allocation fails.
 * @return Pointer to the allocated memory.
*/
_MTEMPL_ _NODISC_ inline T *allocate_memory(const size_t num_elements) {
    static constexpr const size_t align(alignof(T));
    static constexpr const size_t size(sizeof(T) == 0 ? 1 : sizeof(T));
    if (num_elements > (static_cast<size_t>(~0) / size)) throw std::bad_alloc{};
    size_t bytes = size * num_elements;
    if (bytes == 0) bytes = 1;
    T *ptr;
    if (bytes == 1) ptr = static_cast<T*>(std::malloc(1));
    else if constexpr (align > alignof(std::max_align_t)) ptr = static_cast<T*>(math::memory::impl::aligned_allocate(align, bytes));
    else ptr = static_cast<T*>(std::malloc(bytes));        
    if (ptr) [[likely]] return ptr;
    else throw std::bad_alloc{};
}

/**
 * @brief Safely free-ing memory.
 * @tparam T Type of the elements to free memory for.
 * @param memory Pointer to the memory to free.
 * @param created_items Number of elements to call destructor for.
 * @return Pointer to the allocated memory.
*/
_MTEMPL_ requires NothrDtor<T>
inline void free_memory(T* &memory, const size_t created_items) noexcept {
    if (memory != nullptr) {
        if constexpr (!TrvDtor<T>) std::destroy_n(memory, created_items);
        if constexpr (alignof(T) > alignof(std::max_align_t)) math::memory::impl::free(memory);
        else std::free(memory);
        memory = nullptr;
    }
}

/**
 * @brief Reallocating memory, a safer wrapper on realloc.
 * @tparam T Type of the elements to reallocate memory for.
 * @param mem_ptr Pointer to the memory to reallocate.
 * @param old_num_elements Number of elements in old memory.
 * @param num_elements Number of elements to reallocate memory for.
 * @throws std::bad_alloc If the memory reallocation fails.
 * @return Pointer to the reallocated memory.
*/
_MTEMPL_ inline T *reallocate_memory(T* &mem_ptr, const size_t old_num_elements, const size_t num_elements)
requires ((std::is_nothrow_move_constructible_v<T> || CpyCtor<T> || std::is_trivially_copyable_v<T>) && NothrDtor<T>) {
    if (mem_ptr == nullptr) return allocate_memory<T>(num_elements);
    if (old_num_elements == num_elements) return mem_ptr;
    if (num_elements == 0) {
        free_memory(mem_ptr, old_num_elements);
        return mem_ptr;
    }
    if (num_elements < old_num_elements) {
        if constexpr (!TrvDtor<T>) std::destroy_n(mem_ptr + num_elements, old_num_elements - num_elements);
        T *temp = static_cast<T*>(std::realloc(mem_ptr, sizeof(T) * num_elements));
        if (!temp) throw std::bad_alloc{};
        return (mem_ptr = temp);
    }
    if constexpr (std::is_trivially_copyable_v<T> && TrvDtor<T>) {
        T *temp = static_cast<T*>(std::realloc(mem_ptr, sizeof(T) * num_elements));
        if (!temp) throw std::bad_alloc{};
        return (mem_ptr = temp);
    }
    T *temp = allocate_memory<T>(num_elements);
    if constexpr (std::is_nothrow_move_constructible_v<T>)
        std::uninitialized_move_n(mem_ptr, old_num_elements, temp);
    else if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
        size_t created_items = 0;
        try { for (; created_items < old_num_elements; created_items++) std::construct_at(temp + created_items, *(mem_ptr + created_items)); }
        catch(...) { free_memory(temp, created_items); throw; }
    }
    else std::uninitialized_copy_n(mem_ptr, old_num_elements, temp);
    free_memory(mem_ptr, old_num_elements);
    return (mem_ptr = temp);
}
#define _TRY_CONSTRUCT_AT_(ptr, ...) try { std::construct_at(ptr, ##__VA_ARGS__); }
#define _TRY_CONSTRUCT_AT_LOOP_(loop_var, loop_end_condition, loop_increment_cond, ptr, ...) try { for (loop_var = 0; loop_end_condition; loop_increment_cond) std::construct_at(ptr + loop_var, ##__VA_ARGS__); }
}