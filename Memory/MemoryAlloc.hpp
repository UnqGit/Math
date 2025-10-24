// MemoryAlloc.hpp
#pragma once

#include "..\Helper\Headers.hpp"

// Destructor of the math::Classes are noexcept(true) because the class itself can only be made if the std::is_nothrow_destructible_v<T> type_trait is true and hence the free mem function is fine being noexcept
_MMEM_START_
/**
 * @brief Allocating row memory, a C++ wrapper on malloc.
 * @tparam T Type of the elements to allocate memory for.
 * @param num_elements Number of elements to allocate memory for.
 * @throws std::bad_alloc If the memory allocation fails.
 * @return Pointer to the allocated memory.
*/
_MTEMPL_ _NODISC_ inline T *allocate_memory(const size_t num_elements) {
    if (num_elements == 0) return nullptr;
    T *ptr = static_cast<T*>(_STD_ malloc(sizeof(T) * num_elements));
    if (ptr) return ptr;
    throw _STD_ bad_alloc{};
}

/**
 * @brief Safely free-ing memory.
 * @tparam T Type of the elements to free memory for.
 * @param memory Pointer to the memory to free.
 * @param created_items Number of elements to call destructor for.
 * @return Pointer to the allocated memory.
*/
_MTEMPL_ requires _NOTHR_DSTR_
inline void free_memory(T* &memory, const size_t created_items) noexcept {
    if (memory == nullptr) return;
    if constexpr ( !_TRV_DSTR_ ) _STD_ destroy_n(memory, created_items);
    _STD_ free(memory);
    memory = nullptr;
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
requires ((_STD_ is_nothrow_move_constructible_v<T> || _CPY_CSTR_ || _STD_ is_trivially_copyable_v<T>) && _NOTHR_DSTR_) {
    if (mem_ptr == nullptr) return _MEM_ allocate_memory<T>(num_elements);
    if (old_num_elements == num_elements) return mem_ptr;
    if (num_elements == 0) {
        _MEM_ free_memory<T>(mem_ptr, old_num_elements);
        return mem_ptr;
    }
    if (num_elements < old_num_elements) {
        if constexpr (!_TRV_DSTR_) _STD_ destroy_n(mem_ptr + num_elements, old_num_elements - num_elements);
        T *temp = static_cast<T*>(_STD_ realloc(mem_ptr, sizeof(T) * num_elements));
        if (!temp) throw _STD_ bad_alloc{};
        return (mem_ptr = temp);
    }
    if constexpr (_STD_ is_trivially_copyable_v<T> && _TRV_DSTR_) {
        T *temp = static_cast<T*>(_STD_ realloc(mem_ptr, sizeof(T) * num_elements));
        if (!temp) throw _STD_ bad_alloc{};
        return (mem_ptr = temp);
    }
    T *temp = _MEM_ allocate_memory<T>(num_elements);
    if constexpr (_STD_ is_nothrow_move_constructible_v<T>)
        _STD_ uninitialized_move_n(mem_ptr, old_num_elements, temp);
    else if constexpr (!_STD_ is_nothrow_copy_constructible_v<T>) {
        size_t created_items = 0;
        try { for (; created_items < old_num_elements; created_items++) _STD_ construct_at(temp + created_items, *(mem_ptr + created_items)); }
        catch(...) { _MEM_ free_memory<T>(temp, created_items); throw; }
    }
    else _STD_ uninitialized_copy_n(mem_ptr, old_num_elements, temp);
    _MEM_ free_memory<T>(mem_ptr, old_num_elements);
    return (mem_ptr = temp);
}
#define _TRY_CONSTRUCT_AT_(ptr, ...) try { _STD_ construct_at(ptr, ##__VA_ARGS__); }
#define _TRY_CONSTRUCT_AT_LOOP_(loop_var, loop_end_condition, loop_increment_cond, ptr, ...) try { for (loop_var = 0; loop_end_condition; loop_increment_cond) _STD_ construct_at(ptr + loop_var, ##__VA_ARGS__); }
_MATH_END_