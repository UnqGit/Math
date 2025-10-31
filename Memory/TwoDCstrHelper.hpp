// TwoDCstrHelper.hpp
#pragma once
#include "MemoryAlloc.hpp"

#define _PRE_INC_2_(x, y) ++x; ++y;

// To be used in the constructors for a 2d matrix/(some other object).

namespace math::memory::impl {
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
        constructed_items += end - begin;
    }
}
}

namespace math::memory {
/**
 * @brief Destroying data in a pure memory allocation loop.
 * @tparam T Type of the elements to destroy.
 * @param data Pointer to the 2D array of data to destroy.
 * @param curr_i Current index of the 2D array of data.
*/
_MTEMPL_ inline void destroy_data_mem_err(T** &data, const size_t curr_i) {
    for (size_t i = 0; i < curr_i; i++) math::memory::free_memory(data[i], 0);
    math::memory::free_memory(data, 0);
}

/**
 * @brief Destroying data in a continuous memory allocation loop(allocation and construction in the same loop).
 * @tparam T Type of the elements to destroy.
 * @param data Pointer to the 2D array of data to destroy.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
*/
_MTEMPL_ inline void destroy_data_mem_err_continuous(T** &data, const size_t curr_i, const size_t row_size) {
    for (size_t i = 0; i < curr_i; i++) math::memory::free_memory(data[i], row_size);
    math::memory::free_memory(data, 0);
}

/**
 * @brief Destroying data in the construction loop(when the memory is allocated in a separate loop and constructed in another).
 * @tparam T Type of the elements to destroy.
 * @param data Pointer to the 2D array of data to destroy.
 * @param curr_i Current index of the 2D array of data.
 * @param end_row_created_items Number of elements created in the last row.
 * @param num_row Number of rows in the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
*/
_MTEMPL_ inline void destroy_data(T** &data, const size_t curr_i, const size_t end_row_created_items, const size_t num_row, const size_t row_size) {
    for (size_t i = 0; i < curr_i; i++) math::memory::free_memory(data[i], row_size);
    math::memory::free_memory(data[curr_i], end_row_created_items);
    for (size_t i = curr_i + 1; i < num_row; i++) math::memory::free_memory(data[i], 0);
    math::memory::free_memory(data, 0);
}

/**
 * @brief Destroying data in a 2D array(when the memory is allocated and constructed in the same loop).
 * @tparam T Type of the elements to destroy.
 * @param data Pointer to the 2D array of data to destroy.
 * @param curr_i Current index of the 2D array of data.
 * @param end_row_created_items Number of elements created in the last row.
 * @param row_size Size of the rows of the 2D array of data.
*/
_MTEMPL_ inline void destroy_data_continuous(T** &data, const size_t curr_i, const size_t end_row_created_items, const size_t row_size) {
    for (size_t i = 0; i < curr_i; i++) math::memory::free_memory(data[i], row_size);
    math::memory::free_memory(data[curr_i], end_row_created_items);
    math::memory::free_memory(data, 0);
}

// ======DRY SECTOR======
#define _CATCH_MEM_ERR_(ptr, index) catch(...) { math::memory::destroy_data_mem_err(ptr, index); throw; }
#define _CATCH_MEM_ERR_CONT_(ptr, index, size_of_rows) catch(...) { math::memory::destroy_data_mem_err_continuous(ptr, index, size_of_rows); throw; }
#define _CATCH_DES_DATA_(ptr, index, index_created_elements, number_of_rows, size_of_rows_before) catch(...) { math::memory::destroy_data(ptr, index, index_created_elements, number_of_rows, size_of_rows_before); throw; }
#define _CATCH_DES_DATA_CONT_(ptr, index, index_created_elements, size_of_rows_before) catch(...) { math::memory::destroy_data_continuous(ptr, index, index_created_elements, size_of_rows_before); throw; }

/**
 * @brief Allocating memory for a 2D array with exception safety.
 * @tparam T Type of the elements to allocate memory for.
 * @param num_rows Number of rows in the 2D array.
 * @param row_size Size of the rows of the 2D array.
 * @throws std::bad_alloc If the memory allocation fails.
 * @return Pointer to the allocated memory.
*/
_MTEMPL_ inline T** allocate_2d_safe_memory(const size_t num_rows, const size_t row_size) {
    T** mem_ptr = math::memory::allocate_memory<T*>(num_rows);
    size_t i = 0;
    try { for (; i < num_rows; i++) mem_ptr[i] = math::memory::allocate_memory<T>(row_size); }
    catch(...) { math::memory::destroy_data_mem_err(mem_ptr, i); throw; }
    return mem_ptr;
}

/**
 * @brief Allocating memory for a row in a 2D array(allocation and construction in a separate loop).
 * @tparam T Type of the elements to allocate memory for.
 * @param mem_ptr Pointer to the 2D array of data to allocate the row memory for.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::bad_alloc If the memory allocation fails.
*/
_MTEMPL_ inline void allocate_mem_row_2d_safe(T** &mem_ptr, const size_t curr_i, const size_t row_size) {
    try { mem_ptr[curr_i] = math::memory::allocate_memory<T>(row_size); } _CATCH_MEM_ERR_(mem_ptr, curr_i)
}

/**
 * @brief Allocating memory for a row in a 2D array(allocation and construction in the same loop).
 * @tparam T Type of the elements to allocate memory for.
 * @param mem_ptr Pointer to the 2D array of data to allocate the row memory for.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::bad_alloc If the memory allocation fails.
*/
_MTEMPL_ inline void allocate_mem_2d_safe_continuous(T** &mem_ptr, const size_t curr_i, const size_t row_size) {
    try { mem_ptr[curr_i] = math::memory::allocate_memory<T>(row_size); } _CATCH_MEM_ERR_CONT_(mem_ptr, curr_i, row_size)
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in separate loops).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param mem Pointer to the 2D array of data to construct the object in.
 * @param curr_i Current index of the 2D array of data.
 * @param num_rows Number of rows in the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @param _args Arguments to pass to the constructor.
 * @throws std::exception If the constructor throws an exception.
*/
_MTYPE_TEMPL(T, ...Args) inline void mem_2d_safe_construct_at(T* to_construct_at, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size, Args&&... _args) {
    _TRY_CONSTRUCT_AT_(to_construct_at, std::forward<Args>(_args)...)
    _CATCH_DES_DATA_(mem, curr_i, to_construct_at - mem[curr_i], num_rows, row_size)
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in separate loops).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param mem Pointer to the 2D array of data to construct the object in.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @param _args Arguments to pass to the constructor.
 * @throws std::exception If the constructor throws an exception.
*/
_MTYPE_TEMPL(T, ...Args) inline void mem_2d_safe_construct_at_continuous(T* to_construct_at, T** &mem, const size_t curr_i, const size_t row_size, Args&&... _args) {
    _TRY_CONSTRUCT_AT_(to_construct_at, std::forward<Args>(_args)...)
    _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at - mem[curr_i], row_size)
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in separate loops).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param val Value to fill the memory with.
 * @param size Number of elements to fill in memory with the value.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param num_rows Number of rows in the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
_MTEMPL_ inline void mem_2d_safe_uninit_fill_n(T* to_construct_at, const T &val, const size_t size, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
requires CpyCtor<T> {
    if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
        size_t created_items;
        _TRY_CONSTRUCT_AT_LOOP_(created_items, (created_items < size), (created_items++), to_construct_at, val)
        _CATCH_DES_DATA_(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size)
    } else std::uninitialized_fill_n(to_construct_at, size, val);
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in the same loop).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param val Value to fill the memory with.
 * @param size Number of elements to fill in memory with the value.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
_MTEMPL_ inline void mem_2d_safe_uninit_fill_n_continuous(T* to_construct_at, const T &val, const size_t size, T** &mem, const size_t curr_i, const size_t row_size)
requires CpyCtor<T> {
    if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
        size_t created_items;
        _TRY_CONSTRUCT_AT_LOOP_(created_items, (created_items < size), (created_items++), to_construct_at, val)
        _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size)
    } else std::uninitialized_fill_n(to_construct_at, size, val);
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in separate loops).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param size Number of elements to construct in memory.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param num_rows Number of rows in the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
_MTEMPL_ inline void mem_2d_safe_uninit_valcon_n(T* to_construct_at, const size_t size, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
requires DfltCtor<T> {
    if constexpr (!std::is_nothrow_default_constructible_v<T>) {
        size_t created_items;
        _TRY_CONSTRUCT_AT_LOOP_(created_items, (created_items < size), (created_items++), to_construct_at)
        _CATCH_DES_DATA_(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size)
    } else std::uninitialized_value_construct_n(to_construct_at, size);
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in the same loop).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param size Number of elements to construct in memory.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
_MTEMPL_ inline void mem_2d_safe_uninit_valcon_n_continuous(T* to_construct_at, const size_t size, T** &mem, const size_t curr_i, const size_t row_size)
requires DfltCtor<T> {
    if constexpr (!std::is_nothrow_default_constructible_v<T>) {
        size_t created_items;
        _TRY_CONSTRUCT_AT_LOOP_(created_items, (created_items < size), (created_items++), to_construct_at)
        _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size)
    } else std::uninitialized_value_construct_n(to_construct_at, size);
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in separate loops).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param size Number of elements to construct in memory.
 * @param it Iterator to the beginning of the range of data to construct the object from.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param num_rows Number of rows in the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
template <typename T, std::input_iterator Iter>
inline void mem_2d_safe_uninit_copy_n(T* to_construct_at, const size_t size, Iter &it, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
requires CpyCtor<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
    if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
        size_t created_items;
        _TRY_CONSTRUCT_AT_LOOP_(created_items, (created_items < size), ((++created_items), ++it), to_construct_at, *it)
        _CATCH_DES_DATA_(mem, curr_i, to_construct_at + created_items - mem[curr_i], num_rows, row_size)
    } else std::uninitialized_copy_n(it, size, to_construct_at);
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in the same loop).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param size Number of elements to construct in memory.
 * @param it Iterator to the beginning of the range of data to construct the object from.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
template <typename T, std::input_iterator Iter>
inline void mem_2d_safe_uninit_copy_n_continuous(T* to_construct_at, const size_t size, Iter &it, T** &mem, const size_t curr_i, const size_t row_size)
requires CpyCtor<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
    if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
        size_t created_items;
        _TRY_CONSTRUCT_AT_LOOP_(created_items, (created_items < size), ((++created_items), ++it), to_construct_at, *it)
        _CATCH_DES_DATA_CONT_(mem, curr_i, to_construct_at + created_items - mem[curr_i], row_size)
    } else std::uninitialized_copy_n(it, size, to_construct_at);
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in separate loops).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param begin Iterator to the beginning of the range of data to construct the object from.
 * @param end Iterator to the end of the range of data to construct the object from.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param num_rows Number of rows in the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
template <typename T, std::input_iterator Iter>
inline size_t mem_2d_safe_uninit_copy(T* to_construct_at, Iter begin, Iter end, T** &mem, const size_t curr_i, const size_t num_rows, const size_t row_size)
requires CpyCtor<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
    size_t constructed_items = 0;
    if constexpr (!std::is_nothrow_copy_constructible_v<T> || !noexcept( *std::declval<Iter>() ) || !noexcept( ++std::declval<Iter>() ) ) {
        try { while (begin != end) {
            std::construct_at(to_construct_at + constructed_items, *begin);
            _PRE_INC_2_(constructed_items, begin)
        } } _CATCH_DES_DATA_(mem, curr_i, constructed_items, num_rows, row_size)
    }
    else math::memory::impl::nothrow_copy_construct(to_construct_at, begin, end, constructed_items);
    return constructed_items;
}

/**
 * @brief Constructing an object at a given memory location in a 2D array(allocation and construction in the same loop).
 * @tparam T Type of the data to construct.
 * @param to_construct_at Pointer to the memory location to construct the object at.
 * @param begin Iterator to the beginning of the range of data to construct the object from.
 * @param end Iterator to the end of the range of data to construct the object from.
 * @param mem Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param row_size Size of the rows of the 2D array of data.
 * @throws std::exception If the constructor throws an exception.
*/
template <typename T, std::input_iterator Iter>
inline size_t mem_2d_safe_uninit_copy_continuous(T* to_construct_at, Iter begin, Iter end, T** &mem, const size_t curr_i, const size_t row_size)
requires CpyCtor<T> && std::same_as<std::decay_t<T>, std::decay_t<decltype(*std::declval<Iter>())>> {
    size_t constructed_items = 0;
    if constexpr (!std::is_nothrow_copy_constructible_v<T> || !noexcept( *std::declval<Iter>() ) || !noexcept( ++std::declval<Iter>() ) ) {
        try { while (begin != end) {
            std::construct_at(to_construct_at + constructed_items, *begin);
            _PRE_INC_2_(constructed_items, begin)
        } } _CATCH_DES_DATA_CONT_(mem, curr_i, constructed_items, row_size)
    }
    else math::memory::impl::nothrow_copy_construct(to_construct_at, begin, end, constructed_items);
    return constructed_items;
}

/**
* @brief Rewinding the reallocation of a column in a 2D array(only when extending the column).
* @tparam T Type of the data to construct.
* @param mem_ptr Pointer to the 2D array of data.
* @param curr_i Current index of the 2D array of data.
* @param rewind_size Size of the column to rewind.
* @param extended_size Size of the column extended to.
* @param curr_i_created_items Number of elements created in the current column.
*/
_MTEMPL_ inline void rewind_col_reallocate_2d_mem(T **&mem_ptr, const size_t curr_i, const size_t rewind_size, const size_t extended_size, const size_t curr_i_created_items) noexcept {
    for (size_t i = 0; i < curr_i; i++) math::memory::reallocate_memory(mem_ptr[i], extended_size, rewind_size);
    math::memory::reallocate_memory(mem_ptr[curr_i], rewind_size + curr_i_created_items, rewind_size);
    /*
        Does the same thing as:
        std::destroy(m_data[i] + old_col, m_data[i] + old_col + j); // If not trivially destructible
        std::realloc(m_data[i], old_col * sizeof(T));
    */
}

/**
 * @brief Rewinding the reallocation of a row in a 2D array(only when extending the row).
 * @tparam T Type of the data to construct.
 * @param mem_ptr Pointer to the 2D array of data.
 * @param curr_i Current index of the 2D array of data.
 * @param rewind_size Size of the row to rewind.
 * @param new_size Size of the row extended to.
 * @param curr_i_created_items Number of elements created in the current row.
 * @param row_size Size of the rows of the 2D array of data.
 */
_MTEMPL_ inline void rewind_row_reallocate_2d_mem(T **&mem_ptr, const size_t curr_i, const size_t rewind_size, const size_t new_size, const size_t curr_i_created_items, const size_t row_size) noexcept {
    for (size_t i = rewind_size; i < curr_i; i++) math::memory::free_memory(mem_ptr[i], row_size);
    math::memory::free_memory(mem_ptr[curr_i], curr_i_created_items);
    math::memory::reallocate_memory(mem_ptr, new_size, rewind_size); // Because T* is trivially copyable, free-ing memory is important.
}

#define _CATCH_REW_RLC_(ptr, index, prev_size, extended_size, index_created_items) catch(...) { math::memory::rewind_col_reallocate_2d_mem(ptr, index, prev_size, extended_size, index_created_items); throw; }
#define _CATCH_REW_RLR_(ptr, index, prev_size, extended_size, index_created_items, row_size) catch(...) { math::memory::rewind_row_reallocate_2d_mem(ptr, index, prev_size, extended_size, index_created_items, row_size); throw; }
}